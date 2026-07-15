/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Open URL dialog, following SMPlayer's InputURL: a large URL icon on the
    left, a "URL:" label with an editable history combo box (clear button
    in the field), a separator line and OK/Cancel.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: dialog

    signal accepted(string url)

    title: qsTr("Enter URL")
    flags: Qt.Dialog
    modality: Qt.WindowModal
    width: 520
    height: 170
    minimumWidth: 420
    minimumHeight: 150
    maximumHeight: 200
    color: "#f0f0f0"

    function open() {
        urlField.clear()
        if (transientParent) {
            x = transientParent.x + (transientParent.width - width) / 2
            y = transientParent.y + (transientParent.height - height) / 2
        }
        visible = true
        urlField.forceActiveFocus()
    }

    function submit() {
        const url = urlField.text.trim()
        if (url !== "") {
            Settings.addUrlToHistory(url)
            dialog.accepted(url)
        }
        dialog.close()
    }

    Shortcut { sequence: "Escape"; onActivated: dialog.close() }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        // Icon + "URL:" label + editable history combo on one row, matching
        // SMPlayer's InputURL grid (icon | label + combo).
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            Image {
                source: Theme.icon("url_big")
                sourceSize: Qt.size(48, 48)
            }

            Label {
                text: UiHelpers.mnemonicLabel(qsTr("&URL:"))
                textFormat: Text.StyledText
            }

            HelpMark { text: qsTr("Address of a network stream or media file to play "
                                  + "(for example an http or hls link).") }

            ComboBox {
                id: urlEdit
                Layout.fillWidth: true
                editable: true
                model: Settings.urlHistory

                // The custom editor's text is the single source of truth (the
                // editText plumbing is unreliable with a custom contentItem);
                // popup selection is copied into it explicitly on activation.
                onActivated: urlField.text = urlEdit.textAt(currentIndex)

                contentItem: TextField {
                    id: urlField
                    placeholderText: qsTr("https://…")
                    verticalAlignment: Text.AlignVCenter
                    background: null
                    rightPadding: clearButton.width
                    onAccepted: dialog.submit()

                    ToolButton {
                        id: clearButton
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        visible: urlField.text !== ""
                        icon.source: Theme.icon("clear_left")
                        icon.color: "transparent"
                        icon.width: 16
                        icon.height: 16
                        onClicked: {
                            urlField.clear()
                            urlField.forceActiveFocus()
                        }
                    }
                }
            }
        }

        MenuSeparator { Layout.fillWidth: true }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("OK")
                enabled: urlField.text.trim() !== ""
                onClicked: dialog.submit()
            }
            Button {
                text: qsTr("Cancel")
                onClicked: dialog.close()
            }
        }
    }
}
