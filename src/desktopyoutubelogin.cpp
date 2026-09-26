/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "desktopyoutubelogin.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QStandardPaths>

#ifdef VIVACE_HAVE_DESKTOP_YOUTUBE_LOGIN
#include <QQuickWebEngineProfile>
#include <QWebEngineCookieStore>
#endif

namespace {
Q_LOGGING_CATEGORY(lcDesktopYtLogin, "vivace.desktopyoutubelogin")
} // namespace

DesktopYoutubeLogin::DesktopYoutubeLogin(QObject *parent)
    : QObject(parent)
{
#ifdef VIVACE_HAVE_DESKTOP_YOUTUBE_LOGIN
    // The shared default profile is what qml/desktop/YoutubeLoginDialog.qml's
    // WebEngineView uses too (it sets no `profile:` of its own -- see that
    // file's own comment): "Each web engine view has an associated profile.
    // Views that do not have a specific profile set share a common one,
    // which is off-the-record by default" (QQuickWebEngineProfile's own
    // docs). Off-the-record means nothing persists to disk -- fine here,
    // since this class keeps its own in-memory copy of every cookie via the
    // signals below and writes cookies.txt itself; it also means Vivace
    // never leaves a stray Chromium profile directory behind. Connected
    // once, at construction (app startup), not per-dialog-open, so a cookie
    // set on an EARLIER login attempt this session (e.g. before the user
    // closed the dialog without clicking Save) is never lost.
    auto *cookieStore = QQuickWebEngineProfile::defaultProfile()->cookieStore();
    connect(cookieStore, &QWebEngineCookieStore::cookieAdded,
            this, &DesktopYoutubeLogin::onCookieAdded);
    connect(cookieStore, &QWebEngineCookieStore::cookieRemoved,
            this, &DesktopYoutubeLogin::onCookieRemoved);
    cookieStore->loadAllCookies();
#endif
}

bool DesktopYoutubeLogin::isSupported()
{
#ifdef VIVACE_HAVE_DESKTOP_YOUTUBE_LOGIN
    return true;
#else
    return false;
#endif
}

QString DesktopYoutubeLogin::plannedCookiesPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            + QStringLiteral("/youtube-login-cookies-desktop.txt");
}

#ifdef VIVACE_HAVE_DESKTOP_YOUTUBE_LOGIN

QString DesktopYoutubeLogin::cookieKey(const QNetworkCookie &cookie)
{
    // domain+path+name uniquely identifies "the same cookie slot" in a real
    // cookie jar -- re-adding a cookie with the same key (e.g. a refreshed
    // session token) should REPLACE the earlier entry, not accumulate a
    // stale duplicate alongside it.
    return cookie.domain() + QLatin1Char('\t') + cookie.path() + QLatin1Char('\t')
            + QString::fromUtf8(cookie.name());
}

void DesktopYoutubeLogin::onCookieAdded(const QNetworkCookie &cookie)
{
    m_cookies.insert(cookieKey(cookie), cookie);
}

void DesktopYoutubeLogin::onCookieRemoved(const QNetworkCookie &cookie)
{
    m_cookies.remove(cookieKey(cookie));
}

#endif // VIVACE_HAVE_DESKTOP_YOUTUBE_LOGIN

void DesktopYoutubeLogin::reset()
{
#ifdef VIVACE_HAVE_DESKTOP_YOUTUBE_LOGIN
    m_cookies.clear();
    QQuickWebEngineProfile::defaultProfile()->cookieStore()->loadAllCookies();
#endif
}

void DesktopYoutubeLogin::saveCookies(const QString &destPath)
{
#ifndef VIVACE_HAVE_DESKTOP_YOUTUBE_LOGIN
    Q_UNUSED(destPath);
    emit cookiesSaveFailed(tr("Embedded YouTube login is not available in this build."));
#else
    // Netscape cookie-jar format, the same format yt-dlp's own --cookies
    // (and cookiefile= in PythonYoutubeResolver's Python-API path) expect --
    // see AndroidYoutubeLogin::saveCookies()'s own comment for the format
    // itself. Unlike Android's necessarily-approximated fields (its
    // CookieManager only ever returns a flat name=value string), every
    // field here is the cookie's REAL attribute, since QWebEngineCookieStore
    // hands us genuine QNetworkCookie objects.
    //
    // Scope limited to youtube.com/google.com, same two (unrelated,
    // non-subdomain) registrable domains AndroidYoutubeLogin restricts
    // itself to and for the identical reason: YouTube's own login flow sets
    // site-specific cookies under youtube.com (LOGIN_INFO, PREF,
    // VISITOR_INFO1_LIVE, YSC, …) and Google's shared account-identity
    // cookies (SID, HSID, SSID, APISID, SAPISID and their __Secure-*
    // variants) under google.com -- both matter, and nothing else the user
    // might incidentally navigate to inside the same embedded browser
    // should end up in the file.
    QStringList lines;
    lines << QStringLiteral("# Netscape HTTP Cookie File")
          << QStringLiteral("# Written by Vivace's embedded YouTube login -- see "
                             "Preferences > Network > YouTube.");

    int count = 0;
    for (const QNetworkCookie &cookie : std::as_const(m_cookies)) {
        const QString domain = cookie.domain();
        if (!domain.endsWith(QStringLiteral("youtube.com"), Qt::CaseInsensitive)
                && !domain.endsWith(QStringLiteral("google.com"), Qt::CaseInsensitive))
            continue;

        const QString includeSubdomains = domain.startsWith(QLatin1Char('.'))
                ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
        const QString secure = cookie.isSecure() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
        const qint64 expiry = cookie.isSessionCookie()
                ? 0 : cookie.expirationDate().toSecsSinceEpoch();
        const QString path = cookie.path().isEmpty() ? QStringLiteral("/") : cookie.path();

        lines << QStringLiteral("%1\t%2\t%3\t%4\t%5\t%6\t%7")
                         .arg(domain, includeSubdomains, path, secure)
                         .arg(expiry)
                         .arg(QString::fromUtf8(cookie.name()), QString::fromUtf8(cookie.value()));
        ++count;
    }

    if (count == 0) {
        qCWarning(lcDesktopYtLogin) << "no cookies found for youtube.com/google.com -- "
                                        "did the user actually finish signing in?";
        emit cookiesSaveFailed(tr("No YouTube login cookies were found. Make sure you "
                                   "finished signing in before closing the login window."));
        return;
    }

    QDir().mkpath(QFileInfo(destPath).absolutePath());
    QFile file(destPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCWarning(lcDesktopYtLogin) << "could not write" << destPath << ":" << file.errorString();
        emit cookiesSaveFailed(tr("Could not write %1: %2").arg(destPath, file.errorString()));
        return;
    }
    file.write(lines.join(QLatin1Char('\n')).toUtf8());
    file.write("\n");
    file.close();

    qCInfo(lcDesktopYtLogin) << "wrote" << count << "cookies to" << destPath;
    emit cookiesSaved(count);
#endif // VIVACE_HAVE_DESKTOP_YOUTUBE_LOGIN
}
