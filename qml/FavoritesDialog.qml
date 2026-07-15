/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Favorite editor, following SMPlayer's FavoriteEditor layout: a header
    (icon + title), a Name/Media table (always shown, inline editing), a
    2x3 button grid (New item / Delete / Up · New submenu / Delete all /
    Down) and a Close button. "New item" appends a blank row to edit
    inline; double-clicking a submenu row opens it.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: dialog

    // The list to edit (favorites, TV or radio) and its labels; set by
    // openFor() before showing.
    property var favorites: null
    property string caption: qsTr("Favorite list")
    property string headerIcon: Theme.icon("open_favorites")
    property string path: ""

    title: qsTr("Editor")
    flags: Qt.Dialog
    width: 620
    height: 520
    minimumWidth: 480
    minimumHeight: 380
    color: "#f0f0f0"

    function openFor(titleText, captionText, iconSource, model) {
        title = titleText
        caption = captionText
        headerIcon = iconSource
        favorites = model
        path = ""
        listView.currentIndex = -1
        if (transientParent) {
            x = transientParent.x + (transientParent.width - width) / 2
            y = transientParent.y + (transientParent.height - height) / 2
        }
        visible = true
    }

    Shortcut { sequence: "Escape"; onActivated: dialog.close() }

    // The parent of a "2/0"-style path ("" = root).
    function parentPath(p) {
        if (p === "")
            return ""
        const parts = p.split("/")
        parts.pop()
        return parts.join("/")
    }

    // Breadcrumb entries [{ label, path }] from the root down to `p`.
    function crumbModel(p) {
        const res = [{ label: dialog.caption, path: "" }]
        if (p === "" || !dialog.favorites)
            return res
        const parts = p.split("/")
        let acc = ""
        for (let i = 0; i < parts.length; ++i) {
            const idx = parseInt(parts[i])
            const rows = dialog.favorites.items(acc)
            const nm = (rows && rows[idx]) ? rows[idx].name : "?"
            acc = acc === "" ? String(idx) : acc + "/" + idx
            res.push({ label: nm, path: acc })
        }
        return res
    }

    // Recomputed when the path or the tree changes.
    readonly property var crumbData: {
        const rev = dialog.favorites ? dialog.favorites.revision : 0
        return dialog.crumbModel(dialog.path)
    }

    function navigateTo(p) {
        dialog.path = p
        listView.currentIndex = -1
    }

    // Descend into the selected row if it is a submenu.
    function openSelected() {
        const i = listView.currentIndex
        const rows = listView.model
        if (i < 0 || !rows[i] || rows[i].isSubmenu !== true)
            return
        dialog.navigateTo(dialog.path === "" ? String(i) : dialog.path + "/" + i)
    }

    readonly property bool selectionIsSubmenu:
        listView.currentIndex >= 0
        && listView.model[listView.currentIndex]
        && listView.model[listView.currentIndex].isSubmenu === true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        // Header: icon + caption + intro (SMPlayer's title_label).
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Image {
                source: dialog.headerIcon
                sourceSize: Qt.size(48, 48)
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2
                Label {
                    text: dialog.caption
                    font.pixelSize: 20
                    font.bold: true
                }
                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    opacity: 0.8
                    text: qsTr("You can edit, delete, sort or add new items. Select a submenu and press Open (or double-click it) to edit its contents.")
                }
            }
        }

        // Navigation bar: Up-one-level button, breadcrumb path, Open button.
        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Button {
                // A framed button (not a flat ToolButton) so it reads as one.
                display: AbstractButton.IconOnly
                icon.source: Theme.icon("goback")
                icon.color: "transparent"
                enabled: dialog.path !== ""
                ToolTip.text: qsTr("Up one level")
                ToolTip.visible: hovered
                ToolTip.delay: 500
                onClicked: dialog.navigateTo(dialog.parentPath(dialog.path))
            }

            Flow {
                Layout.fillWidth: true
                spacing: 3
                Repeater {
                    model: dialog.crumbData
                    delegate: Row {
                        id: crumb
                        required property int index
                        required property var modelData
                        readonly property bool last: index === dialog.crumbData.length - 1
                        spacing: 3
                        Label {
                            visible: crumb.index > 0
                            text: "▸"
                            opacity: 0.5
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            text: crumb.modelData.label
                            font.bold: crumb.last
                            color: crumb.last ? "#1a1a1a" : "#1e6fd0"
                            anchors.verticalCenter: parent.verticalCenter
                            MouseArea {
                                anchors.fill: parent
                                enabled: !crumb.last
                                cursorShape: Qt.PointingHandCursor
                                onClicked: dialog.navigateTo(crumb.modelData.path)
                            }
                        }
                    }
                }
            }

            Button {
                text: qsTr("&Open")
                icon.source: Theme.icon("open_favorites")
                icon.color: "transparent"
                enabled: dialog.selectionIsSubmenu
                onClicked: dialog.openSelected()
            }
            HelpMark {
                text: qsTr("Submenus group items into folders; select one and press "
                           + "Open (or double-click it) to edit inside, then use the "
                           + "path above to go back.")
            }
        }

        // Table: bordered, header row, always visible even when empty.
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            border.color: "#a0a0a0"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 1
                spacing: 0

                // Column headers.
                Row {
                    Layout.fillWidth: true
                    height: 26
                    Rectangle {
                        width: 40; height: parent.height; color: "#e0e0e0"
                        border.color: "#c0c0c0"
                    }
                    Rectangle {
                        width: dialog.width * 0.35; height: parent.height
                        color: "#e0e0e0"; border.color: "#c0c0c0"
                        Label { anchors.centerIn: parent; text: qsTr("Name") }
                    }
                    Rectangle {
                        width: parent.width - 40 - dialog.width * 0.35
                        height: parent.height; color: "#e0e0e0"
                        border.color: "#c0c0c0"
                        Label {
                            anchors.left: parent.left
                            anchors.leftMargin: 6
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Media")
                        }
                    }
                }

                ListView {
                    id: listView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    currentIndex: -1
                    model: dialog.favorites && dialog.favorites.revision >= 0
                           ? dialog.favorites.items(dialog.path) : []

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
                            onDoubleClicked: {
                                if (row.modelData.isSubmenu) {
                                    dialog.path = dialog.path === ""
                                            ? String(row.index)
                                            : dialog.path + "/" + row.index
                                    listView.currentIndex = -1
                                }
                            }
                        }

                        Row {
                            anchors.fill: parent
                            Item {
                                width: 40; height: parent.height
                                Image {
                                    anchors.centerIn: parent
                                    sourceSize: Qt.size(16, 16)
                                    source: row.modelData.isSubmenu
                                            ? Theme.icon("open_favorites")
                                            : Theme.icon("favorite")
                                }
                            }
                            TextField {
                                width: dialog.width * 0.35
                                height: parent.height
                                text: row.modelData.name
                                background: null
                                onEditingFinished: dialog.favorites.updateItem(
                                        dialog.path, row.index, text,
                                        row.modelData.url)
                            }
                            TextField {
                                width: parent.width - 40 - dialog.width * 0.35
                                height: parent.height
                                text: row.modelData.isSubmenu
                                      ? qsTr("(submenu)") : row.modelData.url
                                readOnly: row.modelData.isSubmenu
                                background: null
                                onEditingFinished: {
                                    if (!row.modelData.isSubmenu)
                                        dialog.favorites.updateItem(
                                            dialog.path, row.index,
                                            row.modelData.name, text)
                                }
                            }
                        }
                    }
                }
            }
        }

        // Button grid, 2 rows x 3 columns (SMPlayer's layout).
        GridLayout {
            Layout.fillWidth: true
            columns: 3
            columnSpacing: 6
            rowSpacing: 6

            Button {
                Layout.fillWidth: true
                text: qsTr("&New item")
                icon.source: Theme.icon("favorite-add")
                icon.color: "transparent"
                onClicked: {
                    dialog.favorites.addItem(dialog.path, "", "")
                    listView.currentIndex = listView.count - 1
                }
            }
            Button {
                Layout.fillWidth: true
                text: qsTr("D&elete")
                icon.source: Theme.icon("delete")
                icon.color: "transparent"
                enabled: listView.currentIndex >= 0
                onClicked: {
                    dialog.favorites.removeAt(dialog.path, listView.currentIndex)
                    listView.currentIndex = -1
                }
            }
            Button {
                Layout.fillWidth: true
                text: qsTr("&Up")
                icon.source: Theme.icon("up")
                icon.color: "transparent"
                enabled: listView.currentIndex > 0
                onClicked: {
                    const i = listView.currentIndex
                    dialog.favorites.move(dialog.path, i, i - 1)
                    listView.currentIndex = i - 1
                }
            }
            Button {
                Layout.fillWidth: true
                text: qsTr("New &submenu")
                icon.source: Theme.icon("favorite-folder")
                icon.color: "transparent"
                onClicked: {
                    dialog.favorites.addSubmenu(dialog.path, qsTr("New submenu"))
                    listView.currentIndex = listView.count - 1
                }
            }
            Button {
                Layout.fillWidth: true
                text: qsTr("Delete &all")
                icon.source: Theme.icon("trash")
                icon.color: "transparent"
                enabled: listView.count > 0
                onClicked: {
                    dialog.favorites.clearAt(dialog.path)
                    listView.currentIndex = -1
                }
            }
            Button {
                Layout.fillWidth: true
                text: qsTr("&Down")
                icon.source: Theme.icon("down")
                icon.color: "transparent"
                enabled: listView.currentIndex >= 0
                         && listView.currentIndex < listView.count - 1
                onClicked: {
                    const i = listView.currentIndex
                    dialog.favorites.move(dialog.path, i, i + 1)
                    listView.currentIndex = i + 1
                }
            }
        }

        MenuSeparator { Layout.fillWidth: true }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button { text: qsTr("Close"); onClicked: dialog.close() }
        }
    }
}
