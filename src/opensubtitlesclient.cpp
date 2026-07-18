/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "opensubtitlesclient.h"

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTemporaryFile>
#include <QUrl>
#include <QUrlQuery>

namespace {
constexpr auto kApiBase = "https://api.opensubtitles.com/api/v1";
const QByteArray kUserAgent = "Vivace v0.1.1";
} // namespace

OpenSubtitlesClient::OpenSubtitlesClient(QObject *parent) : QObject(parent) {}

void OpenSubtitlesClient::setApiKey(const QString &key)
{
    if (key == m_apiKey)
        return;
    m_apiKey = key;
    m_token.clear(); // key change invalidates any session
    emit credentialsChanged();
}

void OpenSubtitlesClient::setUsername(const QString &user)
{
    if (user == m_username)
        return;
    m_username = user;
    m_token.clear();
    emit credentialsChanged();
}

void OpenSubtitlesClient::setPassword(const QString &pass)
{
    if (pass == m_password)
        return;
    m_password = pass;
    m_token.clear();
    emit credentialsChanged();
}

void OpenSubtitlesClient::setBusy(bool busy)
{
    if (busy == m_busy)
        return;
    m_busy = busy;
    emit busyChanged();
}

void OpenSubtitlesClient::setStatus(const QString &status)
{
    m_status = status;
    emit statusChanged();
}

void OpenSubtitlesClient::clearResults()
{
    if (m_results.isEmpty())
        return;
    m_results.clear();
    emit resultsChanged();
}

// ---- movie hash (filesize + first & last 64 KiB as little-endian u64) -------

QString OpenSubtitlesClient::hashFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    const quint64 size = quint64(file.size());
    if (size < 65536)
        return {}; // too small to hash meaningfully

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);
    quint64 hash = size;
    quint64 word = 0;
    for (int i = 0; i < 8192; ++i) {
        in >> word;
        hash += word;
    }
    file.seek(qint64(size - 65536));
    for (int i = 0; i < 8192; ++i) {
        in >> word;
        hash += word;
    }
    return QStringLiteral("%1").arg(hash, 16, 16, QChar('0'));
}

// ---- REST helpers ----------------------------------------------------------

QNetworkRequest OpenSubtitlesClient::apiRequest(const QString &path) const
{
    QNetworkRequest req(QUrl(QString::fromLatin1(kApiBase) + path));
    req.setRawHeader("Api-Key", m_apiKey.toUtf8());
    req.setRawHeader("User-Agent", kUserAgent);
    req.setRawHeader("Accept", "application/json");
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return req;
}

// ---- search ----------------------------------------------------------------

void OpenSubtitlesClient::search(const QString &filePath, const QString &query,
                                 const QString &languages)
{
    if (!configured()) {
        setStatus(tr("Set your OpenSubtitles API key in Preferences > Network."));
        return;
    }
    clearResults();

    QUrlQuery q;
    if (!languages.isEmpty()) {
        // Normalise "ja, en" -> "en,ja" (the API wants lowercase, sorted, no
        // spaces).
        QStringList langs = languages.toLower().remove(' ').split(
            ',', Qt::SkipEmptyParts);
        langs.sort();
        q.addQueryItem(QStringLiteral("languages"), langs.join(','));
    }
    const QString hash = filePath.isEmpty() ? QString() : hashFile(filePath);
    if (!hash.isEmpty())
        q.addQueryItem(QStringLiteral("moviehash"), hash);
    if (!query.isEmpty())
        q.addQueryItem(QStringLiteral("query"), query.toLower());
    if (hash.isEmpty() && query.isEmpty()) {
        setStatus(tr("Enter a search term (the file is too small to hash)."));
        return;
    }

    QNetworkRequest req = apiRequest(QStringLiteral("/subtitles?") + q.toString());
    setBusy(true);
    setStatus(tr("Searching…"));
    QNetworkReply *reply = m_nam.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        setBusy(false);
        if (reply->error() != QNetworkReply::NoError) {
            setStatus(tr("Search failed: %1").arg(reply->errorString()));
            return;
        }
        const QJsonObject root =
            QJsonDocument::fromJson(reply->readAll()).object();
        const QJsonArray data = root.value(QStringLiteral("data")).toArray();
        QVariantList rows;
        for (const QJsonValue &v : data) {
            const QJsonObject attr = v.toObject().value(QStringLiteral("attributes"))
                                         .toObject();
            const QJsonArray files = attr.value(QStringLiteral("files")).toArray();
            if (files.isEmpty())
                continue;
            const QJsonObject file = files.first().toObject();
            rows << QVariantMap {
                { QStringLiteral("language"),
                  attr.value(QStringLiteral("language")).toString() },
                { QStringLiteral("release"),
                  attr.value(QStringLiteral("release")).toString() },
                { QStringLiteral("fileName"),
                  file.value(QStringLiteral("file_name")).toString() },
                { QStringLiteral("downloads"),
                  attr.value(QStringLiteral("download_count")).toInt() },
                { QStringLiteral("rating"),
                  attr.value(QStringLiteral("ratings")).toDouble() },
                { QStringLiteral("fileId"),
                  QString::number(file.value(QStringLiteral("file_id")).toInt()) }
            };
        }
        m_results = rows;
        emit resultsChanged();
        setStatus(rows.isEmpty() ? tr("No subtitles found.")
                                 : tr("%n result(s).", "", int(rows.size())));
    });
}

// ---- download (login if needed -> get link -> fetch & save) ----------------

void OpenSubtitlesClient::download(const QString &fileId, const QString &videoPath)
{
    if (!configured()) {
        setStatus(tr("Set your OpenSubtitles API key in Preferences > Network."));
        return;
    }
    // A login token raises the download quota; anonymous downloads work with
    // just the Api-Key but are very limited.
    if (m_token.isEmpty() && !m_username.isEmpty() && !m_password.isEmpty())
        loginThenDownload(fileId, videoPath);
    else
        requestDownloadLink(fileId, videoPath);
}

void OpenSubtitlesClient::loginThenDownload(const QString &fileId,
                                            const QString &videoPath)
{
    QJsonObject body;
    body.insert(QStringLiteral("username"), m_username);
    body.insert(QStringLiteral("password"), m_password);
    setBusy(true);
    setStatus(tr("Signing in…"));
    QNetworkReply *reply = m_nam.post(apiRequest(QStringLiteral("/login")),
                                      QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, fileId, videoPath]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            setBusy(false);
            setStatus(tr("Sign-in failed: %1").arg(reply->errorString()));
            return;
        }
        const QJsonObject obj =
            QJsonDocument::fromJson(reply->readAll()).object();
        m_token = obj.value(QStringLiteral("token")).toString().toUtf8();
        requestDownloadLink(fileId, videoPath);
    });
}

void OpenSubtitlesClient::requestDownloadLink(const QString &fileId,
                                              const QString &videoPath)
{
    QJsonObject body;
    body.insert(QStringLiteral("file_id"), fileId.toInt());
    QNetworkRequest req = apiRequest(QStringLiteral("/download"));
    if (!m_token.isEmpty())
        req.setRawHeader("Authorization", "Bearer " + m_token);
    setBusy(true);
    setStatus(tr("Requesting download…"));
    QNetworkReply *reply = m_nam.post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, videoPath]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            setBusy(false);
            setStatus(tr("Download failed: %1").arg(reply->errorString()));
            return;
        }
        const QJsonObject obj =
            QJsonDocument::fromJson(reply->readAll()).object();
        const QUrl link(obj.value(QStringLiteral("link")).toString());
        const QString name = obj.value(QStringLiteral("file_name")).toString();
        if (!link.isValid()) {
            setBusy(false);
            setStatus(tr("The server did not return a download link."));
            return;
        }
        fetchAndSave(link, videoPath, name);
    });
}

void OpenSubtitlesClient::fetchAndSave(const QUrl &link, const QString &videoPath,
                                       const QString &suggestedName)
{
    QNetworkReply *reply = m_nam.get(QNetworkRequest(link));
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, videoPath, suggestedName]() {
                reply->deleteLater();
                setBusy(false);
                if (reply->error() != QNetworkReply::NoError) {
                    setStatus(tr("Download failed: %1").arg(reply->errorString()));
                    return;
                }
                const QByteArray data = reply->readAll();

                // Prefer saving next to the video (so it auto-loads next time);
                // fall back to a temp file.
                QString suffix = QFileInfo(suggestedName).suffix();
                if (suffix.isEmpty())
                    suffix = QStringLiteral("srt");
                QString path;
                const QFileInfo vi(videoPath);
                if (!videoPath.isEmpty() && vi.dir().exists()) {
                    path = vi.dir().filePath(vi.completeBaseName() + '.' + suffix);
                }
                if (path.isEmpty() || !QFileInfo(path).dir().exists()) {
                    QTemporaryFile tmp(QDir::tempPath()
                                       + QStringLiteral("/vivace_XXXXXX.") + suffix);
                    tmp.setAutoRemove(false);
                    if (tmp.open()) {
                        tmp.write(data);
                        path = tmp.fileName();
                    }
                } else {
                    QFile f(path);
                    if (f.open(QIODevice::WriteOnly))
                        f.write(data);
                    else
                        path.clear();
                }

                if (path.isEmpty()) {
                    setStatus(tr("Could not save the subtitle file."));
                    return;
                }
                setStatus(tr("Subtitle downloaded."));
                emit subtitleReady(QUrl::fromLocalFile(path));
            });
}
