/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Play > Cast > Smartphone/tablet…: start/stop the embedded CastServer and
    show the LAN address(es) to open on another device's browser.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: dialog

    property QtObject server: null

    title: qsTr("Cast to smartphone/tablet")
    flags: Qt.Dialog
    color: palette.window
    width: 560
    height: col.implicitHeight + 24
    minimumWidth: 480
    minimumHeight: 240

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

    ColumnLayout {
        id: col
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTr("Starts a small web server so a phone or tablet on the same "
                       + "network can open a page and play the video Vivace is "
                       + "currently playing.")
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6
            Label { text: qsTr("Port: %1").arg(Settings.castPort) }
            HelpMark { text: qsTr("Set in Preferences ▸ Network ▸ Cast — kept "
                                  + "fixed there so you can allow it through your "
                                  + "firewall/router once.") }
            Item { Layout.fillWidth: true }
            Button {
                text: dialog.server && dialog.server.running ? qsTr("Stop")
                                                              : qsTr("Start")
                onClicked: {
                    if (!dialog.server)
                        return
                    if (dialog.server.running)
                        dialog.server.stop()
                    else
                        dialog.server.start()
                }
            }
        }

        Label {
            visible: dialog.server && dialog.server.lastError !== ""
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: "#b00000"
            text: dialog.server
                  ? qsTr("Could not start the server: %1").arg(dialog.server.lastError)
                  : ""
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            visible: dialog.server && dialog.server.running

            // QR code for the first address, like SMPlayer's Cast >
            // Smartphone/tablet dialog (same underlying qrcodegen library),
            // on a white background since the code needs light quiet zones
            // around it regardless of the app's palette.
            Rectangle {
                Layout.preferredWidth: 180
                Layout.preferredHeight: 180
                visible: dialog.server && dialog.server.urls.length > 0
                color: "white"
                border.color: "#cccccc"
                Image {
                    anchors.fill: parent
                    anchors.margins: 4
                    fillMode: Image.PreserveAspectFit
                    smooth: false
                    source: dialog.server && dialog.server.urls.length > 0
                            ? dialog.server.qrCodeDataUrl(dialog.server.urls[0]) : ""
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                spacing: 4

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: qsTr("Scan the code, or open one of these addresses in "
                               + "the phone or tablet's web browser:")
                }
                Repeater {
                    model: dialog.server ? dialog.server.urls : []
                    delegate: TextField {
                        Layout.fillWidth: true
                        readOnly: true
                        selectByMouse: true
                        text: modelData
                    }
                }
                Label {
                    visible: dialog.server && dialog.server.urls.length === 0
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                    text: qsTr("No network address was found — check that this "
                               + "computer is connected to a network.")
                }
            }
        }

        Item { Layout.fillHeight: true }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            opacity: 0.7
            font.pixelSize: 12
            text: qsTr("Anyone on your network can open this address while casting "
                       + "is on. Only the file currently playing is served, and only "
                       + "while this is on — turn it off when you're done.")
        }

        MenuSeparator { Layout.fillWidth: true }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button { text: qsTr("Close"); onClicked: dialog.close() }
        }
    }
}
