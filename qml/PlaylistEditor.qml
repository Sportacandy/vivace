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

    // Row/thumbnail size (Preferences-free, toggled from this toolbar):
    // 0 = small (default), 1 = medium, 2 = large. Text stays the same size
    // regardless; only the row height and the thumbnail grow.
    readonly property var rowHeights: [28, 48, 72]
    readonly property int rowHeight: rowHeights[Settings.playlistThumbnailSize]
    readonly property int thumbHeight: rowHeight - 6

    // In "small" mode the inline thumbnail is tiny, so the SELECTED row
    // alone grows tall enough for a real thumbnail (rather than popping up
    // a separate preview window) -- other rows stay at the small height.
    readonly property int expandedRowHeight: 120
    readonly property int expandedThumbHeight: expandedRowHeight - 6

    // macOS Dock-style "wavy" magnification: rather than only the exact
    // selected row jumping to the expanded size, the 2 rows on either side
    // taper toward it too (1 away = 66% of the way there, 2 away = 33%),
    // so the growth reads as a smooth wave through the list instead of a
    // single row abruptly popping.
    function waveFactor(distance) {
        switch (distance) {
        case 0: return 1.0
        case 1: return 0.4
        case 2: return 0.2
        default: return 0.0
        }
    }

    // Safety net: a selection made via keyboard/toolbar navigation could
    // land on an index whose delegate was never instantiated (scrolled
    // past without being drawn, so it never went through prefetchThumbnails
    // below) or whose request raced ahead of the bulk prefetch.
    onSelectedIndexChanged: {
        if (editor.selectedIndex >= 0)
            PlaylistThumbnailProvider.requestThumbnail(
                    editor.controller.playlist.urlAt(editor.selectedIndex))
    }

    // Requests thumbnails for every entry in playlist ORDER (index 0
    // first), in one uninterrupted pass, so generation happens top-to-
    // bottom -- the order the user actually looks at the list -- rather
    // than whatever order the ListView happens to instantiate row
    // delegates in. That order depends on viewport/cache-buffer/layout
    // details the view doesn't guarantee to be sequential, and was
    // observed (user report, a 70-entry playlist) to start generating
    // thumbnails from around entry 35 while the top of the list -- what
    // the user is actually looking at -- stayed blank for minutes.
    // PlaylistThumbnailProvider dedupes already-queued/resolved/in-flight
    // paths, so calling this repeatedly (every time the playlist grows) is
    // cheap; requests already sitting in its FIFO queue keep their place.
    function prefetchThumbnails() {
        const list = editor.controller.playlist
        for (let i = 0; i < list.count; ++i)
            PlaylistThumbnailProvider.requestThumbnail(list.urlAt(i))
    }

    Component.onCompleted: editor.prefetchThumbnails()

    onVisibleChanged: {
        if (editor.visible)
            listView.forceActiveFocus()
    }

    Connections {
        target: editor.controller.playlist
        function onCountChanged() { editor.prefetchThumbnails() }
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

        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            Label {
                Layout.fillWidth: true
                // Named after the backing .m3u/.m3u8 file (without
                // extension) when the in-memory playlist corresponds to
                // one; otherwise the generic "Playlist" label.
                text: {
                    const url = editor.controller.currentPlaylistFile
                    const count = editor.controller.playlist.count
                    if (!url || url.toString() === "")
                        return qsTr("Playlist (%1)").arg(count)
                    const path = UiHelpers.toLocalPath(url)
                    const sep = Math.max(path.lastIndexOf('/'), path.lastIndexOf('\\'))
                    const base = path.substring(sep + 1)
                    const dot = base.lastIndexOf('.')
                    const baseName = dot > 0 ? base.substring(0, dot) : base
                    return qsTr("Playlist: %1 (%2)").arg(baseName).arg(count)
                }
                elide: Text.ElideRight
                color: "white"
                font.bold: true
            }

            // Row/thumbnail size selector, top-right corner as requested.
            ToolButton {
                icon.source: Theme.icon("video_size")
                icon.color: "transparent"
                ToolTip.text: qsTr("Row/thumbnail size")
                ToolTip.visible: hovered
                ToolTip.delay: 700
                onClicked: rowSizeMenu.popup()

                Menu {
                    id: rowSizeMenu
                    MenuItem {
                        text: qsTr("Small")
                        checkable: true
                        checked: Settings.playlistThumbnailSize === 0
                        onTriggered: Settings.playlistThumbnailSize = 0
                    }
                    MenuItem {
                        text: qsTr("Medium")
                        checkable: true
                        checked: Settings.playlistThumbnailSize === 1
                        onTriggered: Settings.playlistThumbnailSize = 1
                    }
                    MenuItem {
                        text: qsTr("Large")
                        checkable: true
                        checked: Settings.playlistThumbnailSize === 2
                        onTriggered: Settings.playlistThumbnailSize = 2
                    }
                }
            }
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
            activeFocusOnTab: true
            focus: true
            clip: true
            currentIndex: -1
            model: editor.controller.playlist

            ScrollBar.vertical: ScrollBar {}

            delegate: Rectangle {
                id: entry

                required property int index
                required property string title
                required property string duration
                required property url url

                readonly property bool isPlaying:
                    index === editor.controller.playlist.currentIndex
                readonly property bool matches: editor.matchesSearch(title)

                // Rows near the selection grow toward a real-sized
                // thumbnail instead of the row's normal inline one, in
                // every size mode -- including "large", whose own inline
                // thumbnail is still smaller than the expanded size.
                readonly property real waveFactor:
                    listView.currentIndex >= 0
                        ? editor.waveFactor(Math.abs(entry.index - listView.currentIndex))
                        : 0.0
                readonly property real thumbSize:
                    editor.thumbHeight
                        + entry.waveFactor * (editor.expandedThumbHeight - editor.thumbHeight)

                // Resolved once at creation (a sibling .jpg or a previously
                // generated cache entry, if either already exists); updated
                // live by the Connections below once a background
                // generation (requested in bulk, in playlist order, by
                // editor.prefetchThumbnails()) finishes. Deliberately does
                // NOT also call requestThumbnail() here: doing so raced
                // against the ordered bulk prefetch (whichever rows the
                // ListView happened to instantiate first jumped the queue),
                // which is exactly the bug prefetchThumbnails() exists to
                // avoid.
                property string thumbnailSrc:
                        PlaylistThumbnailProvider.thumbnailFor(entry.url)

                width: ListView.view.width
                height: matches
                        ? (editor.rowHeight
                           + entry.waveFactor * (editor.expandedRowHeight - editor.rowHeight))
                        : 0
                Behavior on height { NumberAnimation { duration: 120 } }
                visible: matches
                clip: true
                color: ListView.isCurrentItem ? "#3d5a78" : "transparent"

                Connections {
                    target: PlaylistThumbnailProvider
                    function onThumbnailReady(mediaUrl, thumbnailUrl) {
                        // Compare decoded local paths, NOT raw URL strings:
                        // entry.url (parsed from an .m3u8 by PlaylistParser)
                        // and mediaUrl (built in C++ via
                        // QUrl::fromLocalFile()) can percent-encode the same
                        // path differently for non-ASCII/space filenames --
                        // observed directly on a real playlist with CJK/
                        // emoji titles, where a raw-string compare never
                        // matched for ANY of them, so the live update never
                        // fired (only a fresh delegate's synchronous cache
                        // check -- e.g. after scrolling -- ever picked the
                        // thumbnail up). QML's "url" type has no callable
                        // .toLocalFile() (a plain C++ QUrl method, not
                        // exposed to JS -- confirmed by a TypeError at
                        // runtime); UiHelpers.toLocalPath() is this
                        // project's established C++-side equivalent,
                        // already used elsewhere in this file.
                        if (UiHelpers.toLocalPath(mediaUrl) === UiHelpers.toLocalPath(entry.url))
                            entry.thumbnailSrc = thumbnailUrl
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 6
                    anchors.rightMargin: 6
                    spacing: 8

                    Rectangle {
                        Layout.preferredWidth: entry.thumbSize * 16 / 9
                        Layout.preferredHeight: entry.thumbSize
                        Behavior on Layout.preferredWidth { NumberAnimation { duration: 120 } }
                        Behavior on Layout.preferredHeight { NumberAnimation { duration: 120 } }
                        Layout.alignment: Qt.AlignVCenter
                        color: "#1a1a1a"
                        radius: 2
                        clip: true
                        Image {
                            anchors.fill: parent
                            source: entry.thumbnailSrc
                            visible: entry.thumbnailSrc !== ""
                            fillMode: Image.PreserveAspectCrop
                            asynchronous: true
                            cache: false
                        }
                    }
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
