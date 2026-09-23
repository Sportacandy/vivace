/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Android only (Preferences ▸ Network ▸ YouTube's "Log in to YouTube…"
    button, gated on AndroidYoutubeLogin.isSupported()): hosts a real,
    OS-native-backed WebView the user signs into YouTube in normally, then
    extracts the resulting session cookies into a cookies.txt Vivace can
    reuse for Download & play -- Android's answer to desktop's "Get cookies
    from browser," since there's no live desktop-browser cookie store to
    read on Android at all (per-app sandboxing).

    Per QtWebView's own docs, the WebView must never be overlapped by any
    other QML item (a real platform limitation, not a style choice) -- the
    button row below sits in its OWN, non-overlapping region above it in a
    plain ColumnLayout, never stacked on top.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWebView

Window {
    id: dialog

    property QtObject resolver: null // AndroidYoutubeLogin instance

    // EXPERIMENTAL 2026-09-23 (diagnosing a real device report: on Android,
    // once this dialog opens over PreferencesDialog, NEITHER taps anywhere
    // in this window (including this file's own plain QML buttons, not just
    // the WebView) NOR the hardware Back key have any effect -- confirmed
    // via adb: a tap on "I'm signed in" and a BACK keyevent both produced a
    // byte-identical uiautomator dump before/after. This is Preferences ->
    // this dialog, a THIRD stacked top-level Window on Android (this
    // dialog's own transientParent defaults to the main window, since it's
    // declared as a Main.qml sibling of PreferencesDialog, not nested
    // inside it). Hypothesis: Qt for Android's multi-window input routing
    // doesn't reliably hand touch/key focus to a window three levels deep,
    // especially one hosting a WebView's own native Android View. Hiding
    // the intermediate window while this one is open reduces the stack to
    // two levels, to test that hypothesis -- if it fixes input, this stays;
    // otherwise it's a no-op to revert.
    property Window hideWhileOpen: null
    onVisibleChanged: {
        if (hideWhileOpen) {
            if (visible)
                hideWhileOpen.visible = false
            else
                hideWhileOpen.visible = true
        }
    }

    title: qsTr("Log in to YouTube")
    flags: Qt.Dialog
    color: palette.window
    // Same Android-fills-transientParent / real-SafeArea-inset pattern as
    // every other dialog in this project (see PreferencesDialog.qml's own
    // comment for the full "why" -- a guessed constant margin was tried and
    // rejected in favor of this).
    width: Qt.platform.os === "android" && transientParent
           ? transientParent.width : 480
    height: Qt.platform.os === "android" && transientParent
            ? transientParent.height : 640
    minimumWidth: Qt.platform.os === "android" ? 0 : 360
    minimumHeight: Qt.platform.os === "android" ? 0 : 480

    function open() {
        webView.url = "https://www.youtube.com/"
        statusLabel.text = ""
        if (Qt.platform.os === "android") {
            x = 0
            y = 0
        } else if (transientParent) {
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
        anchors.topMargin: 8 + SafeArea.margins.top
        anchors.leftMargin: 8 + SafeArea.margins.left
        anchors.rightMargin: 8 + SafeArea.margins.right
        anchors.bottomMargin: 8 + SafeArea.margins.bottom
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

        // Deliberately the ONLY item below this point in the ColumnLayout --
        // see this file's own header comment on why nothing may overlap it.
        WebView {
            id: webView
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
