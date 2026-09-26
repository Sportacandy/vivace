/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Embedded YouTube login (Windows/Linux/macOS, VIVACE_HAVE_DESKTOP_YOUTUBE_LOGIN
    -- via QtWebEngine). Desktop counterpart to AndroidYoutubeLogin: hosts a
    real Chromium-backed WebEngineView (qml/desktop/YoutubeLoginDialog.qml)
    the user signs into normally, then writes the resulting session cookies
    as a synthesized Netscape-format cookies.txt, wired into the same
    Settings.youtubeCookiesFile both YoutubeResolver and PythonYoutubeResolver
    already read -- the same "no separate export/transfer step" convenience
    Android's own embedded login already provides, plus a real fallback for
    Windows Chrome/Edge users, where the existing "Get cookies from browser"
    feature (YoutubeResolver::cookiesFromBrowser) doesn't work at all (Chrome/
    Edge's own App-Bound Encryption blocks every external tool from reading
    their cookie store, Firefox-on-Windows and every browser on Linux/macOS
    are unaffected there).

    Mechanism differs from Android's: Android's android.webkit.CookieManager
    offers a synchronous "give me the cookie string for this URL" query, so
    AndroidYoutubeLogin just asks it once, on demand, when the user clicks
    Save. QWebEngineCookieStore (desktop) has no equivalent synchronous
    query at all -- only an asynchronous cookieAdded/cookieRemoved signal
    pair plus loadAllCookies() (see the .cpp's own comment) -- so this class
    listens live, from construction (app startup) onward, and accumulates
    real QNetworkCookie objects (with their genuine domain/path/secure/
    expiry attributes, unlike Android's necessarily-approximated ones) as
    the user navigates the login flow.

    A complete no-op (isSupported() returns false, saveCookies() always
    fails) on every non-desktop build, or a desktop build without QtWebEngine
    linked -- the same pattern AndroidYoutubeLogin/PythonYoutubeResolver
    already establish for "this whole feature may not exist in this build,"
    so QML can check it unconditionally.
*/

#ifndef DESKTOPYOUTUBELOGIN_H
#define DESKTOPYOUTUBELOGIN_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

#ifdef VIVACE_HAVE_DESKTOP_YOUTUBE_LOGIN
#include <QNetworkCookie>
#endif

class DesktopYoutubeLogin : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit DesktopYoutubeLogin(QObject *parent = nullptr);

    // True only on a Windows/Linux/macOS build with QtWebEngine actually
    // linked (VIVACE_HAVE_DESKTOP_YOUTUBE_LOGIN) -- QML uses this to decide
    // whether to offer "Log in to YouTube…" at all, mirroring
    // AndroidYoutubeLogin::isSupported()/PythonYoutubeResolver::isSupported().
    Q_INVOKABLE static bool isSupported();

    // Same QStandardPaths::AppDataLocation-based shape as
    // AndroidYoutubeLogin::plannedCookiesPath() -- a DIFFERENT filename
    // (not shared) so the two features never collide if a build somehow
    // supported both at once (never true in practice: Android and desktop
    // are mutually exclusive targets), and so re-logging-in on one platform
    // never silently clobbers a file saved by the other.
    Q_INVOKABLE static QString plannedCookiesPath();

    // Clears the cookies accumulated so far (NOT the browser's own session
    // -- an already-signed-in Google account stays signed in, so reopening
    // the dialog within the same app run doesn't force the user to log in
    // again) and re-primes from whatever the cookie store already holds.
    // Called by qml/desktop/YoutubeLoginDialog.qml's own open(), before
    // navigating to the login page.
    Q_INVOKABLE void reset();

    // Writes every accumulated cookie whose domain is youtube.com or
    // google.com (see the .cpp's own comment on why both) to destPath as a
    // Netscape-format cookies.txt, using each cookie's REAL domain/path/
    // secure/expiry attributes (QWebEngineCookieStore exposes real
    // QNetworkCookie objects, unlike Android's CookieManager, which only
    // ever returns a flat name=value string -- see AndroidYoutubeLogin's own
    // comment on why ITS cookies.txt has to approximate those fields).
    // Emits cookiesSaved(count) on success or cookiesSaveFailed(message) on
    // failure (no cookies found, or destPath couldn't be written), mirroring
    // AndroidYoutubeLogin's exact same two signals so
    // qml/desktop/YoutubeLoginDialog.qml can share its own Connections
    // block's shape with the Android dialog's.
    Q_INVOKABLE void saveCookies(const QString &destPath);

signals:
    void cookiesSaved(int count);
    void cookiesSaveFailed(const QString &message);

private:
#ifdef VIVACE_HAVE_DESKTOP_YOUTUBE_LOGIN
    void onCookieAdded(const QNetworkCookie &cookie);
    void onCookieRemoved(const QNetworkCookie &cookie);
    static QString cookieKey(const QNetworkCookie &cookie);

    QMap<QString, QNetworkCookie> m_cookies;
#endif
};

#endif // DESKTOPYOUTUBELOGIN_H
