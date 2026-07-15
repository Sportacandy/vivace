/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Applies the Preferences > Network proxy settings application-wide.
    Instantiated once in Main.qml with its properties bound to Settings
    (settings.enabled: Settings.proxyEnabled, ...), so it re-applies live
    whenever the user edits a field, like the rest of Vivace's instant-apply
    preferences.

    Two independent mechanisms, because Vivace has no single networking
    stack:
      - QNetworkProxy::setApplicationProxy() covers everything that goes
        through QNetworkAccessManager (OpenSubtitles search, the update
        checker) — both proxy types.
      - http_proxy/https_proxy environment variables cover the FFmpeg-backend
        media network code (in-process; QMediaPlayer's playback of http/https
        streams does not go through QNetworkAccessManager and ignores
        QNetworkProxy) and the yt-dlp child process (which inherits this
        process's environment and honours the same convention itself). Only
        the HTTP proxy type maps to this convention — there is no equivalent
        for SOCKS5, so a SOCKS5 proxy does not reach playback or yt-dlp.
*/

#ifndef NETWORKPROXY_H
#define NETWORKPROXY_H

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

class NetworkProxyController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY configChanged)
    Q_PROPERTY(int type READ type WRITE setType NOTIFY configChanged)
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY configChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY configChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername
                       NOTIFY configChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword
                       NOTIFY configChanged)

public:
    explicit NetworkProxyController(QObject *parent = nullptr);

    bool enabled() const { return m_enabled; }
    void setEnabled(bool enabled);
    int type() const { return m_type; }
    void setType(int type);
    QString host() const { return m_host; }
    void setHost(const QString &host);
    int port() const { return m_port; }
    void setPort(int port);
    QString username() const { return m_username; }
    void setUsername(const QString &username);
    QString password() const { return m_password; }
    void setPassword(const QString &password);

signals:
    void configChanged();

private:
    void apply();

    bool m_enabled = false;
    int m_type = 0; // 0 = HTTP, 1 = SOCKS5
    QString m_host;
    int m_port = 0;
    QString m_username;
    QString m_password;
};

#endif // NETWORKPROXY_H
