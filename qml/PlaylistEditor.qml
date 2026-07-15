/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Playlist editor content, following SMPlayer's playlist window: load/
    save playlist, add files/URL, remove selected/all, play/prev/next,
    repeat & shuffle, move up/down. Single click selects, double click
    plays; the playing entry is shown in bold blue. Hosted either by the
    docked drawer (PlaylistPanel) or the separate window (PlaylistWindow),
    per Settings.playlistAsWindow.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

Item {
    id: editor

    required property PlayerController controller

    readonly property int selectedIndex: listView.currentIndex

    property string searchText: ""

    // Case sensitivity follows the Playlist preference.
    function matchesSearch(t) {
        if (editor.searchText === "")
            return true
        return Settings.caseSensitiveSearch
                ? t.indexOf(editor.searchText) >= 0
                : t.toLowerCase().indexOf(editor.searchText.toLowerCase()) >= 0
    }

    // Uniform toolbar row height so separators line up with the buttons.
    readonly property int toolbarRowHeight: 30

    component Btn: ToolButton {
        height: editor.toolbarRowHeight
        icon.width: 20
        icon.height: 20
        icon.color: "transparent"
        display: AbstractButton.IconOnly
        ToolTip.visible: hovered && ToolTip.text !== ""
        ToolTip.delay: 700
    }

    // A short, narrow separator line, vertically centered on the toolbar row
    // (like SMPlayer's QToolBar separators) rather than top-aligned, full-
    // height or wide.
    component Sep: Item {
        height: editor.toolbarRowHeight
        implicitWidth: 9
        ToolSeparator {
            anchors.centerIn: parent
            height: 18
            padding: 0 // drop the wide default horizontal padding
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Label {
            text: qsTr("Playlist (%1)").arg(editor.controller.playlist.count)
            color: "white"
            font.bold: true
        }

        // Search box: filters the visible rows by name.
        RowLayout {
            Layout.fillWidth: true
            spacing: 4
            TextField {
                id: searchField
                Layout.fillWidth: true
                placeholderText: qsTr("Search…")
                color: "white"
                selectByMouse: true
                onTextChanged: editor.searchText = text
                background: Rectangle {
                    color: "#2a2a2a"
                    border.color: searchField.activeFocus ? "#4da2f0" : "#555"
                    radius: 3
                }
            }
            ToolButton {
                text: "✕"
                visible: searchField.text !== ""
                onClicked: searchField.clear()
                ToolTip.text: qsTr("Clear")
                ToolTip.visible: hovered
            }
            HelpMark {
                text: qsTr("Hides entries whose name does not match; it does not "
                           + "remove them from the playlist.")
            }
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            currentIndex: -1
            model: editor.controller.playlist

            ScrollBar.vertical: ScrollBar {}

            delegate: Rectangle {
                id: entry

                required property int index
                required property string title
                required property string duration

                readonly property bool isPlaying:
                    index === editor.controller.playlist.currentIndex
                readonly property bool matches: editor.matchesSearch(title)

                width: ListView.view.width
                height: matches ? 28 : 0
                visible: matches
                clip: true
                color: ListView.isCurrentItem ? "#3d5a78" : "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 6
                    anchors.rightMargin: 6
                    spacing: 8

                    Label {
                        Layout.fillWidth: true
                        text: entry.title
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        font.bold: entry.isPlaying
                        color: entry.isPlaying ? "#4da2f0" : "white"
                    }
                    Label {
                        text: entry.duration
                        visible: text !== ""
                        font.pixelSize: 12
                        color: entry.isPlaying ? "#4da2f0" : "#b0b0b0"
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    // Index being dragged (-1 = not dragging).
                    property int dragIndex: -1
                    onPressed: {
                        listView.currentIndex = entry.index
                        dragIndex = entry.index
                    }
                    onReleased: dragIndex = -1
                    onCanceled: dragIndex = -1
                    onDoubleClicked: editor.controller.playAt(entry.index)
                    onPositionChanged: {
                        // Live drag-reorder; disabled while searching (rows
                        // are collapsed, so indexAt would be unreliable).
                        if (dragIndex < 0 || editor.searchText !== "")
                            return
                        const pt = mapToItem(listView.contentItem, mouseX, mouseY)
                        const target = listView.indexAt(pt.x, pt.y)
                        if (target >= 0 && target !== dragIndex) {
                            editor.controller.playlist.move(dragIndex, target)
                            dragIndex = target
                            listView.currentIndex = target
                        }
                    }
                }
            }

            Label {
                anchors.centerIn: parent
                visible: listView.count === 0
                text: qsTr("Playlist is empty")
                color: "#808080"
            }
        }

        // Editor toolbar, mirroring SMPlayer's playlist toolbar.
        Flow {
            Layout.fillWidth: true
            spacing: 0

            Btn {
                icon.source: Theme.icon("open_playlist")
                ToolTip.text: qsTr("Load playlist…")
                onClicked: loadPlaylistDialog.open()
            }
            Btn {
                icon.source: Theme.icon("save")
                ToolTip.text: qsTr("Save playlist…")
                enabled: editor.controller.playlist.count > 0
                onClicked: savePlaylistDialog.open()
            }

            Sep {}

            Btn {
                icon.source: Theme.icon("plus")
                ToolTip.text: qsTr("Add files…")
                onClicked: addFilesDialog.open()
            }
            Btn {
                icon.source: Theme.icon("url")
                ToolTip.text: qsTr("Add URL…")
                onClicked: addUrlDialog.open()
            }

            Sep {}

            Btn {
                icon.source: Theme.icon("minus")
                ToolTip.text: qsTr("Remove selected")
                enabled: editor.selectedIndex >= 0
                onClicked: editor.controller.playlist.removeAt(editor.selectedIndex)
            }
            Btn {
                icon.source: Theme.icon("trash")
                ToolTip.text: qsTr("Remove all")
                enabled: editor.controller.playlist.count > 0
                onClicked: editor.controller.playlist.clear()
            }

            Sep {}

            Btn {
                icon.source: Theme.icon("play")
                ToolTip.text: qsTr("Play selected")
                enabled: editor.selectedIndex >= 0
                onClicked: editor.controller.playAt(editor.selectedIndex)
            }
            Btn {
                icon.source: Theme.icon("previous")
                ToolTip.text: qsTr("Previous")
                enabled: editor.controller.playlist.count > 0
                onClicked: editor.controller.previous()
            }
            Btn {
                icon.source: Theme.icon("next")
                ToolTip.text: qsTr("Next")
                enabled: editor.controller.playlist.count > 0
                onClicked: editor.controller.next()
            }

            Sep {}

            Btn {
                icon.source: Theme.icon("repeat")
                ToolTip.text: qsTr("Repeat playlist")
                checkable: true
                checked: Settings.playlistRepeat
                onClicked: Settings.playlistRepeat = checked
            }
            Btn {
                icon.source: Theme.icon("shuffle")
                ToolTip.text: qsTr("Shuffle")
                checkable: true
                checked: Settings.playlistShuffle
                onClicked: Settings.playlistShuffle = checked
            }

            Sep {}

            Btn {
                icon.source: Theme.icon("up")
                ToolTip.text: qsTr("Move up")
                enabled: editor.selectedIndex > 0
                onClicked: {
                    const i = editor.selectedIndex
                    editor.controller.playlist.move(i, i - 1)
                    listView.currentIndex = i - 1
                }
            }
            Btn {
                icon.source: Theme.icon("down")
                ToolTip.text: qsTr("Move down")
                enabled: editor.selectedIndex >= 0
                         && editor.selectedIndex < editor.controller.playlist.count - 1
                onClicked: {
                    const i = editor.selectedIndex
                    editor.controller.playlist.move(i, i + 1)
                    listView.currentIndex = i + 1
                }
            }
        }
    }

    FileDialog {
        id: addFilesDialog
        fileMode: FileDialog.OpenFiles
        nameFilters: [
            qsTr("Media files (*.mp4 *.mkv *.avi *.mov *.webm *.wmv *.ts *.m2ts *.flv *.ogv *.mp3 *.m4a *.flac *.ogg *.opus *.wav *.wma *.m3u *.m3u8)"),
            qsTr("All files (*)")
        ]
        onAccepted: {
            Settings.lastOpenFolder = currentFolder
            editor.controller.enqueue(selectedFiles)
        }
        Component.onCompleted: {
            if (Settings.lastOpenFolder.toString() !== "")
                currentFolder = Settings.lastOpenFolder
        }
    }

    FileDialog {
        id: loadPlaylistDialog
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("Playlists (*.m3u *.m3u8)"), qsTr("All files (*)")]
        onAccepted: {
            Settings.lastOpenFolder = currentFolder
            editor.controller.open([selectedFile])
        }
    }

    FileDialog {
        id: savePlaylistDialog
        fileMode: FileDialog.SaveFile
        defaultSuffix: "m3u8"
        nameFilters: [qsTr("Playlists (*.m3u8 *.m3u)")]
        onAccepted: editor.controller.savePlaylist(selectedFile)
    }

    Window {
        id: addUrlDialog
        title: qsTr("Add URL")
        flags: Qt.Dialog
        modality: Qt.WindowModal
        color: palette.window
        width: 420
        height: addUrlCol.implicitHeight + 24
        minimumWidth: 320
        minimumHeight: addUrlCol.implicitHeight + 24

        function open() {
            addUrlField.clear()
            if (transientParent) {
                x = transientParent.x + (transientParent.width - width) / 2
                y = transientParent.y + (transientParent.height - height) / 2
            }
            visible = true
            raise()
            requestActivate()
            addUrlField.forceActiveFocus()
        }

        function submit() {
            if (addUrlField.text.trim() !== "")
                editor.controller.enqueue([addUrlField.text.trim()])
            addUrlDialog.close()
        }

        Shortcut {
            sequences: [StandardKey.Cancel]
            onActivated: addUrlDialog.close()
        }

        ColumnLayout {
            id: addUrlCol
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            TextField {
                id: addUrlField
                Layout.fillWidth: true
                placeholderText: qsTr("https://…")
                onAccepted: addUrlDialog.submit()
            }

            MenuSeparator { Layout.fillWidth: true }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("OK")
                    enabled: addUrlField.text.trim() !== ""
                    onClicked: addUrlDialog.submit()
                }
                Button {
                    text: qsTr("Cancel")
                    onClicked: addUrlDialog.close()
                }
            }
        }
    }
}
