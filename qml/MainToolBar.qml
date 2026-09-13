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
    signal videoEqualizerRequested()
    signal resizeToVideoPercentRequested(int percent)
    signal setAudioDelayRequested()
    signal loadSubtitlesRequested()
    signal findSubtitlesRequested()
    signal setSubtitleDelayRequested()
    signal addBookmarkRequested()
    signal editBookmarksRequested()

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

    // A toolbar button whose icon, tooltip, visibility and column all come
    // from its item id; instances add only enabled/checked/onClicked.
    component TBtn: ToolButton {
        id: btn
        property string itemId: ""
        property bool menuIndicator: false

        visible: toolBar.col(itemId) >= 0
        Layout.row: 0
        Layout.column: toolBar.col(itemId)

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
    }
    FavoritesMenu {
        id: radioPopup
        controller: toolBar.controller
        model: toolBar.controller.radioChannels
        itemIcon: Theme.icon("open_radio")
        showActions: true
        onEditRequested: toolBar.editRadioChannelsRequested()
        onAddCurrentRequested: toolBar.controller.addCurrentTo(toolBar.controller.radioChannels)
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

    GridLayout {
        anchors.fill: parent
        rows: 1
        columnSpacing: 2
        rowSpacing: 0

        // Repeatable dividers, placed at their layout positions.
        Repeater {
            model: toolBar.positionsOf("separator")
            delegate: ToolSeparator {
                required property int modelData
                Layout.row: 0
                Layout.column: modelData
            }
        }
        Repeater {
            model: toolBar.positionsOf("spacer")
            delegate: Item {
                required property int modelData
                Layout.row: 0
                Layout.column: modelData
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
    }
}
