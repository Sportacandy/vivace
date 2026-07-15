/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "httptssource.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

#include "livestreamdevice.h"

namespace {
// Stop feeding the device once this far ahead of the reader (a paused stream);
// resume when it drains. Bounds the unread buffer.
constexpr qint64 kHighWater = 24 * 1024 * 1024;
// Cap the reply's own buffer so, when we stop draining it, TCP backpressure
// throttles the server instead of Qt buffering without limit.
constexpr qint64 kReplyBuffer = 4 * 1024 * 1024;
constexpr qint64 kPumpChunk = 256 * 1024;
} // namespace

HttpTsSource::HttpTsSource(QObject *parent)
    : QObject(parent),
      m_nam(new QNetworkAccessManager(this)),
      m_device(new LiveStreamDevice(this)),
      m_pumpTimer(new QTimer(this))
{
    m_device->open(QIODevice::ReadOnly);
    // While backpressured we stop getting readyRead (data sits in the reply),
    // so poll to resume feeding as the device drains.
    m_pumpTimer->setInterval(100);
    connect(m_pumpTimer, &QTimer::timeout, this, &HttpTsSource::pump);
}

HttpTsSource::~HttpTsSource()
{
    m_device->abort(); // wake any blocked backend read before teardown
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
    }
}

void HttpTsSource::start(const QUrl &url)
{
    QNetworkRequest req(url);
    // Mimic an FFmpeg-family client; some tuners only stream to those.
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QStringLiteral("Lavf/61.7.100"));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                     QNetworkRequest::AlwaysNetwork);
    m_reply = m_nam->get(req);
    m_reply->setReadBufferSize(kReplyBuffer); // enables TCP backpressure
    connect(m_reply, &QIODevice::readyRead, this, &HttpTsSource::onReadyRead);
    connect(m_reply, &QNetworkReply::finished, this, &HttpTsSource::onFinished);
}

void HttpTsSource::onReadyRead()
{
    if (!m_decided) {
        m_decided = true;
        const QString ct = m_reply->header(QNetworkRequest::ContentTypeHeader)
                                   .toString()
                                   .toLower();
        m_isTs = ct.contains(QLatin1String("mp2t"))
                 || ct.contains(QLatin1String("mpegts"))
                 || ct.contains(QLatin1String("mpeg-ts"));
        if (!m_isTs) {
            m_reply->abort(); // hand back to the caller for direct playback
            emit notTsStream();
            return;
        }
        emit tsConfirmed(); // caller wires setSourceDevice(device()) now
    }
    if (m_isTs)
        pump();
}

void HttpTsSource::pump()
{
    if (!m_reply || !m_isTs)
        return;
    // Feed while the reader isn't already far ahead (backpressure).
    while (m_reply->bytesAvailable() > 0
           && m_device->bufferedAhead() < kHighWater) {
        const QByteArray data = m_reply->read(kPumpChunk);
        if (data.isEmpty())
            break;
        if (!m_syncFound) {
            // Discard the leading keep-alive padding; real TS begins at 0x47.
            const int idx = data.indexOf('\x47');
            if (idx < 0)
                continue;
            m_syncFound = true;
            m_gotData = true;
            m_device->appendData(data.mid(idx));
        } else {
            m_device->appendData(data);
        }
    }
    // If data is still waiting but the reader is full, poll until it drains.
    if (m_reply->bytesAvailable() > 0 && !m_pumpTimer->isActive())
        m_pumpTimer->start();
    else if (m_reply->bytesAvailable() == 0 && m_pumpTimer->isActive())
        m_pumpTimer->stop();
}

void HttpTsSource::onFinished()
{
    m_pumpTimer->stop();
    m_device->finish(); // no more data — let the backend reach EOF
    if (m_isTs && !m_gotData) {
        const QString err = m_reply ? m_reply->errorString() : QString();
        emit failed(err);
    }
}
