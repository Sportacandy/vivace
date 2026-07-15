/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Update checker glue: hosts the UpdateChecker, runs the automatic startup
    check (gated by the interval in Preferences), and shows the result in a
    normal top-level modal dialog. Mirrors SMPlayer's UpdateChecker reporting —
    automatic checks are silent unless a newer version is found; a user-requested
    check also reports "up to date" and errors. The check URL is a placeholder
    for now (see updatechecker.cpp).
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: manager

    // Start a check requested by the user (reports every outcome).
    function checkNow() { checker.check(true) }

    UpdateChecker {
        id: checker

        onNewVersionFound: function(version) {
            // Automatic checks don't nag about a version already seen.
            const seen = version === Settings.updateLastKnownVersion
            Settings.updateLastKnownVersion = version
            if (!manager._auto || !seen)
                manager._showNewVersion(version)
        }
        onUpToDate: function(version) { manager._showUpToDate(version) }
        onErrorOccurred: function(message) { manager._showError(message) }
        onCheckFinished: function(success) {
            if (success)
                Settings.updateLastCheck = manager._todayIso()
            manager._auto = false
        }
    }

    // True while the in-flight check was started automatically (startup).
    property bool _auto: false

    function _todayIso() {
        return Qt.formatDate(new Date(), "yyyy-MM-dd")
    }

    function _open(mode, title, text) {
        msgWindow.mode = mode
        msgWindow.title = title
        msgWindow.text = text
        msgWindow.showCentered()
    }

    function _showNewVersion(version) {
        _open("newversion", qsTr("New version available"),
              qsTr("A new version of Vivace is available.") + "\n\n"
              + qsTr("Installed version: %1").arg(checker.currentVersion) + "\n"
              + qsTr("Available version: %1").arg(version) + "\n\n"
              + qsTr("Would you like to know more about this new version?"))
    }

    function _showUpToDate(version) {
        _open("info", qsTr("Checking for updates"),
              qsTr("Congratulations, Vivace is up to date.") + "\n\n"
              + qsTr("Installed version: %1").arg(checker.currentVersion) + "\n"
              + qsTr("Available version: %1").arg(version))
    }

    function _showError(message) {
        _open("info", qsTr("Error"),
              qsTr("An error happened while checking for the latest version.")
              + "\n\n" + message)
    }

    Window {
        id: msgWindow

        // "newversion" (Yes/No) | "info" (Close)
        property string mode: "info"
        property string text: ""

        flags: Qt.Dialog
        modality: Qt.WindowModal
        color: palette.window
        width: 460
        height: msgCol.implicitHeight + 24
        minimumWidth: 360
        minimumHeight: msgCol.implicitHeight + 24

        function showCentered() {
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
            onActivated: msgWindow.close()
        }

        ColumnLayout {
            id: msgCol
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                textFormat: Text.PlainText
                text: msgWindow.text
            }

            MenuSeparator { Layout.fillWidth: true }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("Yes")
                    visible: msgWindow.mode === "newversion"
                    onClicked: {
                        Qt.openUrlExternally(checker.downloadUrl)
                        msgWindow.close()
                    }
                }
                Button {
                    text: qsTr("No")
                    visible: msgWindow.mode === "newversion"
                    onClicked: msgWindow.close()
                }
                Button {
                    text: qsTr("Close")
                    visible: msgWindow.mode === "info"
                    onClicked: msgWindow.close()
                }
            }
        }
    }

    // Automatic check on startup, only if enabled and the interval has elapsed.
    Component.onCompleted: {
        if (Settings.updateCheckEnabled
                && checker.isCheckDue(Settings.updateLastCheck,
                                      Settings.updateCheckIntervalDays)) {
            manager._auto = true
            checker.check(false)
        }
    }
}
