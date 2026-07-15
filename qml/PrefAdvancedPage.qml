/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Preferences > Advanced: configuration locations and maintenance.
    (SMPlayer's mplayer/mpv argument passing has no equivalent.)
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: page

    property PlayerController controller

    readonly property string helpText: qsTr(
        "<h1>Advanced</h1>"
        + "<p>Shows where Vivace keeps its configuration (main settings live in "
        + "the Windows registry; per-file data in vivace_files.ini) and lets you "
        + "open that folder.</p>"
        + "<p><b>Clear saved file settings</b> forgets all remembered playback "
        + "positions and per-file track choices — this is immediate and is not "
        + "undone by Cancel.</p>")

    spacing: 10

    GroupBox {
        Layout.fillWidth: true
        title: qsTr("Configuration")

        ColumnLayout {
            anchors.fill: parent
            spacing: 6

            RowLayout {
                spacing: 8
                Label { text: qsTr("Configuration folder:") }
                TextField {
                    Layout.fillWidth: true
                    readOnly: true
                    text: UiHelpers.toLocalPath(UiHelpers.configFolderUrl())
                }
                Button {
                    text: qsTr("Open…")
                    onClicked: Qt.openUrlExternally(UiHelpers.configFolderUrl())
                }
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                opacity: 0.7
                font.pixelSize: 12
                text: qsTr("Main settings are stored in the Windows registry (HKCU\\Software\\vivace-player); per-file data in vivace_files.ini in the folder above.")
            }
        }
    }

    GroupBox {
        Layout.fillWidth: true
        title: qsTr("Maintenance")

        ColumnLayout {
            anchors.fill: parent
            spacing: 6

            RowLayout {
                spacing: 8
                Button {
                    text: qsTr("Clear saved file settings")
                    onClicked: {
                        if (page.controller)
                            page.controller.clearFileSettings()
                    }
                }
                HelpMark { text: qsTr("Erases every remembered playback position and per-file track choice at once; this happens immediately and is not undone by Cancel.") }
                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                    font.pixelSize: 12
                    text: qsTr("Forgets all remembered playback positions and track selections.")
                }
            }
        }
    }

    GroupBox {
        Layout.fillWidth: true
        title: qsTr("Logs")

        ColumnLayout {
            anchors.fill: parent
            spacing: 6

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Vivace writes warnings and errors to vivace.log in the "
                           + "configuration folder above (kept across sessions, with "
                           + "the previous session as vivace.log.1). Include it when "
                           + "reporting a problem.")
            }
            Button {
                text: qsTr("Open log file")
                enabled: UiHelpers.logFileUrl() != ""
                onClicked: Qt.openUrlExternally(UiHelpers.logFileUrl())
            }
        }
    }

    Item { Layout.fillHeight: true }
}
