/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Bottom control area, mirroring SMPlayer's DefaultGui: a control row
    (play/pause, stop | seek buttons around the time slider | fullscreen,
    mute, volume) and a status bar (state message left, time right).
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import "ToolbarItems.js" as Items

Pane {
    id: controlBar

    required property PlayerController controller

    signal fullscreenToggleRequested()

    // The active GUI ("Basic"/"Mini"/"Mpc"). Basic uses the editable
    // control-bar layout; Mini/Mpc use their fixed SMPlayer layouts.
    property string guiMode: "Basic"

    readonly property var layoutItems: guiMode === "Basic"
            ? (Settings.controlBarItems.length > 0 ? Settings.controlBarItems
                                                   : Items.defaultControlBar)
            : Items.defaultControlBarFor(guiMode)

    // Status bar: shown in Basic/Mpc (per the preference), hidden in Mini.
    readonly property bool showStatus: guiMode !== "Mini" && Settings.showStatusBar

    // Mpc puts the seek slider on its own full-width row above the buttons.
    readonly property bool seekOnOwnRow: guiMode === "Mpc"

    // QtObject, not MediaPlayer: the QML MediaPlayer type (QQuickMediaPlayer)
    // is not assignable from the C++ QMediaPlayer the controller owns.
    readonly property QtObject player: controller.player

    // Status text: brighter on the slate gradient, gray on the flat dark bg.
    readonly property color statusColor: Settings.toolbarGradient
                                          ? "#eef2f6" : "#a0a0a0"

    function formatTime(ms) {
        const totalSec = Math.max(0, Math.floor(ms / 1000))
        const h = Math.floor(totalSec / 3600)
        const m = Math.floor((totalSec % 3600) / 60)
        const s = totalSec % 60
        const pad = n => (n < 10 ? "0" : "") + n
        let out = pad(h) + ":" + pad(m) + ":" + pad(s)
        if (Settings.showMilliseconds)
            out += "." + ("00" + (Math.max(0, ms) % 1000)).slice(-3)
        return out
    }

    // Position/total or position/-remaining, per the Time format option.
    function timeDisplay(pos, total) {
        if (Settings.timeDisplayRemaining && total > 0)
            return formatTime(pos) + " / -" + formatTime(total - pos)
        return formatTime(pos) + " / " + formatTime(total)
    }

    function basename(url) {
        const s = url.toString()
        return decodeURIComponent(s.substring(s.lastIndexOf("/") + 1))
    }

    component Btn: ToolButton {
        icon.width: Theme.sz(Settings.controlBarIconSize)
        icon.height: Theme.sz(Settings.controlBarIconSize)
        icon.color: "transparent" // keep the theme PNGs full-color, untinted
        display: AbstractButton.IconOnly
        ToolTip.visible: hovered && ToolTip.text !== ""
        ToolTip.delay: 700
    }

    // Windows-trackbar look, as in SMPlayer: flat groove with the elapsed
    // part filled blue, and a light rectangular thumb. tickValues (slider
    // units) draw notch marks, used for DVD chapter positions.
    component WinSlider: Slider {
        id: slider

        property var tickValues: []
        // Optional A-B repeat region (slider units; -1 = unset).
        property real abStart: -1
        property real abEnd: -1

        background: Rectangle {
            x: slider.leftPadding
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            implicitWidth: 120
            implicitHeight: 7
            width: slider.availableWidth
            height: implicitHeight
            radius: 2
            color: slider.enabled ? "#9a9a9a" : "#5a5a5a"

            Rectangle {
                width: slider.visualPosition * parent.width
                height: parent.height
                radius: parent.radius
                color: slider.enabled ? "#1e78d7" : "#707070"
            }

            // A-B repeat region, drawn over the groove.
            Rectangle {
                visible: slider.abStart >= 0 && slider.abEnd > slider.abStart
                         && slider.to > slider.from
                x: (slider.abStart - slider.from) / (slider.to - slider.from)
                   * parent.width
                width: (slider.abEnd - slider.abStart) / (slider.to - slider.from)
                       * parent.width
                height: parent.height
                color: "#b0ffc400" // translucent amber
            }

            Repeater {
                model: slider.tickValues

                Rectangle {
                    required property var modelData
                    x: slider.to > slider.from
                       ? (modelData - slider.from) / (slider.to - slider.from)
                         * parent.width - width / 2
                       : 0
                    anchors.verticalCenter: parent.verticalCenter
                    width: 2
                    height: parent.height + 4
                    color: "#f0f0f0"
                    opacity: 0.9
                }
            }
        }

        handle: Rectangle {
            x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
            y: slider.topPadding + slider.availableHeight / 2 - height / 2
            implicitWidth: 11
            implicitHeight: 22
            radius: 2
            border.color: "#707070"
            color: !slider.enabled ? "#8a8a8a"
                   : slider.pressed ? "#cce8ff"
                   : slider.hovered ? "#ffffff" : "#e8e8e8"
        }
    }

    // The seek slider, shared by the inline (Basic/Mini) placement and the
    // Mpc full-width row. Hides for IFO-less DVDs (no meaningful duration).
    component SeekSlider: WinSlider {
        id: seekSlider
        readonly property bool dvd: controlBar.controller.dvdPlayback
        // Show the A-B repeat region (file playback only).
        abStart: dvd ? -1 : controlBar.controller.abMarkerA
        abEnd: dvd ? -1 : controlBar.controller.abMarkerB
        // A seek deferred to release: always for DVD (rebuilds the stream),
        // and for files when "seek when released" is chosen.
        property real pendingSeek: -1
        visible: !dvd || controlBar.controller.dvdTitleDurationMs > 0
        from: 0
        to: Math.max(1, dvd ? controlBar.controller.dvdTitleDurationMs
                            : controlBar.player.duration)
        tickValues: controlBar.controller.chapters
                        .map(c => c.startMs).filter(ms => ms > 0)
        enabled: controlBar.player.seekable
        onMoved: {
            if (!dvd && Settings.seekOnDrag)
                controlBar.player.position = value
            else
                pendingSeek = value
        }
        onPressedChanged: {
            if (pressed) {
                pendingSeek = -1
            } else if (pendingSeek >= 0) {
                if (dvd)
                    controlBar.controller.seekDvd(pendingSeek)
                else
                    controlBar.player.position = pendingSeek
                pendingSeek = -1
            }
        }
        Binding on value {
            // smoothPosition == player.position except it hides the one-frame
            // backward blip the resume guard corrects (no slider flash on
            // play-after-pause). For DVD it just mirrors player.position.
            value: controlBar.controller.smoothPosition
                   + (dvd ? controlBar.controller.dvdPositionOffsetMs : 0)
            when: !pressed
        }

        // Seek preview thumbnail: on hover, a hidden second player grabs the
        // frame at the cursor time and shows it in a small popup above the
        // groove. File playback only (no DVD / streams).
        readonly property bool previewEnabled:
            !dvd && controlBar.controller.seekPreviewAvailable
        HoverHandler {
            id: seekHover
            enabled: seekSlider.previewEnabled
        }
        // Cursor x within the groove and the time it maps to.
        readonly property real hoverX: seekHover.point.position.x
        readonly property real hoverTime:
            to > from ? Math.max(from, Math.min(to,
                from + hoverX / width * (to - from))) : 0

        // Throttle preview requests so dragging the cursor doesn't flood the
        // hidden player with seeks.
        Timer {
            id: previewThrottle
            interval: 60
            onTriggered: controlBar.controller.requestSeekPreview(
                             seekSlider.hoverTime)
        }
        onHoverTimeChanged: if (seekHover.hovered) previewThrottle.restart()
        onPreviewEnabledChanged: if (!previewEnabled) previewPopup.close()

        Popup {
            id: previewPopup
            visible: seekHover.hovered && seekSlider.previewEnabled
            closePolicy: Popup.NoAutoClose
            padding: 3
            // Center the popup over the cursor, clamped to the slider width.
            x: Math.max(0, Math.min(seekSlider.width - width,
                                    seekSlider.hoverX - width / 2))
            y: -height - 6
            background: Rectangle {
                color: "#101010"
                border.color: "#808080"
                radius: 3
            }
            onOpenedChanged: if (opened) {
                controlBar.controller.setPreviewVideoOutput(previewVideo)
                controlBar.controller.requestSeekPreview(seekSlider.hoverTime)
            }
            contentItem: Column {
                spacing: 2
                VideoOutput {
                    id: previewVideo
                    width: 176
                    height: 99
                    fillMode: VideoOutput.PreserveAspectFit
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: controlBar.formatTime(seekSlider.hoverTime)
                    color: "#ffffff"
                    font.pixelSize: 12
                }
            }
        }
    }

    // Column of an item in the current layout, or -1 when absent; and the
    // positions of every repeatable separator / spacer.
    function col(id) { return layoutItems.indexOf(id) }
    function positionsOf(id) {
        var out = []
        for (var i = 0; i < layoutItems.length; ++i)
            if (layoutItems[i] === id)
                out.push(i)
        return out
    }

    // A control-bar button whose icon/tooltip/visibility/column come from
    // its id; instances add only enabled/onClicked (and dynamic icons).
    component CBtn: ToolButton {
        property string itemId: ""
        visible: controlBar.col(itemId) >= 0
        Layout.row: 0
        Layout.column: controlBar.col(itemId)
        icon.width: Theme.sz(Settings.controlBarIconSize)
        icon.height: Theme.sz(Settings.controlBarIconSize)
        icon.color: "transparent"
        icon.source: visible ? Theme.icon(Items.iconFor(itemId)) : ""
        display: AbstractButton.IconOnly
        ToolTip.text: Items.labelFor(itemId)
        ToolTip.visible: hovered && ToolTip.text !== ""
        ToolTip.delay: 700
    }

    padding: 4

    background: Rectangle {
        color: "#e0202020"
        // SMPlayer's EditableToolbar gradient, toggleable in preferences.
        Rectangle {
            anchors.fill: parent
            visible: Settings.toolbarGradient
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#cedce7" }
                GradientStop { position: 1.0; color: "#596a72" }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 2

        // Mpc GUI: the seek slider spans its own full-width row above the
        // buttons (SMPlayer's separate timeslidewidget toolbar).
        RowLayout {
            Layout.fillWidth: true
            visible: controlBar.seekOnOwnRow
            SeekSlider { Layout.fillWidth: true }
        }

        // Control row (SMPlayer's controlwidget), editable: predefined
        // controls shown/positioned by the layout list.
        GridLayout {
            Layout.fillWidth: true
            rows: 1
            columnSpacing: 2
            rowSpacing: 0

            Repeater {
                model: controlBar.positionsOf("separator")
                delegate: ToolSeparator {
                    required property int modelData
                    Layout.row: 0
                    Layout.column: modelData
                }
            }
            Repeater {
                model: controlBar.positionsOf("spacer")
                delegate: Item {
                    required property int modelData
                    Layout.row: 0
                    Layout.column: modelData
                    Layout.fillWidth: true
                }
            }

            CBtn {
                itemId: "playpause"
                icon.source: visible ? (controlBar.player.playbackState === MediaPlayer.PlayingState
                                        ? Theme.icon("pause") : Theme.icon("play")) : ""
                enabled: controlBar.player.source.toString() !== ""
                         || controlBar.controller.playlist.count > 0
                onClicked: controlBar.controller.togglePlayPause()
            }
            CBtn {
                itemId: "stop"
                enabled: controlBar.player.playbackState !== MediaPlayer.StoppedState
                onClicked: controlBar.controller.stop()
            }
            CBtn {
                itemId: "previous"
                enabled: controlBar.controller.playlist.count > 0
                onClicked: controlBar.controller.previous()
            }
            CBtn {
                itemId: "next"
                enabled: controlBar.controller.playlist.currentIndex
                         < controlBar.controller.playlist.count - 1
                onClicked: controlBar.controller.next()
            }
            CBtn {
                itemId: "prevchapter"
                // Chapter buttons appear only when the media has chapters.
                visible: controlBar.col("prevchapter") >= 0
                         && controlBar.controller.chapters.length > 0
                onClicked: controlBar.controller.previousChapter()
            }
            CBtn {
                itemId: "nextchapter"
                visible: controlBar.col("nextchapter") >= 0
                         && controlBar.controller.chapters.length > 0
                onClicked: controlBar.controller.nextChapter()
            }
            CBtn {
                itemId: "rewindlong"
                ToolTip.text: qsTr("Rewind %1 s").arg(Settings.seekLongStep)
                enabled: controlBar.player.seekable
                onClicked: controlBar.controller.seekRelative(-Settings.seekLongStep * 1000)
            }
            CBtn {
                itemId: "rewindmed"
                ToolTip.text: qsTr("Rewind %1 s").arg(Settings.seekMediumStep)
                enabled: controlBar.player.seekable
                onClicked: controlBar.controller.seekRelative(-Settings.seekMediumStep * 1000)
            }
            CBtn {
                itemId: "rewindshort"
                ToolTip.text: qsTr("Rewind %1 s").arg(Settings.seekShortStep)
                enabled: controlBar.player.seekable
                onClicked: controlBar.controller.seekRelative(-Settings.seekShortStep * 1000)
            }
            CBtn {
                itemId: "forwardshort"
                ToolTip.text: qsTr("Forward %1 s").arg(Settings.seekShortStep)
                enabled: controlBar.player.seekable
                onClicked: controlBar.controller.seekRelative(Settings.seekShortStep * 1000)
            }
            CBtn {
                itemId: "forwardmed"
                ToolTip.text: qsTr("Forward %1 s").arg(Settings.seekMediumStep)
                enabled: controlBar.player.seekable
                onClicked: controlBar.controller.seekRelative(Settings.seekMediumStep * 1000)
            }
            CBtn {
                itemId: "forwardlong"
                ToolTip.text: qsTr("Forward %1 s").arg(Settings.seekLongStep)
                enabled: controlBar.player.seekable
                onClicked: controlBar.controller.seekRelative(Settings.seekLongStep * 1000)
            }
            CBtn {
                itemId: "fullscreen"
                onClicked: controlBar.fullscreenToggleRequested()
            }
            CBtn {
                itemId: "mute"
                icon.source: visible ? (Settings.muted ? Theme.icon("mute")
                                                        : Theme.icon("volume")) : ""
                checkable: true
                checked: Settings.muted
                onClicked: Settings.muted = !Settings.muted
            }

            SeekSlider {
                // Inline (Basic/Mini): shown only when the layout lists it
                // and it is not on the Mpc dedicated row.
                visible: !controlBar.seekOnOwnRow
                         && controlBar.col("seekslider") >= 0
                         && (!dvd || controlBar.controller.dvdTitleDurationMs > 0)
                Layout.row: 0
                Layout.column: controlBar.col("seekslider")
                Layout.fillWidth: true
            }
            WinSlider {
                id: volumeSlider
                visible: controlBar.col("volumeslider") >= 0
                Layout.row: 0
                Layout.column: controlBar.col("volumeslider")
                implicitWidth: 90
                from: 0
                to: 1
                onMoved: Settings.volume = value
                Binding on value {
                    value: Settings.volume
                    when: !volumeSlider.pressed
                }
            }
        }

        // Status bar (SMPlayer's statusbar: message left, time right).
        RowLayout {
            Layout.fillWidth: true
            spacing: 16
            visible: controlBar.showStatus

            Label {
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.pixelSize: 12
                color: controlBar.player.error !== MediaPlayer.NoError
                       ? "#ff6060" : controlBar.statusColor
                text: {
                    if (controlBar.player.error !== MediaPlayer.NoError)
                        return qsTr("Error: %1").arg(controlBar.player.errorString)
                    switch (controlBar.player.playbackState) {
                    case MediaPlayer.PlayingState:
                        return qsTr("Playing %1").arg(
                            controlBar.controller.mediaTitle !== ""
                            ? controlBar.controller.mediaTitle
                            : controlBar.basename(controlBar.player.source))
                    case MediaPlayer.PausedState:
                        return qsTr("Paused")
                    default:
                        return qsTr("Stopped")
                    }
                }
            }

            // Optional video/audio info (Options > Status bar).
            Label {
                visible: Settings.statusVideoInfo && controlBar.player.hasVideo
                font.pixelSize: 12
                color: controlBar.statusColor
                text: {
                    const res = controlBar.player.metaData.value(MediaMetaData.Resolution)
                    const codec = controlBar.player.metaData.stringValue(MediaMetaData.VideoCodec)
                    let t = ""
                    if (res && res.width > 0)
                        t += res.width + "x" + res.height
                    if (codec)
                        t += (t ? " " : "") + codec
                    return t
                }
            }
            Label {
                visible: Settings.statusAudioInfo && controlBar.player.hasAudio
                font.pixelSize: 12
                color: controlBar.statusColor
                text: {
                    const codec = controlBar.player.metaData.stringValue(MediaMetaData.AudioCodec)
                    return codec ? codec : ""
                }
            }
            Label {
                visible: Settings.statusFormatInfo
                         && controlBar.player.source.toString() !== ""
                font.pixelSize: 12
                color: controlBar.statusColor
                text: controlBar.player.metaData.stringValue(MediaMetaData.FileFormat)
            }
            Label {
                visible: Settings.statusBitrateInfo
                font.pixelSize: 12
                color: controlBar.statusColor
                text: {
                    const v = controlBar.player.metaData.value(MediaMetaData.VideoBitRate)
                    const a = controlBar.player.metaData.value(MediaMetaData.AudioBitRate)
                    const parts = []
                    if (v > 0) parts.push(qsTr("V: %1 kbps").arg(Math.round(v / 1000)))
                    if (a > 0) parts.push(qsTr("A: %1 kbps").arg(Math.round(a / 1000)))
                    return parts.join("  ")
                }
            }
            Label {
                visible: Settings.statusFrameCounter && controlBar.player.hasVideo
                font.pixelSize: 12
                color: controlBar.statusColor
                text: {
                    const fps = controlBar.player.metaData.value(MediaMetaData.VideoFrameRate)
                    if (!fps || fps <= 0)
                        return ""
                    return qsTr("Frame: %1").arg(
                            Math.floor(controlBar.controller.smoothPosition / 1000 * fps))
                }
            }

            Label {
                visible: controlBar.player.playbackRate !== 1
                text: "x" + controlBar.player.playbackRate.toFixed(2)
                font.pixelSize: 12
                color: controlBar.statusColor
            }

            Label {
                // DVD titles: total from the IFO structure (the demuxer's
                // own duration estimate is meaningless across VOB cells).
                text: {
                    if (!controlBar.controller.dvdPlayback)
                        return controlBar.timeDisplay(controlBar.controller.smoothPosition,
                                                      controlBar.player.duration)
                    // Chapter jumps restart the stream; the offset keeps
                    // the displayed time global to the title.
                    const pos = controlBar.controller.smoothPosition
                                + controlBar.controller.dvdPositionOffsetMs
                    const total = controlBar.controller.dvdTitleDurationMs
                    return total > 0 ? controlBar.timeDisplay(pos, total)
                                     : controlBar.formatTime(pos)
                }
                font.pixelSize: 12
                color: controlBar.statusColor
            }
        }
    }
}
