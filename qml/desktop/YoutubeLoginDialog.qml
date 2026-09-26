/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Windows/Linux/macOS only (Preferences ▸ Network ▸ YouTube's "Log in to
    YouTube…" button, gated on DesktopYoutubeLogin.isSupported()): hosts a
    real, Chromium-backed WebEngineView the user signs into YouTube in
    normally, then extracts the resulting session cookies into a
    cookies.txt Vivace can reuse for Download & play -- a fallback for
    Windows Chrome/Edge users, where the existing "Get cookies from
    browser" feature can't read their cookies at all (App-Bound
    Encryption), and otherwise just a no-export-step convenience matching
    Android's own embedded login.

    Kept in its own qml/desktop/ subdirectory, registered under the SAME
    "YoutubeLoginDialog" QML type name as qml/YoutubeLoginDialog.qml
    (Android) and qml/stubs/YoutubeLoginDialog.qml -- CMakeLists.txt picks
    exactly one of the three per platform (basename-based QML type
    registration, not path-based). This split exists for the identical
    reason the Android/stub split does (see qml/stubs/YoutubeLoginDialog.qml's
    own comment): qmlimportscanner statically scans every .qml file's
    `import` statements regardless of which platform is actually being
    built for, and QtWebEngine doesn't even ship a QML plugin for Android at
    all (Windows/Linux/macOS only) -- so this file's `import QtWebEngine`
    line must never appear in an Android build's QML_FILES list.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWebEngine

Window {
    id: dialog

    property QtObject resolver: null // DesktopYoutubeLogin instance

    // Hides another window (normally the Preferences dialog this was opened
    // from) while this one is up, so the two don't visually overlap/compete
    // for attention -- restored once this dialog closes.
    property Window hideWhileOpen: null
    onVisibleChanged: {
        if (hideWhileOpen)
            hideWhileOpen.visible = !visible
    }

    title: qsTr("Log in to YouTube")
    flags: Qt.Dialog
    color: palette.window
    width: 640
    height: 760
    minimumWidth: 480
    minimumHeight: 560

    function open() {
        // A fresh reset() before every open (not just once at startup) so
        // an earlier login attempt's own accumulated cookies never leak
        // into what looks like a brand new attempt -- the underlying
        // browser SESSION itself is untouched (see DesktopYoutubeLogin::
        // reset()'s own comment), so an already-signed-in account doesn't
        // need signing into again.
        if (dialog.resolver)
            dialog.resolver.reset()
        webView.url = "https://www.youtube.com/"
        statusLabel.text = ""
        if (transientParent) {
            x = transientParent.x + (transientParent.width - width) / 2
            y = transientParent.y + (transientParent.height - height) / 2
        }
        visible = true
        raise()
        requestActivate()
    }

    Connections {
        target: dialog.resolver
        function onCookiesSaved(count) {
            statusLabel.color = "#2e7d32"
            statusLabel.text = qsTr("%n cookie(s) saved — tap “Close” to finish.", "", count)
        }
        function onCookiesSaveFailed(message) {
            statusLabel.color = "#c62828"
            statusLabel.text = message
        }
    }

    Shortcut {
        sequences: [StandardKey.Cancel]
        onActivated: dialog.close()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Sign in below, then tap “Save cookies”, then “Close”.")
            }
            Button {
                text: qsTr("Save cookies")
                onClicked: {
                    if (dialog.resolver)
                        dialog.resolver.saveCookies(dialog.resolver.plannedCookiesPath())
                }
            }
            Button {
                text: qsTr("Close")
                onClicked: dialog.close()
            }
        }

        Label {
            id: statusLabel
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            visible: text !== ""
        }

        WebEngineView {
            id: webView
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
