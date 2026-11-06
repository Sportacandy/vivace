/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Android's "Share" mechanism (ACTION_SEND): lets another app (a browser,
    etc.) hand Vivace a URL directly via the system Share sheet. It was
    added as the substitute for cross-app drag-and-drop, which Qt's Android
    platform plugin can only receive from 6.11.2 on (confirmed against the
    qtbase source: the platform-side file and its Java companion class first
    exist at the v6.11.2 tag). Vivace now builds with 6.12; whether its
    DropArea receives drops from other Android apps hasn't been tested, and
    Share is still the natural way for a browser to hand over a page URL.

    It also handles "Open with" from a file manager (ACTION_VIEW with a
    content:// media or playlist URI) and a file manager's "Share" of a
    media or playlist file (ACTION_SEND with the URI in EXTRA_STREAM), which
    arrive the same two ways.

    AndroidManifest.xml declares an ACTION_SEND/text-plain intent-filter
    (and an ACTION_VIEW one) on the main activity; this class extracts the
    URL to open from BOTH ends
    of Android's own intent-delivery split: the activity's own initial
    intent (a cold launch straight from the Share sheet -- coldStartUrl(),
    called once at startup) and any LATER intent delivered while already
    running (QtAndroidPrivate::NewIntentListener -- Android's singleTask
    launch mode, set in the manifest, means a second "launch" is always
    this, never a real second process the way desktop single-instance
    forwarding works). A complete no-op on every other platform.
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
