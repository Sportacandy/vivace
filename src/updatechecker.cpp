/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "updatechecker.h"

#include <QCoreApplication>
#include <QDate>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>

namespace {
// PLACEHOLDER endpoints. The .invalid TLD (RFC 6761) never resolves, so an
// automatic check simply fails silently until these are pointed at the real
// version-info document and release page.
constexpr auto kDefaultCheckUrl = "https://updates.vivace-player.invalid/latest.json";
constexpr auto kDefaultDownloadUrl = "https://vivace-player.invalid/download";

// Which platform key to read from the version document.
#if defined(Q_OS_WIN)
constexpr auto kPlatformKey = "windows";
#elif defined(Q_OS_MACOS)
constexpr auto kPlatformKey = "macos";
#else
constexpr auto kPlatformKey = "linux";
#endif
} // namespace

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent),
      m_currentVersion(QCoreApplication::applicationVersion()),
      m_checkUrl(QUrl(QString::fromLatin1(kDefaultCheckUrl))),
      m_downloadUrl(QUrl(QString::fromLatin1(kDefaultDownloadUrl)))
{
}

void UpdateChecker::setCheckUrl(const QUrl &url)
{
    if (url == m_checkUrl)
        return;
    m_checkUrl = url;
    emit checkUrlChanged();
}

void UpdateChecker::setDownloadUrl(const QUrl &url)
{
    if (url == m_downloadUrl)
        return;
    m_downloadUrl = url;
    emit downloadUrlChanged();
}

void UpdateChecker::check(bool userInitiated)
{
    if (m_busy)
        return;
    m_busy = true;
    emit busyChanged();

    QNetworkRequest req(m_checkUrl);
    req.setRawHeader("User-Agent", "Vivace");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = m_net.get(req);
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, userInitiated] { onReply(reply, userInitiated); });
}

void UpdateChecker::onReply(QNetworkReply *reply, bool userInitiated)
{
    reply->deleteLater();
    m_busy = false;
    emit busyChanged();

    if (reply->error() != QNetworkReply::NoError) {
        if (userInitiated)
            emit errorOccurred(reply->errorString());
        emit checkFinished(false);
        return;
    }

    QString foundDownloadUrl;
    const QString latest = parseVersion(reply->readAll(), &foundDownloadUrl);
    if (latest.isEmpty()) {
        if (userInitiated)
            emit errorOccurred(tr("Could not read the latest version number."));
        emit checkFinished(false);
        return;
    }

    if (!foundDownloadUrl.isEmpty())
        setDownloadUrl(QUrl(foundDownloadUrl));

    if (compareVersions(latest, m_currentVersion) > 0)
        emit newVersionFound(latest);
    else if (userInitiated)
        emit upToDate(latest);
    emit checkFinished(true);
}

// Accepts a small JSON document, either flat ({ "version": "x.y.z" }) or with
// per-platform objects ({ "windows": { "version": "x.y.z", "url": "…" } }).
// Both "version" and "stable" are accepted for the version field.
QString UpdateChecker::parseVersion(const QByteArray &data, QString *downloadUrl)
{
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
        return QString();
    const QJsonObject root = doc.object();

    QJsonObject scope = root;
    if (root.value(QString::fromLatin1(kPlatformKey)).isObject())
        scope = root.value(QString::fromLatin1(kPlatformKey)).toObject();

    QString version = scope.value(QStringLiteral("version")).toString();
    if (version.isEmpty())
        version = scope.value(QStringLiteral("stable")).toString();

    if (downloadUrl) {
        QString url = scope.value(QStringLiteral("url")).toString();
        if (url.isEmpty())
            url = root.value(QStringLiteral("url")).toString();
        *downloadUrl = url;
    }
    return version;
}

bool UpdateChecker::isCheckDue(const QString &lastCheckIso, int intervalDays) const
{
    const QDate last = QDate::fromString(lastCheckIso, Qt::ISODate);
    if (!last.isValid())
        return true; // never checked
    return last.daysTo(QDate::currentDate()) >= intervalDays;
}

// Pad each of the first four numeric components so a plain string comparison
// orders versions correctly (mirrors SMPlayer's formattedVersion).
QString UpdateChecker::formattedVersion(const QString &version)
{
    int n[4] = {0, 0, 0, 0};
    const QStringList parts = version.split(QLatin1Char('.'));
    for (int i = 0; i < 4 && i < parts.size(); ++i)
        n[i] = parts.at(i).toInt();
    return QStringLiteral("%1.%2.%3.%4")
            .arg(n[0], 2, 10, QLatin1Char('0'))
            .arg(n[1], 2, 10, QLatin1Char('0'))
            .arg(n[2], 2, 10, QLatin1Char('0'))
            .arg(n[3], 5, 10, QLatin1Char('0'));
}

int UpdateChecker::compareVersions(const QString &a, const QString &b)
{
    return formattedVersion(a).compare(formattedVersion(b));
}
