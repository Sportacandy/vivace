/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Embedded YouTube login (Android only, VIVACE_HAVE_ANDROID_YOUTUBE_LOGIN
    -- see CMakeLists.txt's own comment on why this is Android-only, not
    also a desktop QtWebEngine integration). Hosts a real, OS-native-backed
    QtWebView (qml/YoutubeLoginDialog.qml) the user signs into normally,
    then extracts the resulting session cookies via JNI (android.webkit.
    CookieManager -- see the .cpp's own comment for why QtWebView's own QML
    API can't do this) and writes them as a synthesized Netscape-format
    cookies.txt, wired into the SAME Settings.youtubeCookiesFile both
    YoutubeResolver and PythonYoutubeResolver already read -- so signing in
    here immediately unlocks HD/members-only/age-restricted YouTube
    downloads the exact same way a manually-exported cookies.txt would,
    with no separate desktop export/transfer round trip needed.

    A complete no-op (isSupported() returns false, saveCookies() always
    fails) on every non-Android build, or an Android build without QtWebView
    linked -- exactly the same pattern PythonYoutubeResolver::isSupported()
    already establishes for "this whole feature may not exist in this
    build," so QML can check it unconditionally.
*/

#ifndef ANDROIDYOUTUBELOGIN_H
#define ANDROIDYOUTUBELOGIN_H

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

class AndroidYoutubeLogin : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit AndroidYoutubeLogin(QObject *parent = nullptr);

    // True only on an Android build with QtWebView actually linked
    // (VIVACE_HAVE_ANDROID_YOUTUBE_LOGIN) -- QML uses this to decide
    // whether to offer "Log in to YouTube…" at all, the same way
    // PythonYoutubeResolver::isSupported() gates Streaming mode.
    Q_INVOKABLE static bool isSupported();

    // Where saveCookies() should write to -- same
    // QStandardPaths::AppDataLocation-based shape as PythonYoutubeResolver::
    // plannedInstallPath(), computed here (not in QML) for the same reason:
    // keeps the "where do we put this generated file" decision in one
    // place. QML sets Settings.youtubeCookiesFile to this same path once
    // cookiesSaved() fires, making the result immediately usable with no
    // separate Browse… step.
    Q_INVOKABLE static QString plannedCookiesPath();

    // Reads the live cookies Android's native WebView cookie jar currently
    // holds for youtube.com and google.com (after the user has signed in
    // via qml/YoutubeLoginDialog.qml's embedded WebView) and writes a
    // synthesized cookies.txt to destPath. Synchronous -- this is a pure
    // local read (no network I/O of its own) so there's no need for the
    // async/worker-thread pattern PythonYoutubeResolver's own resolve()/
    // download() use. Emits cookiesSaved(count) on success (count = how
    // many cookies were written -- 0 usually means the user opened the
    // dialog but never actually finished signing in) or
    // cookiesSaveFailed(message) if destPath couldn't be written at all.
    Q_INVOKABLE void saveCookies(const QString &destPath);

signals:
    void cookiesSaved(int count);
    void cookiesSaveFailed(const QString &message);
};

#endif // ANDROIDYOUTUBELOGIN_H
