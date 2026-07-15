/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef HTTPTSSOURCE_H
#define HTTPTSSOURCE_H

#include <QObject>
#include <QUrl>

class LiveStreamDevice;
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

/*  Reads an HTTP MPEG-TS live stream (e.g. an EDCB / EpgDataCap_Bon TV tuner)
    and feeds clean transport-stream bytes into a LiveStreamDevice that
    QMediaPlayer consumes via setSourceDevice().

    Live tuners keep the connection alive with padding until the channel locks,
    then send real TS. QMediaPlayer's FFmpeg backend gives up analysing before
    the real data arrives, so we do the reading ourselves: hold the connection
    through the whole warm-up (no premature give-up) and strip everything before
    the first TS sync byte, so the backend sees valid TS from byte 0.

    We only take over when the response really is MPEG-TS (Content-Type
    video/mp2t); anything else (HLS, mp4, …) is signalled back so the caller
    plays the URL directly. A browser-agnostic User-Agent ("Lavf/…") is sent
    because some tuners only serve the real stream to FFmpeg-family clients. */
class HttpTsSource : public QObject
{
    Q_OBJECT
public:
    explicit HttpTsSource(QObject *parent = nullptr);
    ~HttpTsSource() override;

    void start(const QUrl &url);
    LiveStreamDevice *device() const { return m_device; }

signals:
    void tsConfirmed();   // it's MPEG-TS: device() is ready for setSourceDevice()
    void notTsStream();   // not TS: caller should play the URL directly
    void failed(const QString &message);

private:
    void onReadyRead();
    void onFinished();
    void pump(); // move reply bytes into the device, respecting backpressure

    QNetworkAccessManager *m_nam;
    QNetworkReply *m_reply = nullptr;
    LiveStreamDevice *m_device;
    QTimer *m_pumpTimer;      // resumes feeding as the device drains
    bool m_decided = false;   // Content-Type inspected
    bool m_isTs = false;
    bool m_syncFound = false; // first 0x47 (TS sync) seen
    bool m_gotData = false;
};

#endif // HTTPTSSOURCE_H
