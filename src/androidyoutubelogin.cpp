/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "androidyoutubelogin.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QStandardPaths>

#ifdef VIVACE_HAVE_ANDROID_YOUTUBE_LOGIN
#include <QCoreApplication> // QNativeInterface::QAndroidApplication
#include <QJniObject>
#endif

namespace {
Q_LOGGING_CATEGORY(lcAndroidYtLogin, "vivace.androidyoutubelogin")
} // namespace

AndroidYoutubeLogin::AndroidYoutubeLogin(QObject *parent)
    : QObject(parent)
{
}

bool AndroidYoutubeLogin::isSupported()
{
#ifdef VIVACE_HAVE_ANDROID_YOUTUBE_LOGIN
    return true;
#else
    return false;
#endif
}

QString AndroidYoutubeLogin::plannedCookiesPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            + QStringLiteral("/youtube-login-cookies.txt");
}

#ifdef VIVACE_HAVE_ANDROID_YOUTUBE_LOGIN
namespace {

// Reads android.webkit.CookieManager's own Cookie-header-equivalent string
// for a given URL -- e.g. "SID=abc; HSID=def; APISID=ghi" -- via JNI. This
// is the ONLY way to get real cookie VALUES on Android: QtWebView's own QML
// API (WebView.cookieAdded(domain, name)) deliberately only exposes that a
// cookie changed, never its value (confirmed directly against Qt 6.11's own
// WebView QML docs before writing this). CookieManager.getInstance() needs
// the native WebView subsystem to already be initialized -- true by the
// time this is ever called, since it's only invoked after the user has
// interacted with a real qml/YoutubeLoginDialog.qml WebView instance.
//
// Deliberately NOT the JS-visible document.cookie (e.g. via WebView's own
// runJavaScript()) -- Android's native CookieManager.getCookie() returns
// EVERY cookie including HttpOnly ones (a plain native/privileged API, not
// subject to the JS-side HttpOnly restriction), and several of Google's own
// account-identity cookies (SID/HSID/SSID in particular) are HttpOnly --
// a JS-based read would silently miss exactly the cookies that matter most
// for authentication.
QString cookieHeaderForUrl(const QString &url)
{
    QJniObject cookieManager = QJniObject::callStaticObjectMethod(
            "android/webkit/CookieManager", "getInstance",
            "()Landroid/webkit/CookieManager;");
    if (!cookieManager.isValid())
        return QString();
    QJniObject urlObj = QJniObject::fromString(url);
    QJniObject cookieHeader = cookieManager.callObjectMethod(
            "getCookie", "(Ljava/lang/String;)Ljava/lang/String;",
            urlObj.object<jstring>());
    return cookieHeader.isValid() ? cookieHeader.toString() : QString();
}

// One "domain to ask Android's cookie jar about" -> "domain to write into
// the synthesized cookies.txt" pair. YouTube's own real login flow sets
// cookies under BOTH domains -- youtube.com itself (site-specific state:
// LOGIN_INFO, PREF, VISITOR_INFO1_LIVE, YSC, …) and google.com (Google's
// shared account-identity cookies: SID, HSID, SSID, APISID, SAPISID and
// their __Secure-* variants, used across every Google property) -- and
// they are UNRELATED registrable domains (not subdomains of one another),
// so a single CookieManager.getCookie() call against just one of them
// would miss the other entirely. Queried and written separately, each
// under its own correct domain, rather than guessed at.
struct DomainQuery
{
    QString queryUrl;
    QString cookieDomain; // leading '.' = applies to subdomains too (the
                           // standard Netscape-format convention)
};

const QList<DomainQuery> kDomainQueries = {
    { QStringLiteral("https://www.youtube.com"), QStringLiteral(".youtube.com") },
    { QStringLiteral("https://www.google.com"), QStringLiteral(".google.com") },
};

} // namespace
#endif // VIVACE_HAVE_ANDROID_YOUTUBE_LOGIN

void AndroidYoutubeLogin::saveCookies(const QString &destPath)
{
#ifndef VIVACE_HAVE_ANDROID_YOUTUBE_LOGIN
    Q_UNUSED(destPath);
    emit cookiesSaveFailed(tr("Embedded YouTube login is not available in this build."));
#else
    // Netscape cookie-jar format, the same format yt-dlp's own --cookies
    // (and cookiefile= in PythonYoutubeResolver's Python-API path) expect.
    // Real per-cookie domain/path/secure/expiry attributes are NOT exposed
    // by CookieManager.getCookie() at all -- it only ever returns a flat
    // "name=value; name2=value2" string, the same shape as the HTTP Cookie
    // request header -- so every field below except domain/name/value is a
    // reasonable, honestly-approximated default rather than the cookie's
    // real, original attribute:
    //  - path "/": true for essentially every cookie that matters for
    //    yt-dlp's own requests, which are all to site roots or API
    //    endpoints already covered by "/".
    //  - secure TRUE: the login flow only ever runs over https, and yt-dlp
    //    itself only ever requests https URLs, so this is never wrong in
    //    practice even where the original cookie wasn't marked Secure.
    //  - expiry 2147483647 (2038-01-19, the classic 32-bit time_t max): a
    //    deliberately far-future, maximally-compatible "never expires"
    //    sentinel -- some cookie-jar implementations mishandle a real
    //    post-2038 timestamp on 32-bit systems, so this is the safest
    //    universal choice, not a guess at the cookie's real expiration
    //    (which CookieManager doesn't expose anyway).
    QStringList lines;
    lines << QStringLiteral("# Netscape HTTP Cookie File")
          << QStringLiteral("# Written by Vivace's embedded YouTube login -- see "
                             "Preferences > Network > YouTube.");

    int count = 0;
    for (const auto &query : kDomainQueries) {
        const QString header = cookieHeaderForUrl(query.queryUrl);
        if (header.isEmpty())
            continue;
        const QStringList pairs = header.split(QStringLiteral("; "), Qt::SkipEmptyParts);
        for (const QString &pair : pairs) {
            const int eq = pair.indexOf(QLatin1Char('='));
            if (eq <= 0)
                continue; // malformed / not a real name=value pair
            const QString name = pair.left(eq);
            const QString value = pair.mid(eq + 1);
            lines << QStringLiteral("%1\tTRUE\t/\tTRUE\t2147483647\t%2\t%3")
                             .arg(query.cookieDomain, name, value);
            ++count;
        }
    }

    if (count == 0) {
        qCWarning(lcAndroidYtLogin) << "no cookies found for youtube.com/google.com -- "
                                        "did the user actually finish signing in?";
        emit cookiesSaveFailed(tr("No YouTube login cookies were found. Make sure you "
                                   "finished signing in before closing the login window."));
        return;
    }

    QDir().mkpath(QFileInfo(destPath).absolutePath());
    QFile file(destPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCWarning(lcAndroidYtLogin) << "could not write" << destPath << ":" << file.errorString();
        emit cookiesSaveFailed(tr("Could not write %1: %2").arg(destPath, file.errorString()));
        return;
    }
    file.write(lines.join(QLatin1Char('\n')).toUtf8());
    file.write("\n");
    file.close();

    qCInfo(lcAndroidYtLogin) << "wrote" << count << "cookies to" << destPath;
    emit cookiesSaved(count);
#endif // VIVACE_HAVE_ANDROID_YOUTUBE_LOGIN
}
