/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "networkproxy.h"

#include <QNetworkProxy>
#include <QUrl>

NetworkProxyController::NetworkProxyController(QObject *parent)
    : QObject(parent)
{
}

void NetworkProxyController::setEnabled(bool enabled)
{
    if (enabled == m_enabled)
        return;
    m_enabled = enabled;
    emit configChanged();
    apply();
}

void NetworkProxyController::setType(int type)
{
    if (type == m_type)
        return;
    m_type = type;
    emit configChanged();
    apply();
}

void NetworkProxyController::setHost(const QString &host)
{
    if (host == m_host)
        return;
    m_host = host;
    emit configChanged();
    apply();
}

void NetworkProxyController::setPort(int port)
{
    if (port == m_port)
        return;
    m_port = port;
    emit configChanged();
    apply();
}

void NetworkProxyController::setUsername(const QString &username)
{
    if (username == m_username)
        return;
    m_username = username;
    emit configChanged();
    apply();
}

void NetworkProxyController::setPassword(const QString &password)
{
    if (password == m_password)
        return;
    m_password = password;
    emit configChanged();
    apply();
}

void NetworkProxyController::apply()
{
    if (!m_enabled || m_host.isEmpty()) {
        QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
        qunsetenv("http_proxy");
        qunsetenv("https_proxy");
        return;
    }

    // Covers everything using QNetworkAccessManager (OpenSubtitles search,
    // the update checker) for both proxy types.
    const auto qtType = m_type == 1 ? QNetworkProxy::Socks5Proxy
                                    : QNetworkProxy::HttpProxy;
    QNetworkProxy proxy(qtType, m_host, quint16(m_port), m_username,
                        m_password);
    QNetworkProxy::setApplicationProxy(proxy);

    if (m_type == 0) {
        // HTTP only: also export the http_proxy/https_proxy convention that
        // the FFmpeg-backend media network code and yt-dlp (a child process
        // that inherits this environment) read directly. There is no
        // equivalent env-var convention for a SOCKS5 proxy.
        QUrl url;
        url.setScheme(QStringLiteral("http"));
        url.setHost(m_host);
        if (m_port > 0)
            url.setPort(m_port);
        if (!m_username.isEmpty())
            url.setUserName(m_username);
        if (!m_password.isEmpty())
            url.setPassword(m_password);
        const QByteArray proxyUrl = url.toString(QUrl::FullyEncoded).toUtf8();
        qputenv("http_proxy", proxyUrl);
        qputenv("https_proxy", proxyUrl);
    } else {
        qunsetenv("http_proxy");
        qunsetenv("https_proxy");
    }
}
