/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Offers to download the latest official yt-dlp binary and install it
    (SMPlayer's CodeDownloader). Confirm -> progress -> result, all in one
    dialog. Opened on demand by the "Install / Update yt-dlp…" button in
    Preferences > Network > YouTube, enabled only while "Use managed
    yt-dlp" is on (a user-supplied yt-dlp is never installed/updated by
    Vivace, on demand or automatically). Ongoing updates between manual
    clicks are handled separately and automatically by YoutubeResolver's
    own auto-update cadence. A normal top-level modal dialog (OS window
    frame), matching the other Vivace dialogs.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: dlg

    // The YoutubeResolver instance that performs the download.
    required property YoutubeResolver resolver

    title: qsTr("Install / Update YouTube support")
    flags: Qt.Dialog
    modality: Qt.WindowModal
    color: palette.window
    // Android: fill the transientParent's own bounds EXACTLY instead of a
    // fixed/content-driven desktop size (see PreferencesDialog.qml's own
    // comment for why -- no guessed-constant margin, real SafeArea inset
    // used on the content layout below instead).
    width: Qt.platform.os === "android" && transientParent
           ? transientParent.width : 480
    height: Qt.platform.os === "android" && transientParent
            ? transientParent.height : contentCol.implicitHeight + 24
    minimumWidth: Qt.platform.os === "android" ? 0 : 380
    minimumHeight: Qt.platform.os === "android" ? 0 : contentCol.implicitHeight + 24

    // "confirm" | "downloading" | "done" | "error"
    property string phase: "confirm"
    property string resultText: ""

    function openDialog() {
        dlg.phase = "confirm"
        dlg.resultText = ""
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

    Shortcut {
        sequences: [StandardKey.Cancel]
        onActivated: dlg.close()
    }

    Connections {
        target: dlg.resolver
        function onInstallProgress(received, total) {
            progressBar.indeterminate = total <= 0
            if (total > 0) { progressBar.to = total; progressBar.value = received }
        }
        function onInstallFinished(path) {
            Settings.ytdlPath = path
            dlg.phase = "done"
            dlg.resultText =
                qsTr("yt-dlp was installed successfully as:") + "\n" + path
        }
        function onInstallFailed(message) {
            dlg.phase = "error"
            dlg.resultText = qsTr("The download failed:") + "\n" + message
        }
    }

    ColumnLayout {
        id: contentCol
        anchors.fill: parent
        // Real platform-reported inset, not a guessed constant -- see
        // PreferencesDialog.qml's own outer ColumnLayout comment for why.
        anchors.topMargin: 12 + SafeArea.margins.top
        anchors.leftMargin: 12 + SafeArea.margins.left
        anchors.rightMargin: 12 + SafeArea.margins.right
        anchors.bottomMargin: 12 + SafeArea.margins.bottom
        spacing: 12

        // Confirmation text (shown before the download starts).
        Label {
            Layout.fillWidth: true
            visible: dlg.phase === "confirm"
            wrapMode: Text.WordWrap
            textFormat: Text.PlainText
            text: qsTr("To play YouTube videos, Vivace uses an external program "
                       + "called yt-dlp.")
                  + "\n\n"
                  + qsTr("Vivace can download the latest version from the "
                         + "official website and install it as:")
                  + "\n" + (dlg.resolver ? dlg.resolver.plannedInstallPath() : "")
                  + "\n\n" + qsTr("Would you like to proceed?")
        }

        // Download progress.
        ColumnLayout {
            Layout.fillWidth: true
            visible: dlg.phase === "downloading"
            spacing: 6
            Label { text: qsTr("Downloading yt-dlp…") }
            ProgressBar {
                id: progressBar
                Layout.fillWidth: true
                from: 0; to: 1; value: 0
                indeterminate: true
            }
        }

        // Result (success or error).
        Label {
            Layout.fillWidth: true
            visible: dlg.phase === "done" || dlg.phase === "error"
            wrapMode: Text.WordWrap
            textFormat: Text.PlainText
            text: dlg.resultText
        }

        MenuSeparator { Layout.fillWidth: true }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("Install")
                visible: dlg.phase === "confirm"
                onClicked: {
                    dlg.phase = "downloading"
                    progressBar.indeterminate = true
                    dlg.resolver.installOrUpdate()
                }
            }
            Button {
                text: qsTr("Cancel")
                visible: dlg.phase === "confirm"
                onClicked: dlg.close()
            }
            Button {
                text: qsTr("Close")
                visible: dlg.phase === "done" || dlg.phase === "error"
                onClicked: dlg.close()
            }
        }
    }
}
