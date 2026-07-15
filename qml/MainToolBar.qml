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

    signal openFileRequested()
    signal openUrlRequested()
    signal openDvdRequested()
    signal openDirectoryRequested()
    signal playlistToggleRequested()
    signal screenshotRequested()
    signal infoRequested()
    signal preferencesRequested()
    signal fullscreenToggleRequested()
    signal editFavoritesRequested()

    readonly property QtObject player: controller.player

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
        TBtn { itemId: "fullscreen"; onClicked: toolBar.fullscreenToggleRequested() }
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
    }
}
