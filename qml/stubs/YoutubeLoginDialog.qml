/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Desktop/non-Android stand-in for qml/YoutubeLoginDialog.qml, registered
    under the SAME "YoutubeLoginDialog" QML type name (CMakeLists.txt picks
    exactly one of the two file PATHS to add to QML_FILES, based on
    if(ANDROID), so only one is ever actually compiled into a given build)
    -- Main.qml/PreferencesDialog.qml/PrefNetworkPage.qml reference the type
    unconditionally (matching PythonYoutubeResolver's own established "the
    type always exists, isSupported() gates whether it DOES anything"
    pattern), so a real type registration has to exist on every platform.

    This file deliberately has NO `import QtWebView` at all -- a real,
    surprising bug found and fixed the same day this feature was built: even
    though the C++ WebView linkage is correctly Android-only (CMakeLists.txt's
    VIVACE_HAVE_ANDROID_YOUTUBE_LOGIN gate), qmlimportscanner statically
    scans EVERY .qml file's import statements regardless of which platform
    is being built for -- so the real qml/YoutubeLoginDialog.qml's own
    `import QtWebView` line was enough, by itself, to make a desktop build
    discover and load the real desktop QtWebView QML plugin (Qt ships one
    there too, entirely unrelated to this Android-only feature), which
    produced a genuine "QMetaObject::indexOfSignal: signal xChanged(int)
    from QQuickWindow redefined in QQuickWindowQmlImpl" warning on ordinary
    desktop startup -- confirmed by A/B testing an unmodified, pre-existing
    Vivace build (no WebView code anywhere) against a freshly rebuilt one
    with only this file added, seeing the warning appear only in the latter.
    Keeping the real WebView-importing file out of desktop's own QML_FILES
    list entirely (via this stub) is the fix -- not something achievable
    with a runtime Loader/conditional import, since QML has no conditional
    `import` statement and qmlimportscanner works by static file scanning,
    not by tracing what a Loader might instantiate at runtime.
*/

import QtQuick

Window {
    id: dialog

    property QtObject resolver: null

    visible: false

    function open() {
        // Never actually reachable: the one call site (PrefNetworkPage.qml's
        // "Log in to YouTube…" button) is itself hidden with
        // `visible: Qt.platform.os === "android" && ...`, matching every
        // other Android-only control in that file.
    }
}
