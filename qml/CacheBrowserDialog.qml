/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Browser for the YouTube download cache: a thumbnail grid of the videos kept
    by download mode. A toolbar row on top filters by title (incremental, like
    Explorer), sorts by date or title (ascending/descending) and picks the
    thumbnail size; a trash button removes the checked videos after a
    confirmation. Click a thumbnail to play. Opened from Open > YouTube cache.
    `resolver` is the YoutubeResolver. Deletion is routed through the parent
    (deleteRequested) because the player must release a file before it can be
    deleted.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

Window {
    id: dialog

    property var resolver
    signal playRequested(string fileUrl)
    // The parent releases the player if needed, deletes each file, then reloads.
    signal deleteRequested(var fileUrls)

    title: qsTr("YouTube cache")
    flags: Qt.Dialog
    width: 820
    height: 560
    minimumWidth: 560
    minimumHeight: 360
    color: palette.window

    property var allEntries: []
    property bool sortDesc: true
    // fileUrl -> true for videos ticked for deletion.
    property var selected: ({})
    readonly property int selCount: {
        let n = 0
        for (const k in selected)
            if (selected[k]) n++
        return n
    }

    readonly property int thumbW: [150, 210, 300][
            Math.max(0, Math.min(2, Settings.youtubeCacheThumbSize))]
    readonly property int thumbH: Math.round(thumbW * 9 / 16)

    readonly property var view: {
        const f = filterField.text.toLowerCase().trim()
        const byTitle = sortCombo.currentIndex === 1
        const desc = dialog.sortDesc
        let list = []
        for (let i = 0; i < allEntries.length; i++) {
            const e = allEntries[i]
            if (f === "" || e.title.toLowerCase().indexOf(f) >= 0)
                list.push(e)
        }
        list.sort(function (a, b) {
            const r = byTitle ? a.title.localeCompare(b.title)
                              : (a.modified - b.modified)
            return desc ? -r : r
        })
        return list
    }

    function reload() {
        allEntries = resolver ? resolver.cacheEntries() : []
    }

    function setSelected(fileUrl, on) {
        let s = {}
        for (const k in selected)
            s[k] = selected[k]
        if (on)
            s[fileUrl] = true
        else
            delete s[fileUrl]
        selected = s
    }

    function doDelete() {
        let urls = []
        for (const k in selected)
            if (selected[k]) urls.push(k)
        selected = {}
        if (urls.length > 0)
            dialog.deleteRequested(urls) // parent deletes + calls reload()
    }

    function openDialog() {
        filterField.clear()
        selected = {}
        reload()
        if (transientParent) {
            x = transientParent.x + (transientParent.width - width) / 2
            y = transientParent.y + (transientParent.height - height) / 2
        }
        visible = true
        raise()
        requestActivate()
    }

    Shortcut { sequences: [StandardKey.Cancel]; onActivated: dialog.close() }

    MessageDialog {
        id: confirmDelete
        title: qsTr("Remove from cache")
        text: qsTr("Are you sure you want to remove %1 selected cache file(s)?")
                    .arg(dialog.selCount)
        buttons: MessageDialog.Yes | MessageDialog.No
        onAccepted: dialog.doDelete()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // ---- toolbar: filter / sort / order / thumbnail size / delete ----
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label { text: qsTr("Filter:") }
            TextField {
                id: filterField
                Layout.fillWidth: true
                placeholderText: qsTr("type to narrow the list")
            }

            Label { text: qsTr("Sort:") }
            ComboBox {
                id: sortCombo
                model: [qsTr("Date"), qsTr("Title")]
                currentIndex: 0
            }
            ToolButton {
                text: dialog.sortDesc ? "▼" : "▲" // ▼ / ▲
                ToolTip.visible: hovered
                ToolTip.text: dialog.sortDesc ? qsTr("Descending") : qsTr("Ascending")
                onClicked: dialog.sortDesc = !dialog.sortDesc
            }

            ToolSeparator {}

            Label { text: qsTr("Size:") }
            ComboBox {
                id: sizeCombo
                model: [qsTr("Small"), qsTr("Medium"), qsTr("Large")]
                currentIndex: Settings.youtubeCacheThumbSize
                onActivated: Settings.youtubeCacheThumbSize = currentIndex
            }

            ToolSeparator {}

            ToolButton {
                icon.source: Theme.icon("delete")
                icon.color: "transparent"
                enabled: dialog.selCount > 0
                // The full-colour PNG isn't tinted, so fade it when disabled to
                // read as grayed-out.
                opacity: enabled ? 1.0 : 0.35
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Remove the checked videos from the cache")
                onClicked: confirmDelete.open()
            }
            HelpMark {
                text: qsTr("Tick videos with the checkbox in their corner, then "
                           + "use this button to remove them from the cache "
                           + "(you'll be asked to confirm).")
            }
        }

        Label {
            visible: dialog.view.length === 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            opacity: 0.6
            text: dialog.allEntries.length === 0
                  ? qsTr("The cache is empty.")
                  : qsTr("No videos match the filter.")
        }

        GridView {
            id: grid
            visible: dialog.view.length > 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            cellWidth: dialog.thumbW + 14
            cellHeight: dialog.thumbH + 46
            model: dialog.view

            ScrollBar.vertical: ScrollBar {}

            delegate: Item {
                id: cell
                required property var modelData
                width: grid.cellWidth
                height: grid.cellHeight

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 4

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: dialog.thumbH
                        color: "black"
                        radius: 3
                        clip: true

                        Image {
                            anchors.fill: parent
                            source: cell.modelData.thumbnailUrl
                            visible: cell.modelData.thumbnailUrl !== ""
                            // Fill the fixed 16:9 box uniformly (crop overflow),
                            // so thumbnails of different sizes/aspects all align.
                            fillMode: Image.PreserveAspectCrop
                            asynchronous: true
                            cache: false
                        }
                        Label {
                            anchors.centerIn: parent
                            visible: cell.modelData.thumbnailUrl === ""
                            text: "▶"
                            color: "#888888"
                            font.pixelSize: 36
                        }
                        Label {
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            anchors.margins: 3
                            padding: 2
                            background: Rectangle { color: "#c0000000"; radius: 2 }
                            text: cell.modelData.sizeText
                            color: "white"
                            font.pixelSize: 10
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: cell.modelData.title
                        elide: Text.ElideRight
                        maximumLineCount: 2
                        wrapMode: Text.Wrap
                        font.pixelSize: 12
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        dialog.playRequested(cell.modelData.fileUrl)
                        dialog.close()
                    }
                    ToolTip.visible: containsMouse
                    ToolTip.delay: 600
                    ToolTip.text: cell.modelData.title + "\n" + cell.modelData.sizeText
                }

                // Tick to mark for deletion. On top of (and after) the MouseArea
                // so its own clicks aren't treated as "play". A dark chip behind
                // it keeps it visible over bright thumbnails.
                Rectangle {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: 6
                    width: markBox.width + 4
                    height: markBox.height + 4
                    radius: 3
                    color: "#80000000"
                    CheckBox {
                        id: markBox
                        anchors.centerIn: parent
                        padding: 0
                        checked: dialog.selected[cell.modelData.fileUrl] === true
                        onToggled: dialog.setSelected(cell.modelData.fileUrl, checked)
                    }
                }
            }
        }

        MenuSeparator { Layout.fillWidth: true }

        RowLayout {
            Layout.fillWidth: true
            Label {
                opacity: 0.6
                text: dialog.selCount > 0
                      ? qsTr("%1 selected of %2").arg(dialog.selCount).arg(dialog.view.length)
                      : qsTr("%1 video(s)").arg(dialog.view.length)
            }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("Close")
                onClicked: dialog.close()
            }
        }
    }
}
