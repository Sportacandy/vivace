/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "castserver.h"
#include "qrcodegen.h"

#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QImage>
#include <QMimeDatabase>
#include <QNetworkInterface>
#include <QPainter>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextStream>
#include <functional>
#include <memory>
#include <vector>

namespace {

// 256 KiB per write, paced by bytesWritten() so a whole (potentially
// multi-gigabyte) video file is never buffered in memory at once.
constexpr qint64 kChunkSize = 256 * 1024;

// Stream [start, start+length) of `path` to `socket`, one chunk per
// bytesWritten() so we never hold more than kChunkSize in flight.
void streamFileRange(QTcpSocket *socket, const QString &path, qint64 start, qint64 length)
{
    auto file = std::make_shared<QFile>(path);
    if (!file->open(QIODevice::ReadOnly) || !file->seek(start)) {
        socket->disconnectFromHost();
        return;
    }
    auto remaining = std::make_shared<qint64>(length);
    auto connection = std::make_shared<QMetaObject::Connection>();
    auto writeNext = std::make_shared<std::function<void()>>();
    *writeNext = [socket, file, remaining, connection]() {
        if (*remaining <= 0 || socket->state() != QAbstractSocket::ConnectedState) {
            QObject::disconnect(*connection);
            return;
        }
        const QByteArray chunk = file->read(qMin(kChunkSize, *remaining));
        if (chunk.isEmpty()) {
            QObject::disconnect(*connection);
            socket->disconnectFromHost();
            return;
        }
        socket->write(chunk);
        *remaining -= chunk.size();
    };
    *connection = QObject::connect(socket, &QTcpSocket::bytesWritten, socket,
                                    [writeNext](qint64) { (*writeNext)(); });
    (*writeNext)(); // kick off the first chunk immediately
}

} // namespace

CastServer::CastServer(QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, &CastServer::handleNewConnection);
}

void CastServer::setController(PlayerController *controller)
{
    if (controller == m_controller)
        return;
    m_controller = controller;
    emit controllerChanged();
}

bool CastServer::running() const
{
    return m_server->isListening();
}

void CastServer::setPort(int port)
{
    if (port == m_port)
        return;
    m_port = port;
    emit portChanged();
}

namespace {

// Interface name markers for virtualization software's own private/
// host-only networks (VMware, VirtualBox, Hyper-V/WSL2, Docker, ...): they
// hand out ordinary-looking private IPv4 addresses that are only reachable
// from this machine (or its own VMs/containers), never from a phone on the
// LAN, so QNetworkInterface::Type doesn't reliably distinguish them from a
// real Ethernet/Wi-Fi adapter (Windows reports them as plain "Ethernet"
// media too) — matched by name instead.
const QStringList &virtualAdapterNameMarkers()
{
    static const QStringList markers = {
        QStringLiteral("vmware"),   QStringLiteral("virtualbox"),
        QStringLiteral("vethernet"), QStringLiteral("hyper-v"),
        QStringLiteral("virtual switch"), QStringLiteral("docker"),
        QStringLiteral("wsl"),
    };
    return markers;
}

} // namespace

QStringList CastServer::urls() const
{
    QStringList result;
    if (!running())
        return result;
    const auto interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        const auto flags = iface.flags();
        if (!(flags & QNetworkInterface::IsUp) || !(flags & QNetworkInterface::IsRunning))
            continue;
        if (flags & QNetworkInterface::IsLoopBack)
            continue;

        const QString name = iface.humanReadableName().toLower();
        bool isVirtual = false;
        for (const QString &marker : virtualAdapterNameMarkers()) {
            if (name.contains(marker)) {
                isVirtual = true;
                break;
            }
        }
        if (isVirtual)
            continue;

        const auto entries = iface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries) {
            const QHostAddress addr = entry.ip();
            if (addr.protocol() != QAbstractSocket::IPv4Protocol)
                continue;
            if (addr.isLoopback() || addr.isLinkLocal())
                continue; // link-local (169.254.0.0/16): no real network
            result << QStringLiteral("http://%1:%2/").arg(addr.toString()).arg(m_port);
        }
    }
    return result;
}

bool CastServer::start()
{
    if (running())
        return true;
    if (!m_server->listen(QHostAddress::AnyIPv4, quint16(m_port))) {
        m_lastError = m_server->errorString();
        emit lastErrorChanged();
        return false;
    }
    m_lastError.clear();
    emit lastErrorChanged();
    emit runningChanged();
    return true;
}

void CastServer::stop()
{
    if (!running())
        return;
    m_server->close();
    const auto sockets = m_pending.keys();
    for (QTcpSocket *socket : sockets)
        socket->disconnectFromHost();
    emit runningChanged();
}

void CastServer::handleNewConnection()
{
    while (QTcpSocket *socket = m_server->nextPendingConnection()) {
        m_pending.insert(socket, QByteArray());
        connect(socket, &QTcpSocket::readyRead, this, &CastServer::handleReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &CastServer::handleDisconnected);
        connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    }
}

void CastServer::handleDisconnected()
{
    auto *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket)
        m_pending.remove(socket);
}

void CastServer::handleReadyRead()
{
    auto *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket || !m_pending.contains(socket))
        return;

    QByteArray &buffer = m_pending[socket];
    buffer.append(socket->readAll());
    if (buffer.size() > 32 * 1024) { // a plain GET/HEAD never needs this much
        serveSimple(socket, 400, "Bad Request", "Request too large");
        return;
    }
    const int headerEnd = buffer.indexOf("\r\n\r\n");
    if (headerEnd < 0)
        return; // wait for more data

    const QByteArray head = buffer.left(headerEnd);
    m_pending.remove(socket); // one request per connection; no keep-alive/body handling
    disconnect(socket, &QTcpSocket::readyRead, this, &CastServer::handleReadyRead);
    dispatch(socket, head);
}

void CastServer::dispatch(QTcpSocket *socket, const QByteArray &head)
{
    const QList<QByteArray> lines = head.split('\n');
    if (lines.isEmpty()) {
        socket->disconnectFromHost();
        return;
    }
    const QList<QByteArray> requestLine = lines.first().trimmed().split(' ');
    if (requestLine.size() < 2) {
        serveSimple(socket, 400, "Bad Request", "Malformed request line");
        return;
    }
    const QByteArray method = requestLine.at(0);
    const QByteArray path = requestLine.at(1);
    if (method != "GET" && method != "HEAD") {
        serveSimple(socket, 405, "Method Not Allowed", "Only GET/HEAD are supported");
        return;
    }

    QByteArray rangeHeader;
    for (const QByteArray &line : lines) {
        if (line.startsWith("Range:") || line.startsWith("range:")) {
            rangeHeader = line.mid(6).trimmed();
            break;
        }
    }

    if (path == "/" || path.startsWith("/index"))
        serveIndex(socket);
    else if (path == "/stream")
        serveStream(socket, rangeHeader, method == "HEAD");
    else
        serveSimple(socket, 404, "Not Found", "Not found");
}

bool CastServer::canServeCurrentSource() const
{
    if (!m_controller || !m_controller->player())
        return false;
    QMediaPlayer *player = m_controller->player();
    if (player->sourceDevice()) // DVD titles, live TV: not plain local files
        return false;
    const QUrl url = player->source();
    return url.isLocalFile() && QFileInfo(url.toLocalFile()).isFile();
}

void CastServer::serveIndex(QTcpSocket *socket)
{
    if (!canServeCurrentSource()) {
        serveSimple(socket, 404, "Not Found",
                    "<!doctype html><meta charset=utf-8>"
                    "<p>Vivace isn't currently playing a local file.</p>");
        return;
    }
    // mediaTitle() only carries a friendly name primed by e.g. the YouTube
    // resolver; ordinary local playback leaves it empty (see
    // PlayerController::sourceChanged), so fall back to the file name, same
    // as the main window's own title.
    QString title = m_controller->mediaTitle();
    if (title.isEmpty())
        title = QFileInfo(m_controller->player()->source().toLocalFile()).fileName();
    QByteArray titleHtml = title.toHtmlEscaped().toUtf8();
    if (titleHtml.isEmpty())
        titleHtml = "Vivace";

    QByteArray body;
    QTextStream out(&body);
    out << "<!doctype html><html><head><meta charset=utf-8>"
        << "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
        << "<title>" << titleHtml << "</title>"
        << "<style>html,body{margin:0;background:#000;height:100%}"
        << "video{width:100%;height:100%;object-fit:contain;background:#000}</style>"
        << "</head><body>"
        << "<video src=\"/stream\" controls autoplay playsinline></video>"
        << "</body></html>";
    out.flush();

    QByteArray header;
    QTextStream h(&header);
    h << "HTTP/1.1 200 OK\r\n"
      << "Content-Type: text/html; charset=utf-8\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Connection: close\r\n\r\n";
    h.flush();
    socket->write(header);
    socket->write(body);
    socket->disconnectFromHost();
}

void CastServer::serveStream(QTcpSocket *socket, const QByteArray &rangeHeader, bool headOnly)
{
    if (!canServeCurrentSource()) {
        serveSimple(socket, 404, "Not Found", "No local file is currently playing");
        return;
    }
    const QString path = m_controller->player()->source().toLocalFile();
    const QFileInfo info(path);
    const qint64 size = info.size();
    const QByteArray mime =
            QMimeDatabase().mimeTypeForFile(info).name().toUtf8();

    qint64 start = 0;
    qint64 end = size - 1;
    bool partial = false;
    if (rangeHeader.startsWith("bytes=")) {
        const QByteArray spec = rangeHeader.mid(6);
        const int dash = spec.indexOf('-');
        if (dash >= 0) {
            const QByteArray startPart = spec.left(dash);
            const QByteArray endPart = spec.mid(dash + 1);
            bool ok = false;
            if (!startPart.isEmpty()) {
                const qint64 s = startPart.toLongLong(&ok);
                if (ok)
                    start = s;
            }
            if (!endPart.isEmpty()) {
                const qint64 e = endPart.toLongLong(&ok);
                if (ok)
                    end = e;
            }
            partial = true;
        }
    }
    start = qBound<qint64>(0, start, size > 0 ? size - 1 : 0);
    end = qBound<qint64>(start, end, size > 0 ? size - 1 : 0);
    const qint64 length = size > 0 ? (end - start + 1) : 0;

    QByteArray header;
    QTextStream h(&header);
    h << "HTTP/1.1 " << (partial ? "206 Partial Content" : "200 OK") << "\r\n"
      << "Content-Type: " << mime << "\r\n"
      << "Accept-Ranges: bytes\r\n"
      << "Content-Length: " << length << "\r\n";
    if (partial)
        h << "Content-Range: bytes " << start << "-" << end << "/" << size << "\r\n";
    h << "Connection: close\r\n\r\n";
    h.flush();
    socket->write(header);

    if (headOnly || length <= 0) {
        socket->disconnectFromHost();
        return;
    }
    streamFileRange(socket, path, start, length);
}

void CastServer::serveSimple(QTcpSocket *socket, int status, const QByteArray &statusText,
                              const QByteArray &body)
{
    QByteArray header;
    QTextStream h(&header);
    h << "HTTP/1.1 " << status << " " << statusText << "\r\n"
      << "Content-Type: text/html; charset=utf-8\r\n"
      << "Content-Length: " << body.size() << "\r\n"
      << "Connection: close\r\n\r\n";
    h.flush();
    socket->write(header);
    socket->write(body);
    socket->disconnectFromHost();
}

QString CastServer::qrCodeDataUrl(const QString &text) const
{
    const QByteArray utf8 = text.toUtf8();
    // qrcodegen_encodeText requires a mutable, null-terminated C string.
    std::vector<char> input(utf8.constData(), utf8.constData() + utf8.size() + 1);

    std::vector<uint8_t> qrcode(qrcodegen_BUFFER_LEN_MAX);
    std::vector<uint8_t> tempBuffer(qrcodegen_BUFFER_LEN_MAX);
    const bool ok = qrcodegen_encodeText(
            input.data(), tempBuffer.data(), qrcode.data(), qrcodegen_Ecc_LOW,
            qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
    if (!ok)
        return QString();

    const int modules = qrcodegen_getSize(qrcode.data());
    const int scale = 6;   // pixels per module
    const int quiet = 4;   // quiet-zone modules on each side (spec minimum)
    const int size = (modules + 2 * quiet) * scale;

    QImage image(size, size, QImage::Format_RGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    for (int y = 0; y < modules; ++y) {
        for (int x = 0; x < modules; ++x) {
            if (qrcodegen_getModule(qrcode.data(), x, y)) {
                painter.drawRect((quiet + x) * scale, (quiet + y) * scale, scale, scale);
            }
        }
    }
    painter.end();

    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return QStringLiteral("data:image/png;base64,") + QString::fromLatin1(png.toBase64());
}
