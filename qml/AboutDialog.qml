/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    About dialog, following SMPlayer's about.ui: a header (logo + name) and
    Info / Contributions / Translations / License tabs. Info and License
    carry real content; Contributions credits SMPlayer (whose icon theme and
    UI conventions Vivace ports under the GPL); Translations fills in when
    localization arrives (Phase 6). A normal top-level modal dialog (OS window
    frame), like the other Vivace dialogs.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: dialog

    title: qsTr("About Vivace")
    flags: Qt.Dialog
    modality: Qt.WindowModal
    color: palette.window
    width: 560
    height: 480
    minimumWidth: 460
    minimumHeight: 380

    function open() {
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
        onActivated: dialog.close()
    }

    // Qt versions for the Info tab (qVersion()/QT_VERSION_STR via C++).
    readonly property string runtimeQtVersion: UiHelpers.qtRuntimeVersion()
    readonly property string compileQtVersion: UiHelpers.qtBuildVersion()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        // Header: app icon + name + tagline.
        RowLayout {
            Layout.fillWidth: true
            spacing: 14

            Image {
                source: "qrc:/qt/qml/Vivace/icons/app_64.png"
                sourceSize: Qt.size(64, 64)
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Label {
                    text: "Vivace"
                    font.pixelSize: 26
                    font.bold: true
                }
                Label {
                    text: qsTr("A fast, pure-Qt media player — vee-VAH-cheh")
                    opacity: 0.8
                }
            }
        }

        TabBar {
            id: tabs
            Layout.fillWidth: true
            TabButton {
                text: qsTr("Info")
                icon.source: Theme.icon("info")
                icon.color: "transparent"
            }
            TabButton {
                text: qsTr("Contributions")
                icon.source: Theme.icon("contributors")
                icon.color: "transparent"
            }
            TabButton {
                text: qsTr("Translations")
                icon.source: Theme.icon("translators")
                icon.color: "transparent"
            }
            TabButton {
                text: qsTr("License")
                icon.source: Theme.icon("license")
                icon.color: "transparent"
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabs.currentIndex

            // ------------------------------------------------- Info
            ScrollView {
                clip: true
                contentWidth: availableWidth
                TextArea {
                    readOnly: true
                    wrapMode: Text.WordWrap
                    textFormat: Text.RichText
                    background: null
                    onLinkActivated: link => Qt.openUrlExternally(link)
                    text: "<b>" + qsTr("Version: %1").arg(Qt.application.version)
                          + "</b><br><br>"
                          + qsTr("A ground-up rewrite inspired by SMPlayer, built on "
                                 + "Qt Quick and Qt Multimedia (FFmpeg backend) — "
                                 + "no mplayer/mpv process backend and no Qt Widgets.")
                          + "<br><br>"
                          + qsTr("Using Qt %1 (compiled with Qt %2)")
                                .arg(dialog.runtimeQtVersion).arg(dialog.compileQtVersion)
                          + "<br><br>"
                          + "<b>" + qsTr("Links:") + "</b><br>"
                          + qsTr("Project home:") + " "
                          + "<a href=\"https://github.com/vivace-player\">"
                          + "github.com/vivace-player</a>"
                }
            }

            // ---------------------------------------- Contributions
            ScrollView {
                clip: true
                contentWidth: availableWidth
                TextArea {
                    readOnly: true
                    wrapMode: Text.WordWrap
                    textFormat: Text.RichText
                    background: null
                    onLinkActivated: link => Qt.openUrlExternally(link)
                    text: qsTr("Vivace is developed by %1.").arg("Hironori Komaba")
                          + "<br><br>"
                          + qsTr("Vivace ports UI conventions and the icon themes "
                                 + "(H2O and the default theme) from %1, used under "
                                 + "the GPL. Many thanks to its author Ricardo "
                                 + "Villalba and contributors.")
                                .arg("<a href=\"https://www.smplayer.info\">SMPlayer</a>")
                          + "<br><br>"
                          + qsTr("Secure password storage uses %1 by Frank "
                                 + "Osterfeld and contributors, used under the "
                                 + "modified BSD license.")
                                .arg("<a href=\"https://github.com/frankosterfeld/qtkeychain\">QtKeychain</a>")
                }
            }

            // ----------------------------------------- Translations
            Item {
                Label {
                    anchors.centerIn: parent
                    width: parent.width - 24
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    opacity: 0.8
                    text: qsTr("Vivace is not translated yet. Localization is "
                               + "planned for a later phase; contributions will be "
                               + "credited here.")
                }
            }

            // --------------------------------------------- License
            ScrollView {
                clip: true
                contentWidth: availableWidth
                TextArea {
                    readOnly: true
                    wrapMode: Text.WordWrap
                    textFormat: Text.RichText
                    background: null
                    onLinkActivated: link => Qt.openUrlExternally(link)
                    text: "<i>" + qsTr("This program is free software; you can "
                          + "redistribute it and/or modify it under the terms of the "
                          + "GNU General Public License as published by the Free "
                          + "Software Foundation; either version 3 of the License, or "
                          + "(at your option) any later version.") + "</i><br><br>"
                          + "<a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">"
                          + qsTr("Read the entire license") + "</a>"
                }
            }
        }

        MenuSeparator { Layout.fillWidth: true }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("Close")
                onClicked: dialog.close()
            }
        }
    }
}
