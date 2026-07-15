/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Information and properties, following SMPlayer's FilePropertiesDialog:
    tabs Information / Demuxer / Video codec / Audio codec. The Information
    tab renders PlayerController.mediaInfoHtml (InfoFile format). The other
    tabs list the demuxers/decoders supported by the FFmpeg backend, with
    the ones used by the current file highlighted. (Unlike SMPlayer they
    are informational: Qt Multimedia cannot force a demuxer or codec.
    SMPlayer's "Options for MPlayer" tab has no equivalent and is omitted.)
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: infoDialog

    required property PlayerController controller

    title: qsTr("Information and properties")
    flags: Qt.Dialog
    width: 640
    height: 600
    minimumWidth: 480
    minimumHeight: 360
    color: "#f0f0f0"

    function open() {
        if (transientParent) {
            x = transientParent.x + (transientParent.width - width) / 2
            y = transientParent.y + (transientParent.height - height) / 2
        }
        visible = true
    }

    Shortcut {
        sequence: "Escape"
        onActivated: infoDialog.close()
    }

    component FormatTable: ColumnLayout {
        id: table

        property var rows: []
        property int currentValue: -1

        spacing: 0

        Label {
            Layout.fillWidth: true
            Layout.bottomMargin: 4
            wrapMode: Text.WordWrap
            opacity: 0.7
            font.pixelSize: 12
            text: qsTr("Supported by the playback backend; the entry used by the current file is highlighted. A specific one cannot be forced.")
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Label {
                Layout.preferredWidth: 120
                text: qsTr("Name")
                font.bold: true
            }
            Label {
                Layout.fillWidth: true
                text: qsTr("Description")
                font.bold: true
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: table.rows

            ScrollBar.vertical: ScrollBar {}

            delegate: Rectangle {
                required property int index
                required property var modelData

                readonly property bool isCurrent:
                    modelData.value === table.currentValue

                width: ListView.view.width
                height: rowLayout.implicitHeight + 4
                color: isCurrent ? "#cce8ff"
                                 : index % 2 ? "transparent" : "#00000010"

                RowLayout {
                    id: rowLayout
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    spacing: 8

                    Label {
                        Layout.preferredWidth: 120
                        text: modelData.name
                        font.bold: isCurrent
                    }
                    Label {
                        Layout.fillWidth: true
                        text: modelData.description
                        elide: Text.ElideRight
                        font.bold: isCurrent
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

    TabBar {
        id: tabs
        Layout.fillWidth: true
        TabButton { text: qsTr("&Information") }
        TabButton { text: qsTr("&Demuxer") }
        TabButton { text: qsTr("&Video codec") }
        TabButton { text: qsTr("&Audio codec") }
    }

    StackLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tabs.currentIndex

        // ------------------------------------------------- Information
        ScrollView {
            clip: true
            contentWidth: availableWidth

            TextArea {
                readOnly: true
                textFormat: TextEdit.RichText
                wrapMode: TextEdit.Wrap
                color: "black"
                text: infoDialog.controller.mediaInfoHtml !== ""
                      ? infoDialog.controller.mediaInfoHtml
                      : "<i>" + qsTr("No media loaded") + "</i>"
                background: Rectangle { color: "white" }
            }
        }

        // ----------------------------------------------------- Demuxer
        FormatTable {
            rows: UiHelpers.supportedFileFormats()
            currentValue: infoDialog.controller.currentFileFormat
        }

        // ------------------------------------------------- Video codec
        FormatTable {
            rows: UiHelpers.supportedVideoCodecs()
            currentValue: infoDialog.controller.currentVideoCodec
        }

        // ------------------------------------------------- Audio codec
        FormatTable {
            rows: UiHelpers.supportedAudioCodecs()
            currentValue: infoDialog.controller.currentAudioCodec
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Item { Layout.fillWidth: true }
        Button {
            text: qsTr("Close")
            onClicked: infoDialog.close()
        }
    }

    } // ColumnLayout
}
