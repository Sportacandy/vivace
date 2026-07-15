/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Main menu. The hierarchy mirrors SMPlayer's menu tree (BaseGui::
    populateMainMenu): features that Vivace cannot support without mpv
    (discs, TV, video/audio filters, …) are omitted; features planned for
    later phases keep their place but are disabled. Icons come from
    SMPlayer's default theme (GPL-2.0-or-later, used under GPL-3.0).
*/

import QtQuick
import QtQuick.Controls
import QtMultimedia

MenuBar {
    id: bar

    required property PlayerController controller
    required property bool fullscreen
    required property bool playlistOpen
    // Number of videos in the YouTube download cache (drives the cache-browser
    // menu item's enabled state).
    property int youtubeCacheCount: 0

    signal openFileRequested()
    signal openDirectoryRequested()
    signal openPlaylistRequested()
    signal openDvdRequested()
    signal openUrlRequested()
    signal youtubeCacheRequested()
    signal editTvChannelsRequested()
    signal editRadioChannelsRequested()
    signal editFavoritesRequested()
    signal addBookmarkRequested()
    signal editBookmarksRequested()
    signal loadSubtitlesRequested()
    signal findSubtitlesRequested()
    signal setSubtitleDelayRequested()
    signal setAudioDelayRequested()
    signal videoEqualizerRequested()
    signal resizeToVideoPercentRequested(int percent)
    signal fullscreenToggleRequested()
    signal playlistToggleRequested()
    signal screenshotRequested()
    signal infoRequested()
    signal preferencesRequested()
    signal editMainToolbarRequested()
    signal editControlBarRequested()
    signal aboutRequested()
    signal checkForUpdatesRequested()
    signal installYoutubeSupportRequested()
    signal helpContentsRequested()
    signal castRequested()

    // QtObject, not MediaPlayer: the QML MediaPlayer type (QQuickMediaPlayer)
    // is not assignable from the C++ QMediaPlayer the controller owns.
    readonly property QtObject player: controller.player
    readonly property bool hasMedia: player.source.toString() !== ""

    // Forced display aspect ratios (SMPlayer's Video > Aspect ratio); 0 = auto.
    readonly property var aspectRatios: [
        { label: qsTr("&Auto"), value: 0 },
        { label: "1:1", value: 1 / 1 },
        { label: "5:4", value: 5 / 4 },
        { label: "&4:3", value: 4 / 3 },
        { label: "11:8", value: 11 / 8 },
        { label: "14:10", value: 14 / 10 },
        { label: "3:2", value: 3 / 2 },
        { label: "14:9", value: 14 / 9 },
        { label: "16:10", value: 16 / 10 },
        { label: "1&6:9", value: 16 / 9 },
        { label: "&2.35:1", value: 2.35 }
    ]

    function adjustSpeed(factor) {
        Settings.playbackRate =
                Math.max(0.1, Math.round(Settings.playbackRate * factor * 100) / 100)
    }

    function seekStepText(seconds) {
        return seconds % 60 === 0 && seconds >= 60
               ? qsTr("%n minute(s)", "", seconds / 60)
               : qsTr("%n second(s)", "", seconds)
    }

    // Render the mnemonic underline on the top-level titles too; hover uses
    // the same Windows-like light blue fill with dark text as AppMenuItem.
    delegate: MenuBarItem {
        id: barItem
        background: Rectangle {
            radius: 4
            color: barItem.highlighted ? "#cce8ff" : "transparent"
        }
        contentItem: Label {
            text: UiHelpers.mnemonicLabel(barItem.text)
            textFormat: Text.StyledText
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: "#1a1a1a"
        }
    }

    // ------------------------------------------------------------- Open
    AppMenu {
        title: qsTr("&Open")

        Action {
            text: qsTr("&File…")
            icon.source: Theme.icon("open")
            shortcut: Shortcuts.sequences["open_file"]
            onTriggered: bar.openFileRequested()
        }
        AppMenu {
            id: recentMenu
            title: qsTr("&Recent files")
            icon.source: Theme.icon("recents")

            Instantiator {
                model: bar.controller.recents.titles
                delegate: AppMenuItem {
                    required property int index
                    required property string modelData
                    text: modelData
                    onTriggered: bar.controller.open(
                                     [bar.controller.recents.urlAt(index)])
                }
                onObjectAdded: (index, object) => recentMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => recentMenu.removeItem(object)
            }
            AppMenuItem {
                text: qsTr("(empty)")
                enabled: false
                visible: bar.controller.recents.count === 0
                height: visible ? implicitHeight : 0
            }
            MenuSeparator {}
            AppMenuItem {
                text: qsTr("&Clear")
                enabled: bar.controller.recents.count > 0
                onTriggered: bar.controller.recents.clear()
            }
        }
        FavoritesMenu {
            title: qsTr("F&avorites")
            icon.source: Theme.icon("open_favorites")
            controller: bar.controller
            model: bar.controller.favorites
            showActions: true
            onEditRequested: bar.editFavoritesRequested()
            onAddCurrentRequested: bar.controller.addCurrentTo(bar.controller.favorites)
        }
        Action {
            text: qsTr("&Directory…")
            icon.source: Theme.icon("openfolder")
            onTriggered: bar.openDirectoryRequested()
        }
        Action {
            text: qsTr("&Playlist…")
            icon.source: Theme.icon("open_playlist")
            onTriggered: bar.openPlaylistRequested()
        }
        AppMenu {
            title: qsTr("D&isc")
            icon.source: Theme.icon("open_disc")

            Action {
                text: qsTr("&DVD from drive or folder…")
                icon.source: Theme.icon("dvd")
                onTriggered: bar.openDvdRequested()
            }
            AppMenuItem {
                text: qsTr("&Blu-ray")
                enabled: false
            }
            AppMenuItem {
                text: qsTr("&Audio CD")
                enabled: false
            }
        }
        Action {
            text: qsTr("&URL…")
            icon.source: Theme.icon("url")
            shortcut: Shortcuts.sequences["open_url"]
            onTriggered: bar.openUrlRequested()
        }
        AppMenuItem {
            text: qsTr("YouTube &cache…")
            icon.source: Theme.icon("url")
            // Only when there is something to browse and download mode is on.
            enabled: bar.youtubeCacheCount > 0 && Settings.youtubeEnabled
                     && Settings.youtubeMode === 1
            onTriggered: bar.youtubeCacheRequested()
        }
        FavoritesMenu {
            title: qsTr("&TV")
            icon.source: Theme.icon("open_tv")
            controller: bar.controller
            model: bar.controller.tvChannels
            itemIcon: Theme.icon("open_tv")
            showActions: true
            onEditRequested: bar.editTvChannelsRequested()
            onAddCurrentRequested: bar.controller.addCurrentTo(bar.controller.tvChannels)
        }
        FavoritesMenu {
            title: qsTr("Radi&o")
            icon.source: Theme.icon("open_radio")
            controller: bar.controller
            model: bar.controller.radioChannels
            itemIcon: Theme.icon("open_radio")
            showActions: true
            onEditRequested: bar.editRadioChannelsRequested()
            onAddCurrentRequested: bar.controller.addCurrentTo(bar.controller.radioChannels)
        }
        MenuSeparator {}
        Action {
            text: qsTr("&Quit")
            icon.source: Theme.icon("exit")
            shortcut: Shortcuts.sequences["quit"]
            // close() (not Qt.quit()) so the window can save its geometry.
            onTriggered: bar.Window.window.close()
        }
    }

    // ------------------------------------------------------------- Play
    AppMenu {
        title: qsTr("&Play")

        Action {
            text: qsTr("P&lay")
            icon.source: Theme.icon("play")
            enabled: bar.player.playbackState !== MediaPlayer.PlayingState
                     && (bar.hasMedia || bar.controller.playlist.count > 0)
            onTriggered: bar.controller.togglePlayPause()
        }
        Action {
            text: qsTr("&Pause")
            icon.source: Theme.icon("pause")
            enabled: bar.player.playbackState === MediaPlayer.PlayingState
            onTriggered: bar.player.pause()
        }
        Action {
            text: qsTr("&Stop")
            icon.source: Theme.icon("stop")
            enabled: bar.player.playbackState !== MediaPlayer.StoppedState
            onTriggered: bar.controller.stop()
        }
        Action {
            text: qsTr("Frame &step")
            shortcut: Shortcuts.sequences["frame_step"]
            enabled: bar.player.seekable && bar.player.hasVideo
            onTriggered: bar.controller.frameStep(1)
        }
        Action {
            text: qsTr("Fra&me back step")
            shortcut: Shortcuts.sequences["frame_back_step"]
            enabled: bar.player.seekable && bar.player.hasVideo
            onTriggered: bar.controller.frameStep(-1)
        }
        MenuSeparator {}
        Action {
            text: qsTr("&Rewind %1").arg(bar.seekStepText(Settings.seekShortStep))
            icon.source: Theme.icon("rewind10s")
            shortcut: Shortcuts.sequences["rewind_short"]
            enabled: bar.player.seekable
            onTriggered: bar.controller.seekRelative(-Settings.seekShortStep * 1000)
        }
        Action {
            text: qsTr("&Forward %1").arg(bar.seekStepText(Settings.seekShortStep))
            icon.source: Theme.icon("forward10s")
            shortcut: Shortcuts.sequences["forward_short"]
            enabled: bar.player.seekable
            onTriggered: bar.controller.seekRelative(Settings.seekShortStep * 1000)
        }
        Action {
            text: qsTr("Rewind %1").arg(bar.seekStepText(Settings.seekMediumStep))
            icon.source: Theme.icon("rewind1m")
            shortcut: Shortcuts.sequences["rewind_medium"]
            enabled: bar.player.seekable
            onTriggered: bar.controller.seekRelative(-Settings.seekMediumStep * 1000)
        }
        Action {
            text: qsTr("Forward %1").arg(bar.seekStepText(Settings.seekMediumStep))
            icon.source: Theme.icon("forward1m")
            shortcut: Shortcuts.sequences["forward_medium"]
            enabled: bar.player.seekable
            onTriggered: bar.controller.seekRelative(Settings.seekMediumStep * 1000)
        }
        Action {
            text: qsTr("Rewind %1").arg(bar.seekStepText(Settings.seekLongStep))
            icon.source: Theme.icon("rewind10m")
            shortcut: Shortcuts.sequences["rewind_long"]
            enabled: bar.player.seekable
            onTriggered: bar.controller.seekRelative(-Settings.seekLongStep * 1000)
        }
        Action {
            text: qsTr("Forward %1").arg(bar.seekStepText(Settings.seekLongStep))
            icon.source: Theme.icon("forward10m")
            shortcut: Shortcuts.sequences["forward_long"]
            enabled: bar.player.seekable
            onTriggered: bar.controller.seekRelative(Settings.seekLongStep * 1000)
        }
        MenuSeparator {}
        AppMenu {
            title: qsTr("Sp&eed")
            icon.source: Theme.icon("speed")

            Action {
                text: qsTr("&Normal speed")
                shortcut: Shortcuts.sequences["speed_normal"]
                onTriggered: Settings.playbackRate = 1
            }
            MenuSeparator {}
            Action {
                text: qsTr("&Halve speed")
                shortcut: Shortcuts.sequences["speed_halve"]
                onTriggered: bar.adjustSpeed(0.5)
            }
            Action {
                text: qsTr("&Double speed")
                shortcut: Shortcuts.sequences["speed_double"]
                onTriggered: bar.adjustSpeed(2)
            }
            MenuSeparator {}
            Action {
                text: qsTr("Speed &-10%")
                shortcut: Shortcuts.sequences["speed_dec"]
                onTriggered: bar.adjustSpeed(0.9)
            }
            Action {
                text: qsTr("Speed &+10%")
                shortcut: Shortcuts.sequences["speed_inc"]
                onTriggered: bar.adjustSpeed(1.1)
            }
            MenuSeparator {}
            Action { text: qsTr("Speed -4%"); onTriggered: bar.adjustSpeed(0.96) }
            Action { text: qsTr("Speed +4%"); onTriggered: bar.adjustSpeed(1.04) }
            MenuSeparator {}
            Action { text: qsTr("Speed -1%"); onTriggered: bar.adjustSpeed(0.99) }
            Action { text: qsTr("Speed +1%"); onTriggered: bar.adjustSpeed(1.01) }
            MenuSeparator {}
            Action {
                text: qsTr("Pi&tch compensation")
                checkable: true
                checked: Settings.pitchCompensation
                enabled: bar.player.pitchCompensationAvailability === MediaPlayer.Available
                onTriggered: Settings.pitchCompensation = !Settings.pitchCompensation
            }
        }
        MenuSeparator {}
        AppMenu {
            title: qsTr("&A-B section")
            icon.source: Theme.icon("a_marker")

            Action {
                text: qsTr("Set &A marker")
                icon.source: Theme.icon("a_marker")
                enabled: bar.hasMedia
                onTriggered: bar.controller.setAMarker()
            }
            Action {
                text: qsTr("Set &B marker")
                icon.source: Theme.icon("b_marker")
                enabled: bar.hasMedia
                onTriggered: bar.controller.setBMarker()
            }
            Action {
                text: qsTr("&Clear A-B markers")
                icon.source: Theme.icon("delete")
                enabled: bar.controller.abMarkerA >= 0
                         || bar.controller.abMarkerB >= 0
                onTriggered: bar.controller.clearABMarkers()
            }
            MenuSeparator {}
            Action {
                text: qsTr("&Repeat")
                checkable: true
                checked: Settings.playlistRepeat
                onTriggered: Settings.playlistRepeat = checked
            }
        }
        AppMenuItem { text: qsTr("&Jump to…"); enabled: false }
        MenuSeparator {}
        Action {
            text: qsTr("Pre&vious")
            icon.source: Theme.icon("previous")
            shortcut: Shortcuts.sequences["previous"]
            enabled: bar.controller.playlist.count > 0
            onTriggered: bar.controller.previous()
        }
        Action {
            text: qsTr("&Next")
            icon.source: Theme.icon("next")
            shortcut: Shortcuts.sequences["next"]
            enabled: bar.controller.playlist.currentIndex
                     < bar.controller.playlist.count - 1
            onTriggered: bar.controller.next()
        }
        MenuSeparator {}
        AppMenu {
            title: qsTr("Ca&st")
            icon.source: Theme.icon("cast")

            Action {
                text: qsTr("&Smartphone/tablet…")
                onTriggered: bar.castRequested()
            }
        }
    }

    // ------------------------------------------------------------ Video
    AppMenu {
        title: qsTr("&Video")

        AppMenu {
            id: videoTrackMenu
            title: qsTr("&Track")
            icon.source: Theme.icon("video_track")

            AppMenuItem {
                text: qsTr("<empty>")
                enabled: false
                visible: bar.controller.videoTrackLabels.length === 0
                height: visible ? implicitHeight : 0
            }
            Instantiator {
                model: bar.controller.videoTrackLabels
                delegate: AppMenuItem {
                    required property int index
                    required property string modelData
                    text: modelData
                    checkable: true
                    checked: bar.controller.activeVideoTrack === index
                    onTriggered: {
                        bar.controller.activeVideoTrack = index
                        checked = Qt.binding(() => bar.controller.activeVideoTrack === index)
                    }
                }
                onObjectAdded: (index, object) => videoTrackMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => videoTrackMenu.removeItem(object)
            }
        }
        Action {
            text: qsTr("&Fullscreen")
            icon.source: Theme.icon("fullscreen")
            shortcut: Shortcuts.sequences["fullscreen"]
            checkable: true
            checked: bar.fullscreen
            onTriggered: bar.fullscreenToggleRequested()
        }
        AppMenu {
            title: qsTr("Si&ze")
            icon.source: Theme.icon("video_size")
            enabled: bar.player.hasVideo
            AppMenuItem { text: qsTr("50%"); onTriggered: bar.resizeToVideoPercentRequested(50) }
            AppMenuItem { text: qsTr("100%"); onTriggered: bar.resizeToVideoPercentRequested(100) }
            AppMenuItem { text: qsTr("200%"); onTriggered: bar.resizeToVideoPercentRequested(200) }
        }
        AppMenu {
            title: qsTr("&Zoom and pan")
            enabled: bar.player.hasVideo
            // Shortcut hints need an Action (AppMenuItem has no shortcut).
            Action {
                text: qsTr("&Reset")
                shortcut: Shortcuts.sequences["zoom_reset"]
                onTriggered: bar.controller.resetZoomAndPan()
            }
            Action {
                text: qsTr("Zoom &-")
                shortcut: Shortcuts.sequences["zoom_out"]
                onTriggered: bar.controller.zoomOut()
            }
            Action {
                text: qsTr("Zoom &+")
                shortcut: Shortcuts.sequences["zoom_in"]
                onTriggered: bar.controller.zoomIn()
            }
            MenuSeparator {}
            Action {
                text: qsTr("Move &left")
                shortcut: Shortcuts.sequences["pan_left"]
                onTriggered: bar.controller.panBy(-16, 0)
            }
            Action {
                text: qsTr("Move &right")
                shortcut: Shortcuts.sequences["pan_right"]
                onTriggered: bar.controller.panBy(16, 0)
            }
            Action {
                text: qsTr("Move &up")
                shortcut: Shortcuts.sequences["pan_up"]
                onTriggered: bar.controller.panBy(0, -16)
            }
            Action {
                text: qsTr("Move &down")
                shortcut: Shortcuts.sequences["pan_down"]
                onTriggered: bar.controller.panBy(0, 16)
            }
        }
        AppMenu {
            id: aspectMenu
            title: qsTr("&Aspect ratio")
            icon.source: Theme.icon("aspect")
            enabled: bar.player.hasVideo
            Instantiator {
                model: bar.aspectRatios
                delegate: AppMenuItem {
                    required property int index
                    required property var modelData
                    text: modelData.label
                    checkable: true
                    checked: Math.abs(bar.controller.videoAspect - modelData.value) < 0.001
                    onTriggered: {
                        bar.controller.videoAspect = modelData.value
                        checked = Qt.binding(() => Math.abs(
                            bar.controller.videoAspect - modelData.value) < 0.001)
                    }
                }
                onObjectAdded: (index, object) => aspectMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => aspectMenu.removeItem(object)
            }
        }
        AppMenu {
            title: qsTr("&Rotate")
            icon.source: Theme.icon("rotate")
            enabled: bar.player.hasVideo
            AppMenuItem {
                text: qsTr("&None")
                checkable: true
                checked: bar.controller.videoRotation === 0
                onTriggered: {
                    bar.controller.videoRotation = 0
                    checked = Qt.binding(() => bar.controller.videoRotation === 0)
                }
            }
            AppMenuItem {
                text: qsTr("&Rotate by 90° clockwise")
                checkable: true
                checked: bar.controller.videoRotation === 90
                onTriggered: {
                    bar.controller.videoRotation = 90
                    checked = Qt.binding(() => bar.controller.videoRotation === 90)
                }
            }
            AppMenuItem {
                text: qsTr("Rotate by 90° &counterclockwise")
                checkable: true
                checked: bar.controller.videoRotation === 270
                onTriggered: {
                    bar.controller.videoRotation = 270
                    checked = Qt.binding(() => bar.controller.videoRotation === 270)
                }
            }
            AppMenuItem {
                text: qsTr("Rotate by &180°")
                checkable: true
                checked: bar.controller.videoRotation === 180
                onTriggered: {
                    bar.controller.videoRotation = 180
                    checked = Qt.binding(() => bar.controller.videoRotation === 180)
                }
            }
        }
        AppMenuItem {
            text: qsTr("Fli&p image")
            icon.source: Theme.icon("flip")
            enabled: bar.player.hasVideo
            checkable: true
            checked: bar.controller.videoFlip
            onTriggered: {
                bar.controller.videoFlip = checked
                checked = Qt.binding(() => bar.controller.videoFlip)
            }
        }
        AppMenuItem {
            text: qsTr("Mirr&or image")
            icon.source: Theme.icon("mirror")
            enabled: bar.player.hasVideo
            checkable: true
            checked: bar.controller.videoMirror
            onTriggered: {
                bar.controller.videoMirror = checked
                checked = Qt.binding(() => bar.controller.videoMirror)
            }
        }
        MenuSeparator {}
        Action {
            text: qsTr("E&qualizer…")
            icon.source: Theme.icon("equalizer")
            shortcut: Shortcuts.sequences["equalizer"]
            onTriggered: bar.videoEqualizerRequested()
        }
        Action {
            text: qsTr("&Screenshot")
            icon.source: Theme.icon("screenshot")
            shortcut: Shortcuts.sequences["screenshot"]
            enabled: bar.player.hasVideo
            onTriggered: bar.screenshotRequested()
        }
    }

    // ------------------------------------------------------------ Audio
    AppMenu {
        title: qsTr("&Audio")

        AppMenu {
            id: audioTrackMenu
            title: qsTr("&Track")
            icon.source: Theme.icon("audio_track")

            AppMenuItem {
                text: qsTr("<empty>")
                enabled: false
                visible: bar.controller.audioTrackLabels.length === 0
                height: visible ? implicitHeight : 0
            }
            Instantiator {
                model: bar.controller.audioTrackLabels
                delegate: AppMenuItem {
                    required property int index
                    required property string modelData
                    text: modelData
                    checkable: true
                    checked: bar.controller.activeAudioTrack === index
                    onTriggered: {
                        bar.controller.activeAudioTrack = index
                        checked = Qt.binding(() => bar.controller.activeAudioTrack === index)
                    }
                }
                onObjectAdded: (index, object) => audioTrackMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => audioTrackMenu.removeItem(object)
            }
        }
        MenuSeparator {}
        Action {
            text: qsTr("&Mute")
            icon.source: Theme.icon("mute")
            shortcut: Shortcuts.sequences["mute"]
            checkable: true
            checked: Settings.muted
            onTriggered: Settings.muted = !Settings.muted
        }
        MenuSeparator {}
        Action {
            text: qsTr("Volume &-")
            shortcut: Shortcuts.sequences["volume_dec"]
            onTriggered: Settings.volume =
                             Math.max(0, Settings.volume - Settings.volumeStep / 100)
        }
        Action {
            text: qsTr("Volume &+")
            icon.source: Theme.icon("volume")
            shortcut: Shortcuts.sequences["volume_inc"]
            onTriggered: Settings.volume =
                             Math.min(1, Settings.volume + Settings.volumeStep / 100)
        }
        MenuSeparator {}
        Action {
            text: qsTr("Delay &-")
            onTriggered: bar.controller.adjustFileAudioDelay(-100)
        }
        Action {
            text: qsTr("D&elay +")
            onTriggered: bar.controller.adjustFileAudioDelay(100)
        }
        Action {
            text: qsTr("Set dela&y…")
            onTriggered: bar.setAudioDelayRequested()
        }
    }

    // -------------------------------------------------------- Subtitles
    AppMenu {
        title: qsTr("&Subtitles")

        AppMenu {
            id: subtitleTrackMenu
            title: qsTr("&Track")
            icon.source: Theme.icon("sub")

            AppMenuItem {
                text: qsTr("&Off")
                checkable: true
                checked: bar.controller.activeSubtitleTrack === -1
                enabled: bar.controller.subtitleTrackLabels.length > 0
                // Re-establish the binding the checkable toggle breaks, so the
                // items stay mutually exclusive (now that the controller emits
                // activeTracksChanged, the re-bound expression re-evaluates).
                onTriggered: {
                    bar.controller.activeSubtitleTrack = -1
                    checked = Qt.binding(() => bar.controller.activeSubtitleTrack === -1)
                }
            }
            Instantiator {
                model: bar.controller.subtitleTrackLabels
                delegate: AppMenuItem {
                    required property int index
                    required property string modelData
                    text: modelData
                    checkable: true
                    checked: bar.controller.activeSubtitleTrack === index
                    onTriggered: {
                        bar.controller.activeSubtitleTrack = index
                        checked = Qt.binding(() => bar.controller.activeSubtitleTrack === index)
                    }
                }
                onObjectAdded: (index, object) => subtitleTrackMenu.insertItem(index + 1, object)
                onObjectRemoved: (index, object) => subtitleTrackMenu.removeItem(object)
            }
        }
        MenuSeparator {}
        Action {
            text: qsTr("&Load subtitles…")
            icon.source: Theme.icon("sub")
            enabled: bar.hasMedia
            onTriggered: bar.loadSubtitlesRequested()
        }
        Action {
            text: qsTr("&Find subtitles at OpenSubtitles…")
            enabled: bar.hasMedia
            onTriggered: bar.findSubtitlesRequested()
        }
        Action {
            text: qsTr("U&nload subtitles")
            enabled: bar.controller.hasExternalSubtitles
            onTriggered: bar.controller.unloadSubtitles()
        }
        MenuSeparator {}
        Action {
            text: qsTr("Delay &-")
            enabled: bar.controller.hasExternalSubtitles
            onTriggered: bar.controller.adjustSubtitleDelay(-100)
        }
        Action {
            text: qsTr("Delay &+")
            enabled: bar.controller.hasExternalSubtitles
            onTriggered: bar.controller.adjustSubtitleDelay(100)
        }
        Action {
            text: qsTr("Se&t delay…")
            enabled: bar.controller.hasExternalSubtitles
            onTriggered: bar.setSubtitleDelayRequested()
        }
    }

    // ----------------------------------------------------------- Browse
    AppMenu {
        title: qsTr("&Browse")

        Action {
            text: qsTr("&DVD menu")
            icon.source: Theme.icon("dvd")
            enabled: bar.controller.dvdHasMenu
            onTriggered: bar.controller.showDvdMenu()
        }
        MenuSeparator {}

        AppMenu {
            id: titlesMenu
            title: qsTr("&Title")
            icon.source: Theme.icon("title")

            Instantiator {
                model: bar.controller.dvdTitles
                delegate: AppMenuItem {
                    required property int index
                    required property var modelData
                    text: modelData.label
                    checkable: true
                    checked: modelData.number === bar.controller.dvdCurrentTitle
                    onTriggered: bar.controller.playDvdTitle(modelData.number)
                }
                onObjectAdded: (index, object) => titlesMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => titlesMenu.removeItem(object)
            }
            AppMenuItem {
                text: qsTr("<empty>")
                enabled: false
                visible: bar.controller.dvdTitles.length === 0
                height: visible ? implicitHeight : 0
            }
        }
        AppMenu {
            id: chaptersMenu
            title: qsTr("&Chapters")
            icon.source: Theme.icon("chapter")

            Instantiator {
                model: bar.controller.chapters
                delegate: AppMenuItem {
                    required property int index
                    required property var modelData
                    text: modelData.label
                    onTriggered: bar.controller.playChapter(index)
                }
                onObjectAdded: (index, object) => chaptersMenu.insertItem(index, object)
                onObjectRemoved: (index, object) => chaptersMenu.removeItem(object)
            }
            AppMenuItem {
                text: qsTr("<empty>")
                enabled: false
                visible: bar.controller.chapters.length === 0
                height: visible ? implicitHeight : 0
            }
        }
        AppMenu {
            id: bookmarksMenu
            title: qsTr("&Bookmarks")
            icon.source: Theme.icon("bookmarks")

            // Re-read the current file's bookmarks whenever they change.
            readonly property var entries: {
                bar.controller.bookmarks.revision // establish dependency
                return bar.controller.bookmarks.entries()
            }

            Action {
                text: qsTr("&Add new bookmark")
                icon.source: Theme.icon("add_bookmark")
                shortcut: Shortcuts.sequences["add_bookmark"]
                enabled: bar.hasMedia
                onTriggered: bar.addBookmarkRequested()
            }
            Action {
                text: qsTr("&Edit bookmarks…")
                icon.source: Theme.icon("bookmarks")
                enabled: bar.hasMedia
                onTriggered: bar.editBookmarksRequested()
            }
            MenuSeparator {}

            Instantiator {
                model: bookmarksMenu.entries
                delegate: AppMenuItem {
                    required property int index
                    required property var modelData
                    text: modelData.label
                    onTriggered: bar.controller.goToBookmark(modelData.time)
                }
                onObjectAdded: (index, object) =>
                    bookmarksMenu.insertItem(index + 3, object)
                onObjectRemoved: (index, object) => bookmarksMenu.removeItem(object)
            }
        }
    }

    // ------------------------------------------------------------- View
    AppMenu {
        title: qsTr("Vie&w")

        Action {
            text: qsTr("&Information and properties…")
            icon.source: Theme.icon("info")
            shortcut: Shortcuts.sequences["media_info"]
            enabled: bar.hasMedia
            onTriggered: bar.infoRequested()
        }
        Action {
            text: qsTr("&Playlist")
            icon.source: Theme.icon("playlist")
            shortcut: Shortcuts.sequences["playlist_toggle"]
            checkable: true
            checked: bar.playlistOpen
            onTriggered: bar.playlistToggleRequested()
        }
        AppMenu {
            title: qsTr("&OSD")
            icon.source: Theme.icon("osd")
            AppMenuItem { text: qsTr("Subtitles only"); enabled: false }
            AppMenuItem { text: qsTr("Volume + seek"); enabled: false }
            AppMenuItem { text: qsTr("Volume + seek + timer"); enabled: false }
        }
    }

    // ---------------------------------------------------------- Options
    AppMenu {
        title: qsTr("Op&tions")

        Action {
            text: qsTr("&Preferences…")
            icon.source: Theme.icon("prefs")
            shortcut: Shortcuts.sequences["preferences"]
            onTriggered: bar.preferencesRequested()
        }
        MenuSeparator {}
        Action {
            text: qsTr("S&how icon in system tray")
            checkable: true
            checked: Settings.showTrayIcon
            onTriggered: Settings.showTrayIcon = !Settings.showTrayIcon
        }
        AppMenu {
            title: qsTr("&Toolbars")

            Action {
                text: qsTr("&Toolbar")
                checkable: true
                checked: Settings.showToolbar
                onTriggered: Settings.showToolbar = !Settings.showToolbar
            }
            Action {
                text: qsTr("&Control bar")
                checkable: true
                checked: Settings.showControlBar
                onTriggered: Settings.showControlBar = !Settings.showControlBar
            }
            MenuSeparator {}
            Action {
                text: qsTr("Edit main &toolbar…")
                onTriggered: bar.editMainToolbarRequested()
            }
            Action {
                text: qsTr("Edit control &bar…")
                onTriggered: bar.editControlBarRequested()
            }
        }
        AppMenu {
            title: qsTr("&Status bar")

            Action {
                text: qsTr("&Show status bar")
                checkable: true
                checked: Settings.showStatusBar
                onTriggered: Settings.showStatusBar = !Settings.showStatusBar
            }
            MenuSeparator {}
            Action {
                text: qsTr("&Video info")
                checkable: true
                checked: Settings.statusVideoInfo
                enabled: Settings.showStatusBar
                onTriggered: Settings.statusVideoInfo = !Settings.statusVideoInfo
            }
            Action {
                text: qsTr("&Audio info")
                checkable: true
                checked: Settings.statusAudioInfo
                enabled: Settings.showStatusBar
                onTriggered: Settings.statusAudioInfo = !Settings.statusAudioInfo
            }
            Action {
                text: qsTr("F&ormat info")
                checkable: true
                checked: Settings.statusFormatInfo
                enabled: Settings.showStatusBar
                onTriggered: Settings.statusFormatInfo = !Settings.statusFormatInfo
            }
            Action {
                text: qsTr("&Bitrate info")
                checkable: true
                checked: Settings.statusBitrateInfo
                enabled: Settings.showStatusBar
                onTriggered: Settings.statusBitrateInfo = !Settings.statusBitrateInfo
            }
            Action {
                text: qsTr("&Frame counter")
                checkable: true
                checked: Settings.statusFrameCounter
                enabled: Settings.showStatusBar
                onTriggered: Settings.statusFrameCounter = !Settings.statusFrameCounter
            }
            MenuSeparator {}
            Action {
                text: qsTr("Display &total time")
                checkable: true
                checked: !Settings.timeDisplayRemaining
                onTriggered: Settings.timeDisplayRemaining = false
            }
            Action {
                text: qsTr("Display &remaining time")
                checkable: true
                checked: Settings.timeDisplayRemaining
                onTriggered: Settings.timeDisplayRemaining = true
            }
            MenuSeparator {}
            Action {
                text: qsTr("Show the current time with &milliseconds")
                checkable: true
                checked: Settings.showMilliseconds
                onTriggered: Settings.showMilliseconds = !Settings.showMilliseconds
            }
        }
    }

    // ------------------------------------------------------------- Help
    AppMenu {
        title: qsTr("&Help")

        Action {
            text: qsTr("&Contents")
            icon.source: Theme.icon("guide")
            shortcut: "F1"
            onTriggered: bar.helpContentsRequested()
        }
        MenuSeparator {}
        AppMenuItem {
            text: qsTr("&Check for updates")
            icon.source: Theme.icon("check_updates")
            onTriggered: bar.checkForUpdatesRequested()
        }
        AppMenuItem {
            text: qsTr("Install / Update &YouTube support")
            icon.source: Theme.icon("update_youtube")
            onTriggered: bar.installYoutubeSupportRequested()
        }
        MenuSeparator {}
        Action {
            text: qsTr("&About Vivace")
            icon.source: Theme.icon("logo")
            onTriggered: bar.aboutRequested()
        }
    }
}
