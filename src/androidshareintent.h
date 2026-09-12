/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Android's "Share" mechanism (ACTION_SEND): lets another app (a browser,
    etc.) hand Vivace a URL directly via the system Share sheet -- the
    practical substitute for cross-app drag-and-drop, which Qt's Android
    platform plugin doesn't support receiving until 6.11.2 (this project
    pins 6.11.1 -- confirmed directly against the real qtbase source: the
    C++ platform-side file and its Java-side companion class both only
    exist starting at the v6.11.2 tag, neither at v6.11.1).

    AndroidManifest.xml declares an ACTION_SEND/text-plain intent-filter on
    the main activity; this class extracts the shared URL from BOTH ends
    of Android's own intent-delivery split: the activity's own initial
    intent (a cold launch straight from the Share sheet -- coldStartUrl(),
    called once at startup) and any LATER intent delivered while already
    running (QtAndroidPrivate::NewIntentListener -- Android's singleTop
    launch mode, already set in the manifest, means a second "launch" is
    always this, never a real second process the way desktop single-
    instance forwarding works). A complete no-op on every other platform.
*/

#ifndef ANDROIDSHAREINTENT_H
#define ANDROIDSHAREINTENT_H

#include <QObject>
#include <QString>

#ifdef Q_OS_ANDROID
#include <QtCore/private/qjnihelpers_p.h>
#endif

class AndroidShareIntent : public QObject
#ifdef Q_OS_ANDROID
        , public QtAndroidPrivate::NewIntentListener
#endif
{
    Q_OBJECT

public:
    explicit AndroidShareIntent(QObject *parent = nullptr);
    ~AndroidShareIntent() override;

    // The shared URL from the activity's own launch intent, if this
    // process was cold-started directly from the Share sheet. Empty on
    // every other platform, and empty if the app was launched normally.
    static QString coldStartUrl();

signals:
    // Fired when a URL is shared while the app is already running.
    void urlShared(const QString &url);

#ifdef Q_OS_ANDROID
private:
    bool handleNewIntent(JNIEnv *env, jobject intent) override;
#endif
};

#endif // ANDROIDSHAREINTENT_H
