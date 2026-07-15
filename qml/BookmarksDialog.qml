/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Bookmark editor, following SMPlayer's BookmarkDialog: a Time/Name table
    with inline editing, Add / Delete / Delete all buttons, committing the
    whole set on OK (Cancel discards). The current file's bookmarks come from
    PlayerController.bookmarks; editing works on a copy so Cancel is safe.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: dialog

    required property PlayerController controller

    // Working copy: an array of { time (ms), name }. Reassigned wholesale on
    // add/delete so the ListView refreshes; inline edits mutate in place.
    property var work: []

    title: qsTr("Bookmarks")
    flags: Qt.Dialog
    modality: Qt.WindowModal
    width: 460
    height: 420
    minimumWidth: 360
    minimumHeight: 280
    color: "#f0f0f0"

    function open() {
        const src = controller.bookmarks.entries()
        const copy = []
        for (let i = 0; i < src.length; ++i)
            copy.push({ time: src[i].time, name: src[i].name })
        work = copy
        listView.currentIndex = -1
        if (transientParent) {
            x = transientParent.x + (transientParent.width - width) / 2
            y = transientParent.y + (transientParent.height - height) / 2
        }
        visible = true
        raise()
        requestActivate()
    }
    function close() { visible = false }

    function pad(n) { return (n < 10 ? "0" : "") + n }
    function formatTime(ms) {
        const s = Math.max(0, Math.floor(ms / 1000))
        return pad(Math.floor(s / 3600)) + ":"
             + pad(Math.floor((s % 3600) / 60)) + ":" + pad(s % 60)
    }
    // Parses "hh:mm:ss", "mm:ss" or "ss"; returns ms, or -1 when malformed.
    function parseTime(text) {
        const parts = text.trim().split(":")
        let h = 0, m = 0, s = 0
        if (parts.length === 3) { h = +parts[0]; m = +parts[1]; s = +parts[2] }
        else if (parts.length === 2) { m = +parts[0]; s = +parts[1] }
        else if (parts.length === 1) { s = +parts[0] }
        else return -1
        if (isNaN(h) || isNaN(m) || isNaN(s)) return -1
        return (h * 3600 + m * 60 + s) * 1000
    }

    Shortcut { sequence: "Escape"; onActivated: dialog.close() }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            Image {
                source: Theme.icon("bookmarks")
                sourceSize: Qt.size(48, 48)
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Label {
                    text: qsTr("Bookmarks")
                    font.pixelSize: 20
                    font.bold: true
                }
                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    opacity: 0.8
                    text: qsTr("You can edit the time and name of each bookmark, "
                               + "or add and delete bookmarks.")
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            border.color: "#a0a0a0"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 1
                spacing: 0

                Row {
                    Layout.fillWidth: true
                    height: 26
                    Rectangle {
                        width: 110; height: parent.height
                        color: "#e0e0e0"; border.color: "#c0c0c0"
                        Label {
                            anchors.left: parent.left
                            anchors.leftMargin: 6
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Time")
                        }
                        HelpMark {
                            anchors.right: parent.right
                            anchors.rightMargin: 6
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Accepts hh:mm:ss, mm:ss or plain seconds.")
                        }
                    }
                    Rectangle {
                        width: parent.width - 110; height: parent.height
                        color: "#e0e0e0"; border.color: "#c0c0c0"
                        Label {
                            anchors.left: parent.left
                            anchors.leftMargin: 6
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Name")
                        }
                    }
                }

                ListView {
                    id: listView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    currentIndex: -1
                    model: dialog.work

                    ScrollBar.vertical: ScrollBar {}

                    delegate: Rectangle {
                        id: row
                        required property int index
                        required property var modelData
                        width: ListView.view.width
                        height: 30
                        color: ListView.isCurrentItem ? "#cce8ff"
                               : index % 2 ? "#ffffff" : "#f6f6f6"

                        MouseArea {
                            anchors.fill: parent
                            onClicked: listView.currentIndex = row.index
                        }

                        Row {
                            anchors.fill: parent
                            TextField {
                                width: 110
                                height: parent.height
                                text: dialog.formatTime(row.modelData.time)
                                background: null
                                onActiveFocusChanged: if (activeFocus)
                                                          listView.currentIndex = row.index
                                onEditingFinished: {
                                    const ms = dialog.parseTime(text)
                                    if (ms >= 0)
                                        dialog.work[row.index].time = ms
                                    text = dialog.formatTime(dialog.work[row.index].time)
                                }
                            }
                            TextField {
                                width: parent.width - 110
                                height: parent.height
                                text: row.modelData.name
                                placeholderText: qsTr("(unnamed)")
                                background: null
                                onActiveFocusChanged: if (activeFocus)
                                                          listView.currentIndex = row.index
                                onEditingFinished: dialog.work[row.index].name = text
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Button {
                text: qsTr("&Add")
                icon.source: Theme.icon("add_bookmark")
                icon.color: "transparent"
                onClicked: {
                    const copy = dialog.work.slice()
                    const at = listView.currentIndex >= 0
                             ? listView.currentIndex + 1 : copy.length
                    copy.splice(at, 0, { time: 0, name: "" })
                    dialog.work = copy
                    listView.currentIndex = at
                }
            }
            Button {
                text: qsTr("D&elete")
                icon.source: Theme.icon("delete")
                icon.color: "transparent"
                enabled: listView.currentIndex >= 0
                onClicked: {
                    const copy = dialog.work.slice()
                    copy.splice(listView.currentIndex, 1)
                    dialog.work = copy
                    listView.currentIndex = -1
                }
            }
            Button {
                text: qsTr("Delete a&ll")
                icon.source: Theme.icon("trash")
                icon.color: "transparent"
                enabled: dialog.work.length > 0
                onClicked: {
                    dialog.work = []
                    listView.currentIndex = -1
                }
            }
            Item { Layout.fillWidth: true }
        }

        MenuSeparator { Layout.fillWidth: true }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("OK")
                onClicked: {
                    dialog.controller.bookmarks.clear()
                    for (let i = 0; i < dialog.work.length; ++i) {
                        dialog.controller.bookmarks.add(dialog.work[i].time,
                                                        dialog.work[i].name)
                    }
                    dialog.close()
                }
            }
            Button {
                text: qsTr("Cancel")
                onClicked: dialog.close()
            }
        }
    }
}
