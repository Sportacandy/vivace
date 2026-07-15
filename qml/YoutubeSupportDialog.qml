/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Help > Install / Update YouTube support (SMPlayer's CodeDownloader): offers
    to download the latest official yt-dlp binary and install it, then enables
    the YouTube resolver. Confirm -> progress -> result, all in one dialog.
    A normal top-level modal dialog (OS window frame), matching the other
    Vivace dialogs.
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
    width: 480
    height: contentCol.implicitHeight + 24
    minimumWidth: 380
    minimumHeight: contentCol.implicitHeight + 24

    // "confirm" | "downloading" | "done" | "error"
    property string phase: "confirm"
    property string resultText: ""

    function openDialog() {
        dlg.phase = "confirm"
        dlg.resultText = ""
        if (transientParent) {
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
            Settings.youtubeEnabled = true
            dlg.phase = "done"
            dlg.resultText =
                qsTr("yt-dlp was installed successfully as:") + "\n" + path
                + "\n\n" + qsTr("YouTube playback is now enabled.")
        }
        function onInstallFailed(message) {
            dlg.phase = "error"
            dlg.resultText = qsTr("The download failed:") + "\n" + message
        }
    }

    ColumnLayout {
        id: contentCol
        anchors.fill: parent
        anchors.margins: 12
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
