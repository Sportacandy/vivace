/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Main window: video surface, menu bar, control bar and playlist drawer.
    Playback logic lives in PlayerController (C++); persistent options in
    the Settings singleton (C++). The UI writes user changes to Settings,
    and Binding elements below push Settings into the player, so options
    survive restarts.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtMultimedia
import Qt.labs.platform as Platform
import "ToolbarItems.js" as ToolbarItems

// (FolderDialog also comes from QtQuick.Dialogs)

ApplicationWindow {
    id: root

    // Command-line startup options from C++ (main.cpp parseVivaceArgs):
    // { source, subtitle, startMs, fullscreen, ontop, closeAtEnd,
    //   hasPos, x, y, hasSize, width, height }.
    property var startupOptions: ({})
    // Consumed on the first LoadedMedia after a CLI/second-instance open.
    property url pendingSubtitle: ""
    property int pendingStartMs: -1
    // Guards the per-file eq/speed reset below against re-firing on a seek:
    // QFFmpegMediaPlayer::setPosition() (Qt's FFmpeg backend) unconditionally
    // re-emits mediaStatusChanged(LoadedMedia) on every seek, not just on a
    // genuine new-file load, so that handler must not key off mediaStatus
    // alone.
    property url perFileResetSource: ""
    // --close-at-end: close when playback finishes (session override).
    property bool cliCloseAtEnd: false

    // Latest line of the external downloader's output, for the busy overlay.
    property string downloadStatus: ""

    width: 1280
    height: 760
    visible: true
    color: "black"
    title: playerController.mediaTitle !== ""
           ? playerController.mediaTitle + " — Vivace"
           : (playerController.player.source.toString() !== ""
              ? basename(playerController.player.source) + " — Vivace" : "Vivace")

    readonly property bool fullscreen: visibility === Window.FullScreen

    function basename(url) {
        const s = url.toString()
        return decodeURIComponent(s.substring(s.lastIndexOf("/") + 1))
    }

    // mm:ss or h:mm:ss for OSD messages (e.g. the gesture-seek preview).
    function formatClock(ms) {
        const total = Math.max(0, Math.floor(ms / 1000))
        const h = Math.floor(total / 3600)
        const m = Math.floor((total % 3600) / 60)
        const s = total % 60
        const pad = n => (n < 10 ? "0" + n : "" + n)
        return (h > 0 ? h + ":" + pad(m) : m) + ":" + pad(s)
    }

    function toggleFullscreen() {
        visibility = fullscreen ? Window.Windowed : Window.FullScreen
    }

    // Center on screen and/or keep the window inside the available area
    // (SMPlayer's "Center window" / "Prevent window from getting outside").
    function applyWindowPlacement() {
        if (visibility !== Window.Windowed)
            return
        if (Settings.centerWindow) {
            root.x = Screen.virtualX + (Screen.width - root.width) / 2
            root.y = Screen.virtualY + (Screen.height - root.height) / 2
        }
        if (Settings.preventOutsideScreen) {
            const maxX = Screen.virtualX + Screen.desktopAvailableWidth - root.width
            const maxY = Screen.virtualY + Screen.desktopAvailableHeight - root.height
            root.x = Math.max(Screen.virtualX, Math.min(root.x, maxX))
            root.y = Math.max(Screen.virtualY, Math.min(root.y, maxY))
        }
    }

    // Apply the command-line startup options (from the primary launch or a
    // forwarded second instance): window geometry/state, then the media, with
    // the subtitle and start time deferred to the first LoadedMedia.
    function applyStartupOptions(opts) {
        if (!opts)
            return
        if (opts.hasPos) { root.x = opts.x; root.y = opts.y }
        if (opts.hasSize) { root.width = opts.width; root.height = opts.height }
        if (opts.ontop)
            root.flags = root.flags | Qt.WindowStaysOnTopHint
        if (opts.closeAtEnd)
            root.cliCloseAtEnd = true
        if (opts.subtitle !== undefined && opts.subtitle !== "")
            root.pendingSubtitle = opts.subtitle
        root.pendingStartMs = (opts.startMs !== undefined && opts.startMs >= 0)
                              ? opts.startMs : -1
        if (opts.source !== undefined && opts.source !== "")
            root.openMediaUrl(opts.source)
        if (opts.fullscreen)
            root.visibility = Window.FullScreen
    }

    // Called from C++ when a second instance forwards its arguments (single
    // instance mode): surface the window and apply its options.
    function handleSecondInstance(opts) {
        if (root.visibility === Window.Minimized
                || root.visibility === Window.Hidden)
            root.show()
        root.raise()
        root.requestActivate()
        root.applyStartupOptions(opts)
    }

    // Playlist as docked drawer or separate window, per preferences.
    readonly property bool playlistOpen: Settings.playlistAsWindow
                                         ? playlistWindow.visible
                                         : playlistPanel.opened

    function togglePlaylist() {
        if (Settings.playlistAsWindow) {
            playlistWindow.visible = !playlistWindow.visible
        } else if (playlistPanel.opened) {
            playlistPanel.close()
        } else {
            playlistPanel.open()
        }
    }

    // Keep the playlist in the newly selected home when the preference
    // changes while it is showing.
    Connections {
        target: Settings
        function onPlaylistAsWindowChanged() {
            if (Settings.playlistAsWindow && playlistPanel.opened) {
                playlistPanel.close()
                playlistWindow.visible = true
            } else if (!Settings.playlistAsWindow && playlistWindow.visible) {
                playlistWindow.visible = false
                playlistPanel.open()
            }
        }
    }

    // True while we paused because of minimization, to resume on restore.
    property bool autoPaused: false

    onVisibilityChanged: {
        if (!Settings.pauseWhenMinimized)
            return
        if (visibility === Window.Minimized
                && playerController.player.playbackState === MediaPlayer.PlayingState) {
            playerController.pause()
            autoPaused = true
        } else if (visibility !== Window.Minimized && autoPaused) {
            autoPaused = false
            playerController.player.play()
        }
    }

    onClosing: {
        if (Settings.rememberGeometry && visibility === Window.Windowed)
            Settings.windowGeometry = Qt.rect(root.x, root.y, root.width, root.height)
        playlistWindow.close()
    }

    Component.onCompleted: {
        if (!Settings.rememberVolume)
            Settings.volume = Settings.initialVolume / 100
        if (Settings.rememberGeometry) {
            const g = Settings.windowGeometry
            if (g.width >= 400 && g.height >= 300) {
                root.x = g.x
                root.y = g.y
                root.width = g.width
                root.height = g.height
            }
        }
        applyWindowPlacement()
        if (startupOptions && startupOptions.source !== undefined
                && startupOptions.source !== "")
            applyStartupOptions(startupOptions)
        else {
            applyStartupOptions(startupOptions) // window/state options, no media
            if (Settings.restorePlaylist)
                playerController.restoreSessionPlaylist()
        }
    }

    PlayerController {
        id: playerController
        videoOutput: videoOutput
    }

    // OS media integration: Windows SMTC / Linux MPRIS2 (media keys + the
    // desktop "now playing" widget). No-op on platforms without a backend.
    MediaControls {
        controller: playerController
        window: root
    }

    // Persisted settings drive the player; the UI only ever writes Settings.
    Binding {
        target: playerController.audioOutput
        property: "volume"
        value: Settings.volume
    }
    Binding {
        target: playerController.audioOutput
        property: "muted"
        value: Settings.muted
    }
    Binding {
        target: playerController.player
        property: "playbackRate"
        value: Settings.playbackRate
    }
    Binding {
        target: playerController.player
        property: "pitchCompensation"
        value: Settings.pitchCompensation
        when: playerController.player.pitchCompensationAvailability
              === MediaPlayer.Available
    }
    Binding {
        target: playerController
        property: "shuffle"
        value: Settings.playlistShuffle
    }
    Binding {
        target: playerController
        property: "repeatAll"
        value: Settings.playlistRepeat
    }
    Binding {
        target: playerController
        property: "resumeEnabled"
        value: Settings.resumePlayback
    }
    Binding {
        target: playerController
        property: "preferredAudioLanguages"
        value: Settings.preferredAudioLanguage
    }
    Binding {
        target: playerController
        property: "preferredSubtitleLanguages"
        value: Settings.preferredSubtitleLanguage
    }
    Binding {
        target: playerController
        property: "subtitlesByDefault"
        value: Settings.subtitlesByDefault
    }
    Binding {
        target: playerController
        property: "sessionPlaylistEnabled"
        value: Settings.restorePlaylist
    }
    Binding {
        target: playerController
        property: "autoAddFolderFiles"
        value: Settings.autoAddFolderFiles
    }
    Binding {
        target: playerController.recents
        property: "maxItems"
        value: Settings.recentsMaxItems
    }
    Binding {
        target: playerController
        property: "rememberTrackSelections"
        value: Settings.rememberFileSettings
    }
    Binding {
        target: playerController
        property: "autoPlayNext"
        value: Settings.autoPlayNext
    }
    Binding {
        target: playerController
        property: "disableScreensaver"
        value: Settings.disableScreensaver
    }
    Binding {
        target: playerController
        property: "dvdMenusEnabled"
        value: Settings.dvdUseMenus
    }
    Binding {
        target: playerController
        property: "dvdUseFirstPlay"
        value: Settings.dvdUseFirstPlay
    }
    Binding {
        // Global (device-level) A/V delay, remembered per audio output device;
        // the per-file delay is added on top. Re-evaluates when the active
        // device changes or the stored map changes (revision dependency).
        target: playerController
        property: "globalAudioDelay"
        // The `revision >= 0` term (always true) forces QML to track the store's
        // revision so the lookup re-runs when the map changes.
        value: Settings.audioDeviceDelaysRevision >= 0
               ? Settings.audioDelayForDevice(playerController.currentAudioDeviceId)
               : 0
    }
    Binding {
        target: playerController
        property: "audioDeviceId"
        value: Settings.audioDevice
    }
    Binding {
        target: playerController
        property: "autoloadSubtitles"
        value: Settings.subtitlesAutoload
    }
    Binding {
        target: playerController
        property: "playOnLoadPlaylist"
        value: Settings.playOnLoadPlaylist
    }
    Binding {
        target: playerController
        property: "ignorePlaybackErrors"
        value: Settings.ignorePlaybackErrors
    }
    Binding {
        target: playerController
        property: "networkTimeout"
        value: Settings.networkTimeout
    }
    Binding {
        target: playerController
        property: "mediaToAdd"
        value: Settings.mediaToAdd
    }
    Binding {
        target: playerController
        property: "autosavePlaylistOnExit"
        value: Settings.autosavePlaylistOnExit
    }
    Binding {
        target: playerController.playlist
        property: "displayTitleName"
        value: Settings.displayTitleName
    }
    Binding {
        target: playerController.playlist
        property: "autoSort"
        value: Settings.playlistAutoSort
    }

    menuBar: MainMenuBar {
        id: mainMenuBar
        visible: !root.fullscreen
        controller: playerController
        fullscreen: root.fullscreen
        playlistOpen: root.playlistOpen
        youtubeCacheCount: youtubeResolver.cacheCount
        onOpenFileRequested: fileDialog.open()
        onOpenDirectoryRequested: directoryDialog.open()
        onOpenPlaylistRequested: playlistDialog.open()
        onOpenDvdRequested: dvdDialog.open()
        onOpenUrlRequested: openUrlDialog.open()
        onYoutubeCacheRequested: cacheBrowser.openDialog()
        onCastRequested: castDialog.open()
        onEditTvChannelsRequested: favoritesDialog.openFor(
                qsTr("TV editor"), qsTr("TV channels"), Theme.icon("open_tv"),
                playerController.tvChannels)
        onEditRadioChannelsRequested: favoritesDialog.openFor(
                qsTr("Radio editor"), qsTr("Radio channels"), Theme.icon("open_radio"),
                playerController.radioChannels)
        onEditFavoritesRequested: favoritesDialog.openFor(
                qsTr("Favorite editor"), qsTr("Favorite list"),
                Theme.icon("open_favorites"), playerController.favorites)
        onAddBookmarkRequested: {
            playerController.addBookmark()
            root.showOsd(qsTr("Bookmark added"))
        }
        onEditBookmarksRequested: bookmarksDialog.open()
        onLoadSubtitlesRequested: subtitleDialog.open()
        onFindSubtitlesRequested: findSubtitlesDialog.openDialog()
        onSetSubtitleDelayRequested: subtitleDelayDialog.open()
        onSetAudioDelayRequested: audioDelayDialog.openDialog()
        onVideoEqualizerRequested: videoEqualizerDialog.open()
        onResizeToVideoPercentRequested: percent => root.resizeToVideoPercent(percent)
        onFullscreenToggleRequested: root.toggleFullscreen()
        onPlaylistToggleRequested: root.togglePlaylist()
        onScreenshotRequested: root.takeScreenshot()
        onInfoRequested: mediaInfoDialog.open()
        onPreferencesRequested: preferencesDialog.open()
        onEditMainToolbarRequested: toolbarEditor.openFor(
                qsTr("Edit main toolbar"), "main", Settings.mainToolbarItems,
                ToolbarItems.defaultMainToolbar, Settings.mainToolbarIconSize)
        onEditControlBarRequested: toolbarEditor.openFor(
                qsTr("Edit control bar"), "control", Settings.controlBarItems,
                ToolbarItems.defaultControlBar, Settings.controlBarIconSize)
        onAboutRequested: aboutDialog.open()
        onHelpContentsRequested: helpDialog.open()
        onCheckForUpdatesRequested: updateChecker.checkNow()
        onInstallYoutubeSupportRequested: youtubeSupportDialog.openDialog()
    }

    header: MainToolBar {
        id: mainToolBar
        // Mini and Mpc GUIs drop the main toolbar (SMPlayer's MiniGui/MpcGui).
        visible: !root.fullscreen && Settings.showToolbar
                 && Settings.gui === "Basic"
        controller: playerController
        playlistOpen: root.playlistOpen
        onOpenFileRequested: fileDialog.open()
        onOpenUrlRequested: openUrlDialog.open()
        onPlaylistToggleRequested: root.togglePlaylist()
        onScreenshotRequested: root.takeScreenshot()
        onInfoRequested: mediaInfoDialog.open()
        onPreferencesRequested: preferencesDialog.open()
        onEditFavoritesRequested: favoritesDialog.openFor(
                qsTr("Favorite editor"), qsTr("Favorite list"),
                Theme.icon("open_favorites"), playerController.favorites)
    }

    function takeScreenshot() {
        const path = playerController.takeScreenshot(Settings.screenshotFolder,
                                                     Settings.screenshotFormat)
        showOsd(path !== "" ? qsTr("Screenshot saved as %1").arg(path)
                            : qsTr("Could not take the screenshot"))
    }

    // Resize the window so the video is shown at the given percentage of its
    // native size (SMPlayer's Video > Size).
    function resizeToVideoPercent(percent) {
        const res = playerController.player.metaData.value(MediaMetaData.Resolution)
        if (!res || res.width <= 0 || res.height <= 0)
            return
        const chromeHeight = root.height - videoOutput.height
        const chromeWidth = root.width - videoOutput.width
        root.width = Math.min(Math.round(res.width * percent / 100) + chromeWidth,
                              Screen.desktopAvailableWidth - 60)
        root.height = Math.min(Math.round(res.height * percent / 100) + chromeHeight,
                               Screen.desktopAvailableHeight - 60)
    }

    // Video display area. Clipped so the zoom/pan/rotate transforms below
    // never spill over the menu bar or the control bar.
    Item {
        id: videoArea
        anchors.fill: parent
        clip: true

        VideoOutput {
            id: videoOutput
            anchors.fill: parent
        }

        // Video equalizer: Qt Multimedia has no brightness/contrast/etc., so
        // the video is captured and post-processed by a fragment shader. At
        // neutral settings the shader is an identity transform. Settings'
        // -100..100 range maps to the shader's neutral-at parameters.
        ShaderEffect {
            id: videoEqualizer
            anchors.fill: parent
            property variant source: ShaderEffectSource {
                sourceItem: videoOutput
                hideSource: true
                live: true
            }
            property real brightness: Settings.eqBrightness / 100.0
            property real contrast: (Settings.eqContrast + 100) / 100.0
            property real saturation: (Settings.eqSaturation + 100) / 100.0
            property real gamma: Math.pow(2, Settings.eqGamma / 100.0)
            property real hue: Settings.eqHue / 100.0 * Math.PI
            fragmentShader: "qrc:/qt/qml/Vivace/shaders/equalizer.frag.qsb"

            // ---- view transforms (aspect / zoom / pan / rotate / flip /
            // mirror). Qt Multimedia has no such controls, so they are applied
            // as QML transforms on the displayed video item. ----

            // Native display aspect of the current video (falls back to the
            // area aspect when unknown, i.e. audio-only or before load).
            readonly property real nativeAspect: {
                var res = playerController.player.metaData.value(
                            MediaMetaData.Resolution)
                if (res && res.width > 0 && res.height > 0)
                    return res.width / res.height
                return areaAspect
            }
            readonly property real areaAspect: height > 0 ? width / height : 16 / 9
            // 0 = auto (native); otherwise a forced display aspect ratio.
            readonly property real targetAspect: playerController.videoAspect > 0
                        ? playerController.videoAspect : nativeAspect
            // Letterboxed extent of a given aspect fit within the area.
            function fitWidth(a) { return a > areaAspect ? width : height * a }
            function fitHeight(a) { return a > areaAspect ? width / a : height }
            // Non-uniform scale that morphs the native-fit video to the forced
            // aspect while keeping it fit within the area (1,1 when auto).
            readonly property real aspectScaleX: fitWidth(targetAspect) / fitWidth(nativeAspect)
            readonly property real aspectScaleY: fitHeight(targetAspect) / fitHeight(nativeAspect)

            transform: [
                Scale {
                    origin.x: videoEqualizer.width / 2
                    origin.y: videoEqualizer.height / 2
                    xScale: videoEqualizer.aspectScaleX * playerController.videoZoom
                            * (playerController.videoMirror ? -1 : 1)
                    yScale: videoEqualizer.aspectScaleY * playerController.videoZoom
                            * (playerController.videoFlip ? -1 : 1)
                },
                Rotation {
                    origin.x: videoEqualizer.width / 2
                    origin.y: videoEqualizer.height / 2
                    angle: playerController.videoRotation
                },
                Translate {
                    x: playerController.videoPanX
                    y: playerController.videoPanY
                }
            ]
        }

        // Mouse wheel over the video: seek or volume, per preferences.
        WheelHandler {
            onWheel: event => {
                const direction = event.angleDelta.y > 0 ? 1 : -1
                if (Settings.wheelFunction === 0) {
                    if (playerController.player.seekable)
                        playerController.seekRelative(
                                    direction * Settings.seekWheelStep * 1000)
                } else if (Settings.wheelFunction === 1) {
                    Settings.volume = Math.max(0, Math.min(1,
                            Settings.volume + direction * Settings.volumeStep / 100))
                }
            }
        }
    }

    Label {
        anchors.centerIn: parent
        visible: playerController.player.source.toString() === ""
        text: qsTr("Drop media files here, or press Ctrl+O")
        color: "#808080"
        font.pixelSize: 20
    }

    // External-subtitle overlay (SRT/VTT/ASS parsed by PlayerController),
    // positioned per Settings.subPosition (% from the top).
    Text {
        id: subtitleOverlay
        visible: text !== ""
        text: playerController.currentSubtitleText
        textFormat: Text.StyledText
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        width: parent.width * 0.9
        anchors.horizontalCenter: parent.horizontalCenter
        y: Math.max(0, parent.height * Settings.subPosition / 100 - height / 2)
        color: "white"
        style: Text.Outline
        styleColor: "black"
        font.pixelSize: Settings.subFontSize
        font.family: Settings.subFontFamily !== ""
                     ? Settings.subFontFamily : Qt.application.font.family
    }

    // OSD, top-left over the video like SMPlayer's.
    Rectangle {
        id: osd
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 24
        width: osdLabel.implicitWidth + 24
        height: osdLabel.implicitHeight + 12
        radius: 4
        color: "#c0202020"
        opacity: 0

        Behavior on opacity {
            NumberAnimation { duration: 150 }
        }

        Label {
            id: osdLabel
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: Settings.osdFontSize
        }

        Timer {
            id: osdTimer
            interval: Settings.osdDuration
            onTriggered: osd.opacity = 0
        }
    }

    // Busy overlay shown while the external HD downloader is running (it can
    // take minutes — the tool downloads and merges separate video+audio streams).
    Rectangle {
        visible: externalDownloader.busy || youtubeResolver.downloading
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.8, 640)
        height: dlColumn.implicitHeight + 24
        radius: 8
        color: "#d0202020"

        ColumnLayout {
            id: dlColumn
            anchors.fill: parent
            anchors.margins: 12
            spacing: 8

            Label {
                text: qsTr("Downloading video…")
                color: "white"
                font.bold: true
                font.pixelSize: 16
            }
            Label {
                Layout.fillWidth: true
                text: qsTr("This can take a while — the external tool is "
                           + "downloading and merging HD video and audio.")
                color: "#cccccc"
                wrapMode: Text.WordWrap
            }
            Label {
                Layout.fillWidth: true
                text: root.downloadStatus
                visible: root.downloadStatus !== ""
                color: "#9e9e9e"
                elide: Text.ElideRight
                font.pixelSize: 11
            }
            Button {
                text: qsTr("Cancel")
                Layout.alignment: Qt.AlignRight
                onClicked: {
                    if (youtubeResolver.downloading)
                        youtubeResolver.cancel()
                    if (externalDownloader.busy)
                        externalDownloader.cancel()
                    root.downloadStatus = ""
                }
            }
        }
    }

    // Connecting / buffering indicator. Network streams (esp. TV tuners, which
    // send keep-alive padding until the channel locks) can take many seconds
    // before the first frame — without feedback the black screen looks broken.
    // A 700 ms delay keeps it from flashing on instant local-file loads.
    //
    // Restricted to network sources so local files — including those on a
    // network share (file://), which routinely report buffering — never get
    // the overlay; that flashing was the reported local-playback regression.
    // Only LoadingMedia (initial open) and StalledMedia (playback interrupted,
    // waiting for data) count; BufferingMedia ("buffering but has enough to keep
    // playing") is excluded so it can't flash over smooth playback. The tuner
    // device uses a non-file hint URL, so it still qualifies.
    readonly property int mediaStatusValue: playerController.player.mediaStatus
    readonly property bool sourceIsLocal:
            playerController.player.source.toString().startsWith("file:")
    // Note: a tuner played via setSourceDevice() has an empty source, and an
    // empty string is not local, so it correctly qualifies; local files
    // (file://) and idle (NoMedia) do not.
    readonly property bool streamLoading:
            !sourceIsLocal
            && (mediaStatusValue === MediaPlayer.LoadingMedia
                || mediaStatusValue === MediaPlayer.StalledMedia)
    property bool connectingShown: false
    onStreamLoadingChanged: {
        if (streamLoading) {
            connectingDelay.restart()
        } else {
            connectingDelay.stop()
            connectingShown = false
        }
    }
    Timer { id: connectingDelay; interval: 700; onTriggered: root.connectingShown = true }

    Rectangle {
        id: connectingOverlay
        visible: root.connectingShown && !externalDownloader.busy
                 && !youtubeResolver.downloading
        anchors.centerIn: parent
        width: connectingRow.implicitWidth + 32
        height: connectingRow.implicitHeight + 24
        radius: 8
        color: "#d0202020"

        RowLayout {
            id: connectingRow
            anchors.centerIn: parent
            spacing: 14

            BusyIndicator {
                // Only animate while the overlay is actually shown; a spinner
                // left running (hidden) churns the render loop and stutters
                // video playback.
                running: connectingOverlay.visible
                Layout.preferredWidth: 36
                Layout.preferredHeight: 36
            }
            ColumnLayout {
                spacing: 3
                Label {
                    text: root.mediaStatusValue === MediaPlayer.LoadingMedia
                          ? (root.sourceIsLocal ? qsTr("Loading…")
                                                : qsTr("Connecting…"))
                          : qsTr("Buffering… %1%").arg(Math.round(
                                playerController.player.bufferProgress * 100))
                    color: "white"
                    font.bold: true
                    font.pixelSize: 15
                }
                Label {
                    visible: !root.sourceIsLocal
                    text: qsTr("Live streams can take several seconds to start.")
                    color: "#bbbbbb"
                    font.pixelSize: 11
                }
            }
        }
    }

    function showOsd(text) {
        if (!Settings.osdEnabled)
            return
        osdLabel.text = text
        osd.opacity = 1
        osdTimer.restart()
    }

    function formatOsdTime(ms) {
        const totalSec = Math.max(0, Math.floor(ms / 1000))
        const h = Math.floor(totalSec / 3600)
        const m = Math.floor((totalSec % 3600) / 60)
        const s = totalSec % 60
        const pad = n => (n < 10 ? "0" : "") + n
        return pad(h) + ":" + pad(m) + ":" + pad(s)
    }

    Connections {
        target: Settings
        function onVolumeChanged() {
            root.showOsd(qsTr("Volume: %1%").arg(Math.round(Settings.volume * 100)))
        }
        function onMutedChanged() {
            root.showOsd(Settings.muted ? qsTr("Mute") : qsTr("Mute off"))
        }
        function onPlaybackRateChanged() {
            root.showOsd(qsTr("Speed: %1x").arg(Settings.playbackRate))
        }
    }

    // Each newly loaded file starts from the saved default equalizer values
    // (SMPlayer's per-file eq baseline; Set-as-default updates it, Reset zeroes
    // the current file).
    Connections {
        target: playerController.player
        function onMediaStatusChanged() {
            if (playerController.player.mediaStatus !== MediaPlayer.LoadedMedia)
                return
            // Seeking re-fires LoadedMedia for the SAME file (see the
            // perFileResetSource comment above) — only reset once per
            // genuinely new source, not on every seek. DVD's per-chapter
            // rebuilds still reset each time, since each chapter gets its
            // own unique hint URL (source genuinely changes).
            if (playerController.player.source === root.perFileResetSource)
                return
            root.perFileResetSource = playerController.player.source
            Settings.eqContrast = Settings.eqDefContrast
            Settings.eqBrightness = Settings.eqDefBrightness
            Settings.eqHue = Settings.eqDefHue
            Settings.eqSaturation = Settings.eqDefSaturation
            Settings.eqGamma = Settings.eqDefGamma
            // Playback speed is per-file (SMPlayer's mset.speed): each new
            // file starts at normal speed. Skip DVD — its LoadedMedia fires
            // on every chapter rebuild, which would reset speed mid-disc.
            if (!playerController.dvdPlayback)
                Settings.playbackRate = 1.0
        }
    }

    // Apply command-line --sub / --start once, on the first media load.
    Connections {
        target: playerController.player
        function onMediaStatusChanged() {
            if (playerController.player.mediaStatus !== MediaPlayer.LoadedMedia)
                return
            if (root.pendingSubtitle.toString() !== "") {
                playerController.loadSubtitles(root.pendingSubtitle)
                root.pendingSubtitle = ""
            }
            if (root.pendingStartMs >= 0) {
                if (playerController.player.seekable)
                    playerController.player.position = root.pendingStartMs
                root.pendingStartMs = -1
            }
        }
    }

    // Resize the main window to the video resolution when a new video
    // loads (SMPlayer's autoresize), if enabled and sensible to do.
    Connections {
        target: playerController.player
        function onMediaStatusChanged() {
            if (playerController.player.mediaStatus !== MediaPlayer.LoadedMedia
                    || root.visibility !== Window.Windowed) {
                return
            }
            if (Settings.startInFullscreen
                    && playerController.player.hasVideo) {
                root.visibility = Window.FullScreen
                return
            }
            // Hide the (empty) video area for audio-only files by collapsing
            // the window to its chrome (SMPlayer's hide-video-on-audio).
            if (Settings.hideVideoOnAudio && !playerController.player.hasVideo) {
                if (videoOutput.height > 1) {
                    root.heightBeforeAudioCollapse = root.height
                    root.height = root.height - videoOutput.height + 1
                }
                return
            }
            // A video appeared after a prior audio collapse: restore height.
            if (root.heightBeforeAudioCollapse > 0
                    && playerController.player.hasVideo) {
                root.height = root.heightBeforeAudioCollapse
                root.heightBeforeAudioCollapse = 0
            }
            if (Settings.mainwindowResizeMode === 0) // 0 = never
                return
            const res = playerController.player.metaData.value(MediaMetaData.Resolution)
            if (!res || res.width <= 0 || res.height <= 0)
                return
            const chromeHeight = root.height - videoOutput.height
            root.width = Math.min(res.width, Screen.desktopAvailableWidth - 60)
            root.height = Math.min(res.height + chromeHeight,
                                   Screen.desktopAvailableHeight - 60)
        }
    }

    // Saved window height while collapsed for audio-only playback (0 = not).
    property int heightBeforeAudioCollapse: 0

    Connections {
        target: playerController
        function onSeeked(positionMs) {
            root.showOsd(root.formatOsdTime(positionMs) + " / "
                         + root.formatOsdTime(playerController.player.duration))
        }
        function onPlaybackFinished() {
            if (Settings.closeOnFinish || root.cliCloseAtEnd)
                root.close()
        }
        function onErrorMessage(message) {
            root.showOsd(message)
        }
        function onOsdMessage(message) {
            root.showOsd(message)
        }
    }

    // Auto-hide the pointer over the video after inactivity while playing, in
    // the windowed main view as well as fullscreen, like SMPlayer. A stationary
    // MouseArea.cursorShape won't re-apply, so hiding uses the application
    // override cursor (UiHelpers.setCursorHidden) — which is app-wide and must
    // be tightly scoped. It is gated on the pointer being over the video
    // (videoMouse.containsMouse) and cleared the instant the pointer moves
    // (videoMouse.onPositionChanged), leaves the video (containsMouse false) or
    // the window is deactivated (a dialog opens) — so the menu bar, control bar
    // and dialogs are never left cursorless.
    property bool cursorHidden: false
    readonly property bool cursorAutoHide:
            Settings.hideMouseInFullscreen && videoMouse.containsMouse
            && playerController.player.playbackState === MediaPlayer.PlayingState
            && !playerController.dvdInMenu

    onCursorHiddenChanged: UiHelpers.setCursorHidden(cursorHidden)

    onCursorAutoHideChanged: {
        if (cursorAutoHide) {
            cursorHideTimer.restart()
        } else {
            cursorHideTimer.stop()
            cursorHidden = false
        }
    }

    // Restore if the window is deactivated (e.g. a dialog opens via keyboard
    // while the pointer sits idle over the video), so no window is cursorless.
    onActiveChanged: if (!active) cursorHidden = false

    Timer {
        id: cursorHideTimer
        interval: Settings.mouseHideDelay
        onTriggered: {
            if (root.cursorAutoHide)
                root.cursorHidden = true
        }
    }

    // Delays the single-click action so double clicks don't also fire it.
    Timer {
        id: leftClickTimer
        interval: 220
        onTriggered: {
            switch (Settings.leftClickFunction) {
            case 1: playerController.togglePlayPause(); break
            case 2: root.toggleFullscreen(); break
            }
        }
    }

    MouseArea {
        id: videoMouse
        anchors.fill: parent
        // Don't cover the fullscreen chrome overlays, so their controls get the
        // clicks instead of the video's gesture/click handling.
        anchors.topMargin: fullscreenTopChrome.visible ? fullscreenTopChrome.height : 0
        anchors.bottomMargin: fullscreenControls.visible ? fullscreenControls.height : 0
        acceptedButtons: Qt.LeftButton | Qt.MiddleButton
        // Track hover so containsMouse gates the cursor auto-hide to the video
        // and onPositionChanged reveals the pointer on movement.
        hoverEnabled: true
        cursorShape: Qt.ArrowCursor
        // A long-press near an edge reveals the fullscreen chrome (touch has no
        // hover). ~0.5 s so it doesn't clash with tap or swipe-seek.
        pressAndHoldInterval: 500

        // --- Gesture seek: swipe left/right (touch or mouse-drag) to seek. ---
        // Horizontal drag past a threshold scrubs relative to where it began;
        // an OSD previews the target while dragging and the seek applies on
        // release. A drag suppresses the click action so it doesn't also fire.
        readonly property real gestureMsPerPixel: 150
        readonly property int gestureThreshold: 24
        property bool gestureActive: false
        property bool longPressed: false
        property real gestureStartX: 0
        property real gesturePendingMs: 0

        function seekEnabled() {
            return Settings.gestureSeek && playerController.player.seekable
                   && !playerController.dvdPlayback
        }

        onPressed: mouse => {
            gestureActive = false
            longPressed = false
            gestureStartX = mouse.x
            gesturePendingMs = 0
        }
        // Long-press in fullscreen: top half reveals the menu bar + toolbar,
        // bottom half reveals the transport controls.
        onPressAndHold: mouse => {
            if (!root.fullscreen)
                return
            longPressed = true
            if (mouse.y < height / 2)
                root.revealTopChrome()
            else
                root.revealBottomChrome()
        }
        onPositionChanged: mouse => {
            // Any real pointer movement over the video reveals the cursor and
            // re-arms the idle timer (fires on moves, not per-frame).
            root.cursorHidden = false
            if (root.cursorAutoHide)
                cursorHideTimer.restart()
            if (playerController.dvdInMenu)
                menuIdleTimer.restart()
            if (!pressed || !seekEnabled())
                return
            const dx = mouse.x - gestureStartX
            if (!gestureActive && Math.abs(dx) < gestureThreshold)
                return
            gestureActive = true
            const dur = playerController.player.duration
            let target = playerController.smoothPosition + dx * gestureMsPerPixel
            target = Math.max(0, dur > 0 ? Math.min(target, dur) : target)
            gesturePendingMs = target
            const deltaSec = Math.round((target - playerController.smoothPosition) / 1000)
            root.showOsd((deltaSec >= 0 ? "+" : "") + deltaSec + " s  →  "
                         + root.formatClock(target))
        }
        onReleased: {
            if (gestureActive) {
                playerController.setPosition(gesturePendingMs)
                gestureActive = false
            }
        }

        onDoubleClicked: mouse => {
            if (mouse.button !== Qt.LeftButton)
                return
            leftClickTimer.stop()
            switch (Settings.doubleClickFunction) {
            case 0: root.toggleFullscreen(); break
            case 1: playerController.togglePlayPause(); break
            }
        }
        onClicked: mouse => {
            // A completed swipe or a long-press already handled the interaction.
            if (gestureActive || longPressed) {
                gestureActive = false
                longPressed = false
                return
            }
            // In a DVD menu, clicks outside a button do nothing (button clicks
            // are handled by the overlay on top); don't toggle play/pause.
            if (playerController.dvdInMenu)
                return
            if (mouse.button === Qt.LeftButton) {
                if (Settings.leftClickFunction !== 0)
                    leftClickTimer.restart()
                return
            }
            if (mouse.button !== Qt.MiddleButton)
                return
            switch (Settings.middleClickFunction) {
            case 0: Settings.muted = !Settings.muted; break
            case 1: playerController.togglePlayPause(); break
            case 2: root.toggleFullscreen(); break
            }
        }
    }

    // Experimental DVD menu overlay: clickable button rectangles parsed from
    // the disc's PCI (PlayerController.dvdMenuButtons), mapped from the menu
    // video's coordinate space onto the rectangle where the frame is drawn
    // (videoOutput.contentRect). Click activates a button; hover highlights it;
    // arrow keys navigate and Enter activates.
    Item {
        id: dvdMenuOverlay
        anchors.fill: videoArea
        visible: playerController.dvdInMenu
        focus: visible

        readonly property int spaceW: Math.max(1, playerController.dvdMenuSpaceWidth)
        readonly property int spaceH: Math.max(1, playerController.dvdMenuSpaceHeight)
        readonly property rect content: {
            var c = videoOutput.contentRect
            return (c.width > 0 && c.height > 0) ? c
                                                 : Qt.rect(0, 0, width, height)
        }
        readonly property real sx: content.width / spaceW
        readonly property real sy: content.height / spaceH
        // The disc's own subpicture highlight (button outlines, selected button
        // recoloured). When present we show it and skip the drawn borders.
        readonly property string spuUrl: playerController.dvdMenuHighlightUrl
        readonly property bool hasSpu: spuUrl !== ""

        // Subpicture highlight image, stretched over the video content rect.
        // Composited translucently: DVD menus often author the selected-button
        // highlight as an opaque fill over the button, which would hide the
        // label rendered in the video beneath — a partial opacity keeps the
        // label/thumbnail readable through the highlight colour (the disc's
        // shape + colour are preserved).
        Image {
            visible: dvdMenuOverlay.hasSpu
            x: dvdMenuOverlay.content.x
            y: dvdMenuOverlay.content.y
            width: dvdMenuOverlay.content.width
            height: dvdMenuOverlay.content.height
            source: dvdMenuOverlay.spuUrl
            fillMode: Image.Stretch
            opacity: 0.5
            smooth: true
            cache: false
        }

        Repeater {
            model: playerController.dvdMenuButtons
            delegate: Rectangle {
                id: btnRect
                required property var modelData
                readonly property bool selected:
                        modelData.index === playerController.dvdMenuSelected
                x: dvdMenuOverlay.content.x + modelData.x * dvdMenuOverlay.sx
                y: dvdMenuOverlay.content.y + modelData.y * dvdMenuOverlay.sy
                width: modelData.w * dvdMenuOverlay.sx
                height: modelData.h * dvdMenuOverlay.sy
                // Drawn highlight is a fallback when the disc has no subpicture.
                color: (!dvdMenuOverlay.hasSpu && selected) ? "#33ffd54a" : "transparent"
                border.width: dvdMenuOverlay.hasSpu ? 0 : (selected ? 3 : 1)
                border.color: selected ? "#ffd54a" : "#66ffffff"
                radius: 3

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onEntered: {
                        playerController.dvdMenuHover(btnRect.modelData.index)
                        menuIdleTimer.restart()
                    }
                    onClicked: playerController.dvdMenuActivate(btnRect.modelData.index)
                }
            }
        }

        // Return to playback (the main title) after a spell of no interaction,
        // so the menu doesn't sit forever; any menu activity restarts it.
        // Settings.dvdMenuTimeout == 0 disables it (menu stays until a choice).
        Timer {
            id: menuIdleTimer
            interval: Math.max(1, Settings.dvdMenuTimeout) * 1000
            running: dvdMenuOverlay.visible && Settings.dvdMenuTimeout > 0
            repeat: false
            onTriggered: playerController.dvdPlayMainTitle()
        }

        Keys.onUpPressed: { playerController.dvdMenuMove("up"); menuIdleTimer.restart() }
        Keys.onDownPressed: { playerController.dvdMenuMove("down"); menuIdleTimer.restart() }
        Keys.onLeftPressed: { playerController.dvdMenuMove("left"); menuIdleTimer.restart() }
        Keys.onRightPressed: { playerController.dvdMenuMove("right"); menuIdleTimer.restart() }
        Keys.onReturnPressed: playerController.dvdMenuActivateSelected()
        Keys.onEnterPressed: playerController.dvdMenuActivateSelected()
    }

    DropArea {
        anchors.fill: parent
        onDropped: drop => {
            if (!drop.hasUrls)
                return
            // Capture the dropped URLs as plain strings (the drop event is only
            // valid during this handler) and defer the actual open. Opening
            // media synchronously here would run setSource()/play() inside
            // Windows' OLE drag-and-drop nested modal loop, which corrupts the
            // FFmpeg backend's initialisation -- seen as either "Media session
            // serious error" or a spurious "Unsupported media, a codec is
            // missing" depending on timing/codec. A Qt.callLater() defer was
            // tried first and did NOT fix it: it only queues to the next idle
            // moment, which can still be *inside* the OLE loop's own nested
            // message pump. A real Timer delay gives that native loop actual
            // wall-clock time to finish unwinding before the media backend is
            // touched.
            var urls = []
            for (var i = 0; i < drop.urls.length; i++)
                urls.push(drop.urls[i].toString())
            dropOpenTimer.pendingUrls = urls
            dropOpenTimer.restart()
        }
    }

    Timer {
        id: dropOpenTimer
        interval: 150
        property var pendingUrls: []
        onTriggered: root.openDroppedUrls(pendingUrls)
    }

    FileDialog {
        id: fileDialog
        fileMode: FileDialog.OpenFiles
        options: Settings.useNativeFileDialog ? 0 : FileDialog.DontUseNativeDialog
        nameFilters: [
            qsTr("Media files (*.mp4 *.mkv *.avi *.mov *.webm *.wmv *.ts *.m2ts *.flv *.ogv *.mp3 *.m4a *.flac *.ogg *.opus *.wav *.wma *.m3u *.m3u8)"),
            qsTr("All files (*)")
        ]
        onAccepted: {
            if (Settings.rememberLastDir)
                Settings.lastOpenFolder = currentFolder
            playerController.open(selectedFiles)
        }
        Component.onCompleted: {
            if (Settings.rememberLastDir
                    && Settings.lastOpenFolder.toString() !== "")
                currentFolder = Settings.lastOpenFolder
        }
    }

    FolderDialog {
        id: directoryDialog
        options: Settings.useNativeFileDialog ? 0 : FolderDialog.DontUseNativeDialog
        onAccepted: playerController.openDirectory(
                        selectedFolder, Settings.addDirectoriesRecursively)
    }

    FolderDialog {
        id: dvdDialog
        title: qsTr("Select the DVD drive or a folder containing VIDEO_TS")
        options: Settings.useNativeFileDialog ? 0 : FolderDialog.DontUseNativeDialog
        onAccepted: {
            if (!playerController.openDvd(selectedFolder))
                root.showOsd(qsTr("No DVD video found in %1")
                             .arg(UiHelpers.toLocalPath(selectedFolder)))
        }
    }

    FileDialog {
        id: playlistDialog
        fileMode: FileDialog.OpenFile
        options: Settings.useNativeFileDialog ? 0 : FileDialog.DontUseNativeDialog
        nameFilters: [
            qsTr("Playlists (*.m3u *.m3u8)"),
            qsTr("All files (*)")
        ]
        onAccepted: {
            if (Settings.rememberLastDir)
                Settings.lastOpenFolder = currentFolder
            playerController.open([selectedFile])
        }
        Component.onCompleted: {
            if (Settings.rememberLastDir
                    && Settings.lastOpenFolder.toString() !== "")
                currentFolder = Settings.lastOpenFolder
        }
    }

    OpenUrlDialog {
        id: openUrlDialog
        onAccepted: url => root.openMediaUrl(url)
    }

    // Optional yt-dlp handler. Stream mode resolves a direct URL; download mode
    // downloads an HD file (cookies + ffmpeg merge) to a temp folder, plays it,
    // and deletes it on the next load / exit.
    YoutubeResolver {
        id: youtubeResolver
        ytdlPath: Settings.ytdlPath
        preferredHeight: Settings.youtubeQuality
        cookiesFile: Settings.youtubeCookiesFile
        ffmpegLocation: Settings.youtubeFfmpegLocation
        cacheDir: Settings.youtubeCacheDir
        cacheSize: Settings.youtubeCacheSize
        thumbnailOffset: Settings.youtubeThumbnailOffset
        onResolved: (mediaUrl, title, pageUrl) => {
            playerController.openStream(mediaUrl, title)
            root.showOsd(title !== "" ? title : qsTr("Playing stream"))
        }
        onProgress: line => root.downloadStatus = line
        onDownloaded: (fileUrl, title) => {
            root.downloadStatus = ""
            playerController.open([fileUrl])
            root.showOsd(title !== "" ? title : qsTr("Playing downloaded video"))
        }
        onFailed: message => {
            root.downloadStatus = ""
            root.showOsd(qsTr("YouTube: %1").arg(message))
        }
    }

    // Optional external downloader for HD YouTube: runs the user's tool to
    // download the video (merging separate HD video+audio streams), then plays
    // the produced file.
    ExternalDownloader {
        id: externalDownloader
        command: Settings.youtubeDownloaderCommand
        arguments: Settings.youtubeDownloaderArgs
        outputFolder: Settings.youtubeDownloadFolder
        onProgress: line => root.downloadStatus = line
        onDownloaded: (fileUrl, title) => {
            root.downloadStatus = ""
            playerController.open([fileUrl])
            root.showOsd(title !== "" ? title : qsTr("Playing downloaded video"))
        }
        onFailed: message => {
            root.downloadStatus = ""
            root.showOsd(qsTr("Download failed: %1").arg(message))
        }
    }

    // Applies Preferences > Network > Proxy application-wide whenever a
    // field changes (instant-apply, like the rest of Preferences).
    NetworkProxyController {
        enabled: Settings.proxyEnabled
        type: Settings.proxyType
        host: Settings.proxyHost
        port: Settings.proxyPort
        username: Settings.proxyUsername
        password: Settings.proxyPassword
    }

    // Play > Cast > Smartphone/tablet: serves the file currently playing to
    // any browser on the LAN. Lives here (not owned by the dialog) so an
    // active session survives the dialog being closed.
    CastServer {
        id: castServer
        controller: playerController
        port: Settings.castPort
    }
    CastDialog {
        id: castDialog
        server: castServer
    }

    YoutubeSupportDialog {
        id: youtubeSupportDialog
        resolver: youtubeResolver
    }

    // Thumbnail browser for the YouTube download cache (Open > YouTube cache).
    CacheBrowserDialog {
        id: cacheBrowser
        resolver: youtubeResolver
        onPlayRequested: fileUrl => playerController.open([fileUrl])
        onDeleteRequested: fileUrls => {
            // If one of the files is loaded in the player, release it first so
            // its OS handle is freed and the file can actually be deleted.
            const current = playerController.player.source.toString()
            for (let i = 0; i < fileUrls.length; i++)
                if (fileUrls[i] === current) {
                    playerController.closeSource()
                    break
                }
            for (let i = 0; i < fileUrls.length; i++)
                youtubeResolver.removeCacheEntry(fileUrls[i])
            cacheBrowser.reload()
        }
    }

    // Route an opened URL: a Windows ".url" internet shortcut is first expanded
    // to its target; a YouTube page then goes to the external HD downloader (if
    // configured) or the yt-dlp streaming resolver (when enabled); everything
    // else is played directly by QMediaPlayer.
    function openMediaUrl(url) {
        var s = url.toString()
        if (s.toLowerCase().endsWith(".url")) {
            const target = UiHelpers.readInternetShortcut(url)
            if (target === "") {
                root.showOsd(qsTr("Could not read the shortcut file."))
                return
            }
            s = target
        }
        if (Settings.youtubeEnabled && youtubeResolver.isSupportedUrl(s)) {
            if (Settings.youtubeMode === 1) {
                // Download & play with Vivace's yt-dlp (HD, cookies). A cached
                // video plays instantly; otherwise the busy overlay (bound to
                // youtubeResolver.downloading) shows while it downloads.
                youtubeResolver.download(s)
            } else if (Settings.youtubeMode === 2
                       && externalDownloader.isConfigured()) {
                // Download & play with the user's external tool.
                root.downloadStatus = qsTr("Starting download…")
                root.showOsd(qsTr("Downloading video (this can take a while)…"))
                externalDownloader.download(s)
            } else {
                // Stream (default; also the fallback if a mode isn't configured).
                root.showOsd(qsTr("Resolving with yt-dlp…"))
                youtubeResolver.resolve(s)
            }
        } else {
            playerController.open([s])
        }
    }

    // Open URLs captured from a drag-and-drop. Called deferred (dropOpenTimer)
    // from DropArea.onDropped so the media backend is not initialised inside
    // Windows' OLE drag-and-drop nested modal loop — doing so corrupts the
    // FFmpeg backend's init. `urls` is a plain array of strings (the drop
    // event is not valid past its handler).
    function openDroppedUrls(urls) {
        if (urls.length === 0)
            return
        if (urls.length === 1) {
            root.openMediaUrl(urls[0])
            return
        }
        var items = []
        for (var i = 0; i < urls.length; i++) {
            var u = urls[i]
            if (u.toLowerCase().endsWith(".url")) {
                const target = UiHelpers.readInternetShortcut(u)
                if (target !== "")
                    items.push(target)
            } else {
                items.push(u)
            }
        }
        if (items.length > 0)
            playerController.open(items)
    }

    FileDialog {
        id: subtitleDialog
        title: qsTr("Load subtitle file")
        fileMode: FileDialog.OpenFile
        options: Settings.useNativeFileDialog ? 0 : FileDialog.DontUseNativeDialog
        nameFilters: [
            qsTr("Subtitle files (*.srt *.vtt *.ass *.ssa)"),
            qsTr("All files (*)")
        ]
        onAccepted: playerController.loadSubtitles(selectedFile)
    }

    Window {
        id: subtitleDelayDialog
        title: qsTr("Subtitle delay")
        flags: Qt.Dialog
        modality: Qt.WindowModal
        color: palette.window
        width: 320
        height: subDelayCol.implicitHeight + 24
        minimumWidth: 260
        minimumHeight: subDelayCol.implicitHeight + 24

        function open() {
            delaySpin.value = playerController.subtitleDelay
            if (transientParent) {
                x = transientParent.x + (transientParent.width - width) / 2
                y = transientParent.y + (transientParent.height - height) / 2
            }
            visible = true
            raise()
            requestActivate()
        }

        Shortcut {
            sequences: [StandardKey.Cancel]
            onActivated: subtitleDelayDialog.close()
        }

        ColumnLayout {
            id: subDelayCol
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Label { text: qsTr("Delay:") }
                SpinBox {
                    id: delaySpin
                    from: -60000; to: 60000; stepSize: 100
                    editable: true
                }
                Label { text: qsTr("ms") }
                Item { Layout.fillWidth: true }
            }

            MenuSeparator { Layout.fillWidth: true }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("OK")
                    onClicked: {
                        playerController.subtitleDelay = delaySpin.value
                        subtitleDelayDialog.close()
                    }
                }
                Button {
                    text: qsTr("Cancel")
                    onClicked: subtitleDelayDialog.close()
                }
            }
        }
    }

    // Per-file audio-delay dialog. A movable top-level window (like SMPlayer's),
    // with a "use the default" checkbox so the user can drop a per-file override
    // and fall back to the global default (Preferences > General > Audio).
    Window {
        id: audioDelayDialog
        title: qsTr("Audio delay — Vivace")
        flags: Qt.Dialog
        width: 400
        height: contentColumn.implicitHeight + 24
        minimumWidth: 340
        minimumHeight: contentColumn.implicitHeight + 24
        color: palette.window

        function openDialog() {
            useDefaultCheck.checked = playerController.fileAudioDelay === 0
            audioDelaySpin.value = playerController.fileAudioDelay
            show()
            raise()
            requestActivate()
        }

        // Escape closes without applying, like a normal dialog. (StandardKey.
        // Cancel maps to more than one sequence, so use `sequences`.)
        Shortcut {
            sequences: [StandardKey.Cancel]
            onActivated: audioDelayDialog.close()
        }

        ColumnLayout {
            id: contentColumn
            anchors.fill: parent
            anchors.margins: 12
            spacing: 8

            CheckBox {
                id: useDefaultCheck
                text: qsTr("No delay for this file (use the global delay only)")
                onToggled: if (checked) audioDelaySpin.value = 0
            }
            RowLayout {
                spacing: 8
                Label { text: qsTr("This file's delay:") }
                SpinBox {
                    id: audioDelaySpin
                    from: -10000; to: 10000; stepSize: 100
                    editable: true
                    enabled: !useDefaultCheck.checked
                }
                Label { text: qsTr("ms") }
                Item { Layout.fillWidth: true }
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                opacity: 0.7
                font.pixelSize: 12
                text: qsTr("The delay applied to this file is this value plus the "
                           + "global delay for the current audio device (%1 ms), "
                           + "set in Preferences > General > Audio. Negative values "
                           + "delay the video to match audio that arrives late "
                           + "(e.g. Bluetooth headphones).")
                      .arg(playerController.globalAudioDelay)
            }
            // Apply (left) keeps the dialog open so the user can try several
            // delay values against the playing video; Close (right) dismisses.
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("&Apply")
                    // The per-file value the dialog would apply.
                    readonly property int pending: useDefaultCheck.checked
                            ? 0 : audioDelaySpin.value
                    // Nothing to apply while it already matches the current file.
                    enabled: pending !== playerController.fileAudioDelay
                    onClicked: playerController.fileAudioDelay = pending
                }
                Button {
                    text: qsTr("&Close")
                    onClicked: audioDelayDialog.close()
                }
            }
        }
    }

    FindSubtitlesDialog {
        id: findSubtitlesDialog
        controller: playerController
    }

    AboutDialog {
        id: aboutDialog
    }

    HelpDialog {
        id: helpDialog
    }

    UpdateDialog {
        id: updateChecker
    }

    VideoEqualizerDialog {
        id: videoEqualizerDialog
    }

    PlaylistPanel {
        id: playlistPanel
        controller: playerController
        width: Math.min(360, root.width * 0.4)
        height: root.height
    }

    PlaylistWindow {
        id: playlistWindow
        controller: playerController
        visible: false
    }

    MediaInfoDialog {
        id: mediaInfoDialog
        controller: playerController
    }

    PreferencesDialog {
        id: preferencesDialog
        controller: playerController
    }

    FavoritesDialog {
        id: favoritesDialog
    }

    BookmarksDialog {
        id: bookmarksDialog
        controller: playerController
    }

    ToolbarEditor {
        id: toolbarEditor
        onAccepted: (target, items, iconSize) => {
            if (target === "main") {
                Settings.mainToolbarItems = items
                Settings.mainToolbarIconSize = iconSize
            } else if (target === "control") {
                Settings.controlBarItems = items
                Settings.controlBarIconSize = iconSize
            }
        }
    }

    // System tray icon (Options > Show icon in system tray). Left click
    // toggles the window; the context menu has show/hide, play/pause, quit.
    Platform.SystemTrayIcon {
        visible: Settings.showTrayIcon
        icon.source: "qrc:/qt/qml/Vivace/icons/app_32.png"
        tooltip: root.title

        onActivated: reason => {
            if (reason === Platform.SystemTrayIcon.Trigger
                    || reason === Platform.SystemTrayIcon.DoubleClick) {
                if (root.visibility === Window.Hidden
                        || root.visibility === Window.Minimized) {
                    root.show()
                    root.raise()
                    root.requestActivate()
                } else {
                    root.hide()
                }
            }
        }

        menu: Platform.Menu {
            Platform.MenuItem {
                text: root.visible ? qsTr("Hide window") : qsTr("Show window")
                onTriggered: root.visible ? root.hide()
                                          : (root.show(), root.requestActivate())
            }
            Platform.MenuItem {
                text: qsTr("Play / Pause")
                onTriggered: playerController.togglePlayPause()
            }
            Platform.MenuItem {
                text: qsTr("Stop")
                onTriggered: playerController.stop()
            }
            Platform.MenuSeparator {}
            Platform.MenuItem {
                text: qsTr("Quit")
                onTriggered: root.close()
            }
        }
    }

    // Shortcuts without a menu entry (menu Actions carry the rest). Sequences
    // come from the editable registry, like the menu Actions.
    Shortcut {
        sequence: Shortcuts.sequences["play_pause"]
        onActivated: playerController.togglePlayPause()
    }
    Shortcut {
        sequence: Shortcuts.sequences["volume_up"]
        onActivated: Settings.volume =
                         Math.min(1, Settings.volume + Settings.volumeStep / 100)
    }
    Shortcut {
        sequence: Shortcuts.sequences["volume_down"]
        onActivated: Settings.volume =
                         Math.max(0, Settings.volume - Settings.volumeStep / 100)
    }
    Shortcut {
        sequence: "Escape"
        enabled: root.fullscreen
        onActivated: root.visibility = Window.Windowed
    }

    // Windowed mode: the control bar sits below the video, outside of it,
    // like SMPlayer's bottom toolbar.
    footer: ControlBar {
        visible: !root.fullscreen && Settings.showControlBar
        controller: playerController
        guiMode: Settings.gui
        onFullscreenToggleRequested: root.toggleFullscreen()
    }

    // Fullscreen chrome (top menu bar + toolbar, bottom transport controls) is
    // overlaid on the video and auto-hidden. It's revealed by the pointer
    // approaching an edge (mouse) or a long-press near an edge (touch, since
    // there's no hover on a tablet), then auto-hidden after a few idle seconds.
    HoverHandler {
        id: windowHover
        target: null
    }

    property bool topChromeShown: false
    property bool bottomChromeShown: false
    Timer { id: topHideTimer; interval: 4000; onTriggered: root.topChromeShown = false }
    Timer { id: bottomHideTimer; interval: 4000; onTriggered: root.bottomChromeShown = false }
    function revealTopChrome() { topChromeShown = true; topHideTimer.restart() }
    function revealBottomChrome() { bottomChromeShown = true; bottomHideTimer.restart() }

    // Mouse near an edge reveals (and keeps alive) that chrome.
    readonly property real fsPointerY: root.fullscreen ? windowHover.point.position.y : -1
    onFsPointerYChanged: {
        if (!root.fullscreen || fsPointerY < 0)
            return
        if (fsPointerY < fullscreenTopChrome.height)
            revealTopChrome()
        else if (fsPointerY > root.height - fullscreenControls.height)
            revealBottomChrome()
    }

    // Top overlay: the menu bar + toolbar, reusing the docked instances' actions
    // by connecting each signal to the docked bar's (single source of handlers).
    Column {
        id: fullscreenTopChrome
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        z: 20
        visible: root.fullscreen && root.topChromeShown

        MainMenuBar {
            id: fsMenuBar
            width: parent.width
            controller: playerController
            fullscreen: root.fullscreen
            playlistOpen: root.playlistOpen
            youtubeCacheCount: youtubeResolver.cacheCount
        }
        MainToolBar {
            id: fsToolBar
            width: parent.width
            visible: Settings.showToolbar && Settings.gui === "Basic"
            controller: playerController
            playlistOpen: root.playlistOpen
        }

        Component.onCompleted: {
            const mm = ["openFileRequested", "openDirectoryRequested",
                "openPlaylistRequested", "openDvdRequested", "openUrlRequested",
                "youtubeCacheRequested", "editTvChannelsRequested",
                "editRadioChannelsRequested", "editFavoritesRequested",
                "addBookmarkRequested", "editBookmarksRequested",
                "loadSubtitlesRequested", "findSubtitlesRequested",
                "setSubtitleDelayRequested", "setAudioDelayRequested",
                "videoEqualizerRequested", "resizeToVideoPercentRequested",
                "fullscreenToggleRequested", "playlistToggleRequested",
                "screenshotRequested", "infoRequested", "preferencesRequested",
                "editMainToolbarRequested", "editControlBarRequested",
                "aboutRequested", "checkForUpdatesRequested",
                "installYoutubeSupportRequested", "helpContentsRequested"]
            for (const n of mm)
                fsMenuBar[n].connect(mainMenuBar[n])
            const tt = ["openFileRequested", "openUrlRequested", "openDvdRequested",
                "openDirectoryRequested", "playlistToggleRequested",
                "screenshotRequested", "infoRequested", "preferencesRequested",
                "fullscreenToggleRequested", "editFavoritesRequested"]
            for (const n of tt)
                fsToolBar[n].connect(mainToolBar[n])
        }
    }

    ControlBar {
        id: fullscreenControls
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        z: 20
        visible: root.fullscreen && root.bottomChromeShown
        controller: playerController
        guiMode: Settings.gui
        onFullscreenToggleRequested: root.toggleFullscreen()
    }
}
