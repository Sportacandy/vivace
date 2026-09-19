/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Main toolbar under the menu bar (SMPlayer's toolbar1). Editable: the
    visible buttons and their order come from Settings.mainToolbarItems
    (catalog + default in ToolbarItems.js). Since the editor forbids
    duplicates, every button is a single predefined instance whose
    visibility and column are bound to the layout list — no per-id
    dispatch. Separators/spacers (repeatable) come from small Repeaters.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import "ToolbarItems.js" as Items

ToolBar {
    id: toolBar

    required property PlayerController controller
    required property bool playlistOpen
    // Number of videos in the YouTube download cache (drives the cache-browser
    // button's enabled state) -- mirrors MainMenuBar's own property of the
    // same name.
    property int youtubeCacheCount: 0

    signal openFileRequested()
    signal openUrlRequested()
    signal openDvdRequested()
    signal openBlurayRequested()
    signal openDirectoryRequested()
    signal playlistToggleRequested()
    signal screenshotRequested()
    signal infoRequested()
    signal preferencesRequested()
    signal fullscreenToggleRequested()
    signal editFavoritesRequested()
    signal youtubeCacheRequested()
    signal castRequested()
    signal editTvChannelsRequested()
    signal editRadioChannelsRequested()
    // Bubbled from favoritesPopup/tvPopup/radioPopup below -- see
    // FavoritesMenu.qml's own urlActivated doc comment for why this needs
    // to route through Main.qml's openMediaUrl() rather than opening
    // directly.
    signal urlActivated(string url)
    signal videoEqualizerRequested()
    signal resizeToVideoPercentRequested(int percent)
    signal setAudioDelayRequested()
    signal loadSubtitlesRequested()
    signal findSubtitlesRequested()
    signal setSubtitleDelayRequested()
    signal addBookmarkRequested()
    signal editBookmarksRequested()
    signal openPlaylistRequested()
    signal helpContentsRequested()
    signal checkForUpdatesRequested()
    signal aboutRequested()

    readonly property QtObject player: controller.player

    // Forced display aspect ratios (SMPlayer's Video > Aspect ratio); 0 = auto.
    // Mirrors MainMenuBar's own array of the same name/shape.
    readonly property var aspectRatios: [
        { label: qsTr("Auto"), value: 0 },
        { label: "1:1", value: 1 / 1 },
        { label: "5:4", value: 5 / 4 },
        { label: "4:3", value: 4 / 3 },
        { label: "11:8", value: 11 / 8 },
        { label: "14:10", value: 14 / 10 },
        { label: "3:2", value: 3 / 2 },
        { label: "14:9", value: 14 / 9 },
        { label: "16:10", value: 16 / 10 },
        { label: "16:9", value: 16 / 9 },
        { label: "2.35:1", value: 2.35 }
    ]

    // Same semantics as MainMenuBar's adjustSpeed()/adjustSpeedStep(): the
    // former is multiplicative (Halve/Double speed), the latter a fixed
    // additive step (+/-10%, matching SMPlayer's Core::incSpeed10/etc).
    function adjustSpeed(factor) {
        Settings.playbackRate =
                Math.max(0.1, Math.round(Settings.playbackRate * factor * 100) / 100)
    }
    function adjustSpeedStep(delta) {
        Settings.playbackRate =
                Math.max(0.1, Math.round((Settings.playbackRate + delta) * 100) / 100)
    }

    readonly property var layoutItems: Settings.mainToolbarItems.length > 0
                                        ? Settings.mainToolbarItems
                                        : Items.defaultMainToolbar

    // Column of an item in the current layout, or -1 when absent.
    function col(id) { return layoutItems.indexOf(id) }
    // Layout positions of every repeatable separator / spacer.
    function positionsOf(id) {
        var out = []
        for (var i = 0; i < layoutItems.length; ++i)
            if (layoutItems[i] === id)
                out.push(i)
        return out
    }

    // Row-wrap support: a phone screen in portrait is much narrower than a
    // typical toolbar's natural width, so items wrap onto additional rows
    // instead of clipping/overflowing off the right edge. Ordinary buttons
    // and separators tile into a uniform-width grid (columnsPerRow columns
    // per row); "spacer" gets a dedicated full-width row of its own
    // (Layout.columnSpan) rather than sharing a column with them -- a
    // GridLayout column's width is shared by every row that uses it, so a
    // fillWidth item landing in an ordinary column would force that same
    // column wide in every other row too.
    readonly property real cellWidth: Theme.sz(Settings.mainToolbarIconSize) + 20
    // The containing WINDOW's width, not this Pane's own `width`: the
    // fullscreen overlay instance (Main.qml's fsToolBar) sets its own
    // width via a plain `width: parent.width` binding, and Pane/ToolBar's
    // "resize contentItem to fit its single child's implicit size"
    // mechanism (see Qt's own Pane docs) made columnsPerRow -- and
    // therefore the GridLayout's own implicitHeight -- transiently
    // entangled with that same local width binding closely enough that
    // Qt's declarative engine reported a real (if apparently harmless)
    // "Binding loop detected for property implicitHeight" for that
    // instance specifically (never for the docked header, whose width
    // ApplicationWindow assigns through a different mechanism). The
    // containing OS window's own width has no such entanglement.
    readonly property int columnsPerRow: Math.max(1, Math.floor(
        (Window.window ? Window.window.width : width) / cellWidth))
    readonly property var wrapPositions: computeWrapPositions()

    function computeWrapPositions() {
        const perRow = columnsPerRow
        // Fast path: everything already fits on one row -- keep the exact
        // original single-row placement (column = i, "spacer" inline like
        // every other item), since there is no second row for its
        // fillWidth column to spill into.
        if (layoutItems.length <= perRow) {
            const out = []
            for (let i = 0; i < layoutItems.length; ++i)
                out.push({ row: 0, column: i, span: 1 })
            return out
        }
        const out = []
        let row = 0, column = 0
        for (let i = 0; i < layoutItems.length; ++i) {
            if (layoutItems[i] === "spacer") {
                if (column > 0)
                    row++
                out.push({ row: row, column: 0, span: perRow })
                row++
                column = 0
                continue
            }
            if (column >= perRow) {
                row++
                column = 0
            }
            out.push({ row: row, column: column, span: 1 })
            column++
        }
        return out
    }
    function wrapRow(i) { return i >= 0 && wrapPositions[i] ? wrapPositions[i].row : 0 }
    function wrapColumn(i) { return i >= 0 && wrapPositions[i] ? wrapPositions[i].column : 0 }
    function wrapSpan(i) { return i >= 0 && wrapPositions[i] ? wrapPositions[i].span : 1 }

    // A toolbar button whose icon, tooltip, visibility and column all come
    // from its item id; instances add only enabled/checked/onClicked.
    component TBtn: ToolButton {
        id: btn
        property string itemId: ""
        property bool menuIndicator: false

        visible: toolBar.col(itemId) >= 0
        Layout.row: toolBar.wrapRow(toolBar.col(itemId))
        Layout.column: toolBar.wrapColumn(toolBar.col(itemId))

        icon.width: Theme.sz(Settings.mainToolbarIconSize)
        icon.height: Theme.sz(Settings.mainToolbarIconSize)
        icon.color: "transparent"
        icon.source: visible ? Theme.icon(Items.iconFor(itemId)) : ""
        display: AbstractButton.IconOnly
        ToolTip.text: Items.labelFor(itemId)
        ToolTip.visible: hovered && ToolTip.text !== ""
        ToolTip.delay: 700

        Canvas {
            visible: btn.menuIndicator
            width: 8; height: 6
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: 3
            anchors.bottomMargin: 4
            onPaint: {
                const ctx = getContext("2d")
                ctx.reset()
                ctx.beginPath()
                ctx.moveTo(0.5, 0.5); ctx.lineTo(width - 0.5, 0.5)
                ctx.lineTo(width / 2, height - 0.5); ctx.closePath()
                ctx.fillStyle = "#707070"; ctx.fill()
                ctx.lineWidth = 1; ctx.strokeStyle = "#303030"; ctx.stroke()
            }
        }
    }

    background: Rectangle {
        color: "#282828"
        Rectangle {
            anchors.fill: parent
            visible: Settings.toolbarGradient
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#cedce7" }
                GradientStop { position: 1.0; color: "#596a72" }
            }
        }
    }

    // Popup menus for the three menu buttons.
    FavoritesMenu {
        id: favoritesPopup
        controller: toolBar.controller
        model: toolBar.controller.favorites
        showActions: true
        onEditRequested: toolBar.editFavoritesRequested()
        onAddCurrentRequested: toolBar.controller.addCurrentTo(
                                   toolBar.controller.favorites)
        onUrlActivated: url => toolBar.urlActivated(url)
    }
    AppMenu {
        id: audioMenu
        Instantiator {
            model: toolBar.controller.audioTrackLabels
            delegate: AppMenuItem {
                required property int index
                required property string modelData
                text: modelData
                checkable: true
                checked: toolBar.controller.activeAudioTrack === index
                onTriggered: {
                    toolBar.controller.activeAudioTrack = index
                    checked = Qt.binding(() => toolBar.controller.activeAudioTrack === index)
                }
            }
            onObjectAdded: (index, object) => audioMenu.insertItem(index, object)
            onObjectRemoved: (index, object) => audioMenu.removeItem(object)
        }
    }
    AppMenu {
        id: subtitleMenu
        AppMenuItem {
            text: qsTr("&Off")
            checkable: true
            checked: toolBar.controller.activeSubtitleTrack === -1
            onTriggered: {
                toolBar.controller.activeSubtitleTrack = -1
                checked = Qt.binding(() => toolBar.controller.activeSubtitleTrack === -1)
            }
        }
        Instantiator {
            model: toolBar.controller.subtitleTrackLabels
            delegate: AppMenuItem {
                required property int index
                required property string modelData
                text: modelData
                checkable: true
                checked: toolBar.controller.activeSubtitleTrack === index
                onTriggered: {
                    toolBar.controller.activeSubtitleTrack = index
                    checked = Qt.binding(() => toolBar.controller.activeSubtitleTrack === index)
                }
            }
            onObjectAdded: (index, object) => subtitleMenu.insertItem(index + 1, object)
            onObjectRemoved: (index, object) => subtitleMenu.removeItem(object)
        }
    }
    FavoritesMenu {
        id: tvPopup
        controller: toolBar.controller
        model: toolBar.controller.tvChannels
        itemIcon: Theme.icon("open_tv")
        showActions: true
        onEditRequested: toolBar.editTvChannelsRequested()
        onAddCurrentRequested: toolBar.controller.addCurrentTo(toolBar.controller.tvChannels)
        onUrlActivated: url => toolBar.urlActivated(url)
    }
    FavoritesMenu {
        id: radioPopup
        controller: toolBar.controller
        model: toolBar.controller.radioChannels
        itemIcon: Theme.icon("open_radio")
        showActions: true
        onEditRequested: toolBar.editRadioChannelsRequested()
        onAddCurrentRequested: toolBar.controller.addCurrentTo(toolBar.controller.radioChannels)
        onUrlActivated: url => toolBar.urlActivated(url)
    }
    AppMenu {
        id: speedPopup
        AppMenuItem {
            text: qsTr("Normal speed")
            icon.source: Theme.icon("speed-x100")
            onTriggered: Settings.playbackRate = 1
        }
        MenuSeparator {}
        AppMenuItem {
            text: qsTr("Halve speed")
            icon.source: Theme.icon("speed-x050")
            onTriggered: toolBar.adjustSpeed(0.5)
        }
        AppMenuItem {
            text: qsTr("Double speed")
            icon.source: Theme.icon("speed-x200")
            onTriggered: toolBar.adjustSpeed(2)
        }
        MenuSeparator {}
        AppMenuItem {
            text: qsTr("Speed -10%")
            icon.source: Theme.icon("speed-10")
            onTriggered: toolBar.adjustSpeedStep(-0.1)
        }
        AppMenuItem {
            text: qsTr("Speed +10%")
            icon.source: Theme.icon("speed+10")
            onTriggered: toolBar.adjustSpeedStep(0.1)
        }
        MenuSeparator {}
        AppMenuItem { text: qsTr("Speed -4%"); icon.source: Theme.icon("speed-04"); onTriggered: toolBar.adjustSpeedStep(-0.04) }
        AppMenuItem { text: qsTr("Speed +4%"); icon.source: Theme.icon("speed+04"); onTriggered: toolBar.adjustSpeedStep(0.04) }
        MenuSeparator {}
        AppMenuItem { text: qsTr("Speed -1%"); icon.source: Theme.icon("speed-01"); onTriggered: toolBar.adjustSpeedStep(-0.01) }
        AppMenuItem { text: qsTr("Speed +1%"); icon.source: Theme.icon("speed+01"); onTriggered: toolBar.adjustSpeedStep(0.01) }
        MenuSeparator {}
        AppMenuItem {
            text: qsTr("Pitch compensation")
            checkable: true
            checked: Settings.pitchCompensation
            enabled: toolBar.player.pitchCompensationAvailability === MediaPlayer.Available
            onTriggered: {
                Settings.pitchCompensation = checked
                checked = Qt.binding(() => Settings.pitchCompensation)
            }
        }
    }
    AppMenu {
        id: videoTrackPopup
        Instantiator {
            model: toolBar.controller.videoTrackLabels.length === 0 ? 1 : 0
            delegate: AppMenuItem { text: qsTr("<empty>"); enabled: false }
            onObjectAdded: (index, object) => videoTrackPopup.insertItem(0, object)
            onObjectRemoved: (index, object) => videoTrackPopup.removeItem(object)
        }
        Instantiator {
            model: toolBar.controller.videoTrackLabels
            delegate: AppMenuItem {
                required property int index
                required property string modelData
                text: modelData
                checkable: true
                checked: toolBar.controller.activeVideoTrack === index
                onTriggered: {
                    toolBar.controller.activeVideoTrack = index
                    checked = Qt.binding(() => toolBar.controller.activeVideoTrack === index)
                }
            }
            onObjectAdded: (index, object) => videoTrackPopup.insertItem(index, object)
            onObjectRemoved: (index, object) => videoTrackPopup.removeItem(object)
        }
    }
    AppMenu {
        id: aspectPopup
        Instantiator {
            model: toolBar.aspectRatios
            delegate: AppMenuItem {
                required property int index
                required property var modelData
                text: modelData.label
                checkable: true
                checked: Math.abs(toolBar.controller.videoAspect - modelData.value) < 0.001
                onTriggered: {
                    toolBar.controller.videoAspect = modelData.value
                    checked = Qt.binding(() => Math.abs(
                        toolBar.controller.videoAspect - modelData.value) < 0.001)
                }
            }
            onObjectAdded: (index, object) => aspectPopup.insertItem(index, object)
            onObjectRemoved: (index, object) => aspectPopup.removeItem(object)
        }
    }
    AppMenu {
        id: rotatePopup
        AppMenuItem {
            text: qsTr("&None")
            checkable: true
            checked: toolBar.controller.videoRotation === 0
            onTriggered: {
                toolBar.controller.videoRotation = 0
                checked = Qt.binding(() => toolBar.controller.videoRotation === 0)
            }
        }
        AppMenuItem {
            text: qsTr("&Rotate by 90° clockwise")
            checkable: true
            checked: toolBar.controller.videoRotation === 90
            onTriggered: {
                toolBar.controller.videoRotation = 90
                checked = Qt.binding(() => toolBar.controller.videoRotation === 90)
            }
        }
        AppMenuItem {
            text: qsTr("Rotate by 90° &counterclockwise")
            checkable: true
            checked: toolBar.controller.videoRotation === 270
            onTriggered: {
                toolBar.controller.videoRotation = 270
                checked = Qt.binding(() => toolBar.controller.videoRotation === 270)
            }
        }
        AppMenuItem {
            text: qsTr("Rotate by &180°")
            checkable: true
            checked: toolBar.controller.videoRotation === 180
            onTriggered: {
                toolBar.controller.videoRotation = 180
                checked = Qt.binding(() => toolBar.controller.videoRotation === 180)
            }
        }
    }
    AppMenu {
        id: videoSizePopup
        AppMenuItem { text: qsTr("50%"); onTriggered: toolBar.resizeToVideoPercentRequested(50) }
        AppMenuItem { text: qsTr("100%"); onTriggered: toolBar.resizeToVideoPercentRequested(100) }
        AppMenuItem { text: qsTr("200%"); onTriggered: toolBar.resizeToVideoPercentRequested(200) }
    }
    AppMenu {
        id: titlesPopup
        Instantiator {
            model: toolBar.controller.dvdPlayback ? toolBar.controller.dvdTitles
                 : toolBar.controller.blurayPlayback ? toolBar.controller.blurayTitles
                 : []
            delegate: AppMenuItem {
                required property int index
                required property var modelData
                text: modelData.label
                checkable: true
                checked: toolBar.controller.dvdPlayback
                         ? modelData.number === toolBar.controller.dvdCurrentTitle
                         : modelData.index === toolBar.controller.blurayCurrentTitle
                onTriggered: toolBar.controller.dvdPlayback
                             ? toolBar.controller.playDvdTitle(modelData.number)
                             : toolBar.controller.playBlurayTitle(modelData.index)
            }
            onObjectAdded: (index, object) => titlesPopup.insertItem(index, object)
            onObjectRemoved: (index, object) => titlesPopup.removeItem(object)
        }
        Instantiator {
            model: (toolBar.controller.dvdTitles.length === 0
                    && toolBar.controller.blurayTitles.length === 0) ? 1 : 0
            delegate: AppMenuItem { text: qsTr("<empty>"); enabled: false }
            onObjectAdded: (index, object) => titlesPopup.insertItem(0, object)
            onObjectRemoved: (index, object) => titlesPopup.removeItem(object)
        }
    }
    AppMenu {
        id: chaptersPopup
        Instantiator {
            model: toolBar.controller.chapters
            delegate: AppMenuItem {
                required property int index
                required property var modelData
                text: modelData.label
                onTriggered: toolBar.controller.playChapter(index)
            }
            onObjectAdded: (index, object) => chaptersPopup.insertItem(index, object)
            onObjectRemoved: (index, object) => chaptersPopup.removeItem(object)
        }
        Instantiator {
            model: toolBar.controller.chapters.length === 0 ? 1 : 0
            delegate: AppMenuItem { text: qsTr("<empty>"); enabled: false }
            onObjectAdded: (index, object) => chaptersPopup.insertItem(0, object)
            onObjectRemoved: (index, object) => chaptersPopup.removeItem(object)
        }
    }
    AppMenu {
        id: bookmarksPopup
        readonly property var entries: {
            toolBar.controller.bookmarks.revision // establish dependency
            return toolBar.controller.bookmarks.entries()
        }
        AppMenuItem {
            text: qsTr("&Add new bookmark")
            enabled: toolBar.player.source.toString() !== ""
            onTriggered: toolBar.addBookmarkRequested()
        }
        AppMenuItem {
            text: qsTr("&Edit bookmarks…")
            enabled: toolBar.player.source.toString() !== ""
            onTriggered: toolBar.editBookmarksRequested()
        }
        MenuSeparator {}
        Instantiator {
            model: bookmarksPopup.entries
            delegate: AppMenuItem {
                required property int index
                required property var modelData
                text: modelData.label
                onTriggered: toolBar.controller.goToBookmark(modelData.time)
            }
            onObjectAdded: (index, object) => bookmarksPopup.insertItem(index + 3, object)
            onObjectRemoved: (index, object) => bookmarksPopup.removeItem(object)
        }
    }
    AppMenu {
        id: recentFilesPopup
        Instantiator {
            model: toolBar.controller.recents.titles
            delegate: AppMenuItem {
                required property int index
                required property string modelData
                text: modelData
                onTriggered: toolBar.controller.open(
                                 [toolBar.controller.recents.urlAt(index)])
            }
            onObjectAdded: (index, object) => recentFilesPopup.insertItem(index, object)
            onObjectRemoved: (index, object) => recentFilesPopup.removeItem(object)
        }
        Instantiator {
            model: toolBar.controller.recents.count === 0 ? 1 : 0
            delegate: AppMenuItem { text: qsTr("(empty)"); enabled: false }
            onObjectAdded: (index, object) => recentFilesPopup.insertItem(0, object)
            onObjectRemoved: (index, object) => recentFilesPopup.removeItem(object)
        }
        MenuSeparator {}
        AppMenuItem {
            text: qsTr("Clear")
            enabled: toolBar.controller.recents.count > 0
            onTriggered: toolBar.controller.recents.clear()
        }
    }
    AppMenu {
        id: zoomPanPopup
        AppMenuItem { text: qsTr("Reset"); onTriggered: toolBar.controller.resetZoomAndPan() }
        AppMenuItem { text: qsTr("Zoom -"); onTriggered: toolBar.controller.zoomOut() }
        AppMenuItem { text: qsTr("Zoom +"); onTriggered: toolBar.controller.zoomIn() }
        MenuSeparator {}
        AppMenuItem { text: qsTr("Move left"); onTriggered: toolBar.controller.panBy(-16, 0) }
        AppMenuItem { text: qsTr("Move right"); onTriggered: toolBar.controller.panBy(16, 0) }
        AppMenuItem { text: qsTr("Move up"); onTriggered: toolBar.controller.panBy(0, -16) }
        AppMenuItem { text: qsTr("Move down"); onTriggered: toolBar.controller.panBy(0, 16) }
    }
    AppMenu {
        id: helpPopup
        AppMenuItem {
            text: qsTr("Contents")
            icon.source: Theme.icon("guide")
            onTriggered: toolBar.helpContentsRequested()
        }
        MenuSeparator {}
        AppMenuItem {
            text: qsTr("Check for updates")
            icon.source: Theme.icon("check_updates")
            onTriggered: toolBar.checkForUpdatesRequested()
        }
        MenuSeparator {}
        AppMenuItem {
            text: qsTr("About Vivace")
            icon.source: Theme.icon("logo")
            onTriggered: toolBar.aboutRequested()
        }
    }

    GridLayout {
        anchors.fill: parent
        columnSpacing: 2
        rowSpacing: 0

        // Repeatable dividers, placed at their layout positions.
        Repeater {
            model: toolBar.positionsOf("separator")
            delegate: ToolSeparator {
                required property int modelData
                Layout.row: toolBar.wrapRow(modelData)
                Layout.column: toolBar.wrapColumn(modelData)
            }
        }
        Repeater {
            model: toolBar.positionsOf("spacer")
            delegate: Item {
                required property int modelData
                Layout.row: toolBar.wrapRow(modelData)
                Layout.column: toolBar.wrapColumn(modelData)
                Layout.columnSpan: toolBar.wrapSpan(modelData)
                Layout.fillWidth: true
            }
        }

        // Predefined action buttons (each shown/positioned by its id).
        TBtn { itemId: "open"; onClicked: toolBar.openFileRequested() }
        TBtn { itemId: "opendvd"; onClicked: toolBar.openDvdRequested() }
        TBtn { itemId: "openfolder"; onClicked: toolBar.openDirectoryRequested() }
        TBtn { itemId: "url"; onClicked: toolBar.openUrlRequested() }
        TBtn {
            itemId: "favorites"; menuIndicator: true
            onClicked: favoritesPopup.popup(this, 0, height)
        }
        TBtn {
            itemId: "screenshot"
            enabled: toolBar.player.hasVideo
            onClicked: toolBar.screenshotRequested()
        }
        TBtn {
            itemId: "info"
            enabled: toolBar.player.source.toString() !== ""
            onClicked: toolBar.infoRequested()
        }
        TBtn {
            itemId: "playlist"
            checkable: true
            checked: toolBar.playlistOpen
            onClicked: toolBar.playlistToggleRequested()
        }
        TBtn { itemId: "preferences"; onClicked: toolBar.preferencesRequested() }
        TBtn {
            itemId: "playpause"
            icon.source: visible ? (toolBar.player.playbackState === MediaPlayer.PlayingState
                                    ? Theme.icon("pause") : Theme.icon("play")) : ""
            enabled: toolBar.player.source.toString() !== ""
                     || toolBar.controller.playlist.count > 0
            onClicked: toolBar.controller.togglePlayPause()
        }
        TBtn {
            itemId: "stop"
            enabled: toolBar.player.playbackState !== MediaPlayer.StoppedState
            onClicked: toolBar.controller.stop()
        }
        TBtn {
            itemId: "previous"
            enabled: toolBar.controller.playlist.count > 0
            onClicked: toolBar.controller.previous()
        }
        TBtn {
            itemId: "next"
            enabled: toolBar.controller.playlist.currentIndex
                     < toolBar.controller.playlist.count - 1
            onClicked: toolBar.controller.next()
        }
        TBtn {
            itemId: "prevchapter"
            enabled: toolBar.controller.dvdChapters.length > 0
            onClicked: toolBar.controller.previousDvdChapter()
        }
        TBtn {
            itemId: "nextchapter"
            enabled: toolBar.controller.dvdChapters.length > 0
            onClicked: toolBar.controller.nextDvdChapter()
        }
        TBtn {
            itemId: "rewindlong"
            enabled: toolBar.player.seekable
            onClicked: toolBar.controller.seekRelative(-Settings.seekLongStep * 1000)
        }
        TBtn {
            itemId: "rewindmed"
            enabled: toolBar.player.seekable
            onClicked: toolBar.controller.seekRelative(-Settings.seekMediumStep * 1000)
        }
        TBtn {
            itemId: "rewindshort"
            enabled: toolBar.player.seekable
            onClicked: toolBar.controller.seekRelative(-Settings.seekShortStep * 1000)
        }
        TBtn {
            itemId: "forwardshort"
            enabled: toolBar.player.seekable
            onClicked: toolBar.controller.seekRelative(Settings.seekShortStep * 1000)
        }
        TBtn {
            itemId: "forwardmed"
            enabled: toolBar.player.seekable
            onClicked: toolBar.controller.seekRelative(Settings.seekMediumStep * 1000)
        }
        TBtn {
            itemId: "forwardlong"
            enabled: toolBar.player.seekable
            onClicked: toolBar.controller.seekRelative(Settings.seekLongStep * 1000)
        }
        TBtn {
            itemId: "fullscreen"
            // Same reasoning as ControlBar.qml's own fullscreen button:
            // fullscreen is forced on Android, so this has nothing left
            // to toggle there.
            visible: toolBar.col(itemId) >= 0 && Qt.platform.os !== "android"
            onClicked: toolBar.fullscreenToggleRequested()
        }
        TBtn {
            itemId: "mute"
            icon.source: visible ? (Settings.muted ? Theme.icon("mute")
                                                    : Theme.icon("volume")) : ""
            checkable: true
            checked: Settings.muted
            onClicked: Settings.muted = !Settings.muted
        }
        TBtn {
            itemId: "audiotrack"; menuIndicator: true
            enabled: toolBar.controller.audioTrackLabels.length > 0
            onClicked: audioMenu.popup(this, 0, height)
        }
        TBtn {
            itemId: "subtitletrack"; menuIndicator: true
            enabled: toolBar.controller.subtitleTrackLabels.length > 0
            onClicked: subtitleMenu.popup(this, 0, height)
        }
        TBtn { itemId: "openbluray"; onClicked: toolBar.openBlurayRequested() }
        TBtn {
            itemId: "tv"; menuIndicator: true
            onClicked: tvPopup.popup(this, 0, height)
        }
        TBtn {
            itemId: "radio"; menuIndicator: true
            onClicked: radioPopup.popup(this, 0, height)
        }
        TBtn {
            itemId: "youtubecache"
            enabled: toolBar.youtubeCacheCount > 0 && Settings.youtubeEnabled
                     && Settings.youtubeMode === 1
            onClicked: toolBar.youtubeCacheRequested()
        }
        TBtn { itemId: "cast"; onClicked: toolBar.castRequested() }
        TBtn {
            itemId: "speed"; menuIndicator: true
            onClicked: speedPopup.popup(this, 0, height)
        }
        TBtn { itemId: "speedhalve"; onClicked: toolBar.adjustSpeed(0.5) }
        TBtn { itemId: "speednormal"; onClicked: Settings.playbackRate = 1 }
        TBtn { itemId: "speeddouble"; onClicked: toolBar.adjustSpeed(2) }
        TBtn { itemId: "speeddec10"; onClicked: toolBar.adjustSpeedStep(-0.1) }
        TBtn { itemId: "speedinc10"; onClicked: toolBar.adjustSpeedStep(0.1) }
        TBtn { itemId: "abmarkera"; onClicked: toolBar.controller.setAMarker() }
        TBtn { itemId: "abmarkerb"; onClicked: toolBar.controller.setBMarker() }
        TBtn { itemId: "abclear"; onClicked: toolBar.controller.clearABMarkers() }
        TBtn {
            itemId: "videotrack"; menuIndicator: true
            enabled: toolBar.controller.videoTrackLabels.length > 0
            onClicked: videoTrackPopup.popup(this, 0, height)
        }
        TBtn { itemId: "equalizer"; onClicked: toolBar.videoEqualizerRequested() }
        TBtn {
            itemId: "aspectratio"; menuIndicator: true
            enabled: toolBar.player.hasVideo
            onClicked: aspectPopup.popup(this, 0, height)
        }
        TBtn {
            itemId: "rotate"; menuIndicator: true
            enabled: toolBar.player.hasVideo
            onClicked: rotatePopup.popup(this, 0, height)
        }
        TBtn {
            itemId: "videosize"; menuIndicator: true
            enabled: toolBar.player.hasVideo
            onClicked: videoSizePopup.popup(this, 0, height)
        }
        TBtn {
            itemId: "flip"
            enabled: toolBar.player.hasVideo
            checkable: true
            checked: toolBar.controller.videoFlip
            onClicked: toolBar.controller.videoFlip = !toolBar.controller.videoFlip
        }
        TBtn {
            itemId: "mirror"
            enabled: toolBar.player.hasVideo
            checkable: true
            checked: toolBar.controller.videoMirror
            onClicked: toolBar.controller.videoMirror = !toolBar.controller.videoMirror
        }
        TBtn {
            itemId: "audiodelaydec"
            onClicked: toolBar.controller.adjustFileAudioDelay(-100)
        }
        TBtn {
            itemId: "audiodelayinc"
            onClicked: toolBar.controller.adjustFileAudioDelay(100)
        }
        TBtn { itemId: "audiodelayset"; onClicked: toolBar.setAudioDelayRequested() }
        TBtn { itemId: "loadsubtitles"; onClicked: toolBar.loadSubtitlesRequested() }
        TBtn { itemId: "findsubtitles"; onClicked: toolBar.findSubtitlesRequested() }
        TBtn { itemId: "unloadsubtitles"; onClicked: toolBar.controller.unloadSubtitles() }
        TBtn {
            itemId: "subtitledelaydec"
            onClicked: toolBar.controller.adjustSubtitleDelay(-100)
        }
        TBtn {
            itemId: "subtitledelayinc"
            onClicked: toolBar.controller.adjustSubtitleDelay(100)
        }
        TBtn { itemId: "subtitledelayset"; onClicked: toolBar.setSubtitleDelayRequested() }
        TBtn {
            itemId: "dvdmenu"
            enabled: toolBar.controller.dvdHasMenu
            onClicked: toolBar.controller.showDvdMenu()
        }
        TBtn {
            itemId: "titles"; menuIndicator: true
            onClicked: titlesPopup.popup(this, 0, height)
        }
        TBtn {
            itemId: "chaptersmenu"; menuIndicator: true
            onClicked: chaptersPopup.popup(this, 0, height)
        }
        TBtn {
            itemId: "bookmarksmenu"; menuIndicator: true
            onClicked: bookmarksPopup.popup(this, 0, height)
        }
        TBtn {
            itemId: "addbookmark"
            enabled: toolBar.player.source.toString() !== ""
            onClicked: toolBar.addBookmarkRequested()
        }
        TBtn {
            itemId: "recentfiles"; menuIndicator: true
            onClicked: recentFilesPopup.popup(this, 0, height)
        }
        TBtn { itemId: "openplaylist"; onClicked: toolBar.openPlaylistRequested() }
        TBtn { itemId: "framestep"; onClicked: toolBar.controller.frameStep(1) }
        TBtn { itemId: "framebackstep"; onClicked: toolBar.controller.frameStep(-1) }
        TBtn {
            itemId: "abrepeat"
            checkable: true
            checked: Settings.playlistRepeat
            onClicked: Settings.playlistRepeat = !Settings.playlistRepeat
        }
        TBtn {
            itemId: "zoompan"; menuIndicator: true
            enabled: toolBar.player.hasVideo
            onClicked: zoomPanPopup.popup(this, 0, height)
        }
        TBtn {
            itemId: "help"; menuIndicator: true
            onClicked: helpPopup.popup(this, 0, height)
        }
    }
}
