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
    // Number of videos in the YouTube download cache (drives the cache-browser
    // button's enabled state) -- mirrors MainMenuBar/MainToolBar's own
    // property of the same name.
    property int youtubeCacheCount: 0

    signal fullscreenToggleRequested()
    signal openBlurayRequested()
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

    // Forced display aspect ratios (SMPlayer's Video > Aspect ratio); 0 = auto.
    // Mirrors MainMenuBar/MainToolBar's own array of the same name/shape.
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

    // Same semantics as MainMenuBar/MainToolBar's adjustSpeed()/
    // adjustSpeedStep(): the former is multiplicative (Halve/Double
    // speed), the latter a fixed additive step (+/-10%, matching
    // SMPlayer's Core::incSpeed10/etc).
    function adjustSpeed(factor) {
        Settings.playbackRate =
                Math.max(0.1, Math.round(Settings.playbackRate * factor * 100) / 100)
    }
    function adjustSpeedStep(delta) {
        Settings.playbackRate =
                Math.max(0.1, Math.round((Settings.playbackRate + delta) * 100) / 100)
    }

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
    // Mpc full-width row. Stays visible (disabled) for IFO-less DVDs and
    // DVD menus (no meaningful duration) rather than disappearing --
    // hiding it entirely collapses its Layout.fillWidth space and shifts
    // every sibling control in the row (found 2026-08-16: the whole
    // control bar visibly re-flows the instant a DVD menu appears).
    component SeekSlider: WinSlider {
        id: seekSlider
        readonly property bool dvd: controlBar.controller.dvdPlayback
        // Same treatment as dvd throughout this component -- a Blu-ray
        // seek also rebuilds the stream (see BlurayDisc::ClipRun's own
        // doc comment for why: whichever ONE clip is currently open may
        // not be the one the target time falls into).
        readonly property bool bluray: controlBar.controller.blurayPlayback
        readonly property bool disc: dvd || bluray
        // Show the A-B repeat region (file playback only).
        abStart: disc ? -1 : controlBar.controller.abMarkerA
        abEnd: disc ? -1 : controlBar.controller.abMarkerB
        // A seek deferred to release: always for a disc (rebuilds the
        // stream), and for files when "seek when released" is chosen.
        property real pendingSeek: -1
        from: 0
        to: Math.max(1, dvd ? controlBar.controller.dvdTitleDurationMs
                     : bluray ? controlBar.controller.blurayTitleDurationMs
                     : controlBar.player.duration)
        tickValues: controlBar.controller.chapters
                        .map(c => c.startMs).filter(ms => ms > 0)
        enabled: controlBar.player.seekable
                 && (!dvd || controlBar.controller.dvdTitleDurationMs > 0)
                 && (!bluray || controlBar.controller.blurayTitleDurationMs > 0)
        onMoved: {
            if (!disc && Settings.seekOnDrag)
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
                else if (bluray)
                    controlBar.controller.seekBluray(pendingSeek)
                else
                    controlBar.player.position = pendingSeek
                pendingSeek = -1
            }
        }
        Binding on value {
            // smoothPosition == player.position except it hides the one-frame
            // backward blip the resume guard corrects (no slider flash on
            // play-after-pause). For DVD/Blu-ray it adds that disc type's
            // own title-global offset.
            value: controlBar.controller.smoothPosition
                   + (dvd ? controlBar.controller.dvdPositionOffsetMs
                      : bluray ? controlBar.controller.blurayPositionOffsetMs : 0)
            when: !pressed
        }

        // Seek preview thumbnail: shown on hover OR while pressed/dragging,
        // a hidden second player grabs the frame at the target time and
        // shows it in a small popup above the groove. File playback only
        // (no DVD / streams).
        //
        // Touch has no hover concept at all (a finger press never fires
        // HoverHandler), so "hover-only" left tablet users with no way to
        // ever see this. Extending it to "hovered || pressed" is not a full
        // fix -- it still means look-ahead and commit share the same
        // gesture, so the slider handle (and this popup) sit right under
        // the finger doing the dragging -- but it at least makes the
        // preview reachable on a tablet, which hover alone never was.
        readonly property bool previewEnabled:
            !dvd && controlBar.controller.seekPreviewAvailable
        readonly property bool previewActive: seekHover.hovered || seekSlider.pressed
        HoverHandler {
            id: seekHover
            enabled: seekSlider.previewEnabled
        }
        // Cursor x within the groove and the time it maps to (mouse hover).
        readonly property real hoverX: seekHover.point.position.x
        readonly property real hoverTime:
            to > from ? Math.max(from, Math.min(to,
                from + hoverX / width * (to - from))) : 0

        // While pressed (mouse-drag or touch), HoverHandler's own point
        // does not reliably keep updating -- it is a hover-only mechanism,
        // and a touch press never enters hover in the first place. The
        // Slider's own value/visualPosition, though, are already updated
        // live by its native drag handling for either input, and are what
        // the preview should reflect anyway (the exact target the handle
        // itself is about to seek to, not a separately-computed pixel
        // mapping) -- so use those directly whenever pressed.
        readonly property real previewTime: pressed ? value : hoverTime
        readonly property real previewX: pressed ? visualPosition * width : hoverX

        // Poll for preview updates at a steady cadence while active, rather
        // than debouncing on every value change: a "restart on change" timer
        // never elapses during a continuous drag, since almost every pixel
        // of motion restarts the countdown before it can fire -- the
        // thumbnail would freeze at the first frame and never advance for
        // as long as the drag kept going. A repeating timer instead throttles
        // to at most one request per interval without ever fully starving.
        Timer {
            id: previewThrottle
            interval: 60
            repeat: true
            running: seekSlider.previewActive && seekSlider.previewEnabled
            onTriggered: controlBar.controller.requestSeekPreview(
                             seekSlider.previewTime)
        }
        onPreviewEnabledChanged: if (!previewEnabled) previewPopup.close()

        Popup {
            id: previewPopup
            visible: seekSlider.previewActive && seekSlider.previewEnabled
            closePolicy: Popup.NoAutoClose
            padding: 3
            // Center the popup over the cursor/finger, clamped to the
            // slider width.
            x: Math.max(0, Math.min(seekSlider.width - width,
                                    seekSlider.previewX - width / 2))
            y: -height - 6
            background: Rectangle {
                color: "#101010"
                border.color: "#808080"
                radius: 3
            }
            onOpenedChanged: if (opened) {
                controlBar.controller.setPreviewVideoOutput(previewVideo)
                controlBar.controller.requestSeekPreview(seekSlider.previewTime)
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
                    text: controlBar.formatTime(seekSlider.previewTime)
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

    // Row-wrap support (same mechanism as MainToolBar.qml's own -- see its
    // comment for the full rationale): items wrap onto additional rows
    // instead of clipping off the edge on a narrow screen. Ordinary buttons
    // and separators tile into a uniform-width grid; the seek slider and
    // "spacer" (both Layout.fillWidth) get a dedicated full-width row each,
    // so their fillWidth doesn't force every other row sharing that column
    // index to stretch too. The volume slider is a small, fixed width and
    // stays an ordinary grid cell.
    readonly property real cellWidth: Theme.sz(Settings.controlBarIconSize) + 20
    // The containing WINDOW's width, not this Pane's own `width` -- see
    // MainToolBar.qml's identical property for the full "binding loop"
    // rationale (this control bar is a Pane too, so it's exposed to the
    // exact same Pane content-fit entanglement).
    readonly property int columnsPerRow: Math.max(1, Math.floor(
        (Window.window ? Window.window.width : width) / cellWidth))
    readonly property var wrapPositions: computeWrapPositions()

    function computeWrapPositions() {
        const perRow = columnsPerRow
        // Fast path: everything already fits on one row -- keep the exact
        // original single-row placement (column = i, seekslider/spacer
        // inline like every other item), since there is no second row for
        // their fillWidth column to spill into.
        if (layoutItems.length <= perRow) {
            const out = []
            for (let i = 0; i < layoutItems.length; ++i)
                out.push({ row: 0, column: i, span: 1 })
            return out
        }
        const out = []
        let row = 0, column = 0
        for (let i = 0; i < layoutItems.length; ++i) {
            if (layoutItems[i] === "spacer" || layoutItems[i] === "seekslider") {
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

    // A control-bar button whose icon/tooltip/visibility/column come from
    // its id; instances add only enabled/onClicked (and dynamic icons).
    component CBtn: ToolButton {
        id: cbtn
        property string itemId: ""
        property bool menuIndicator: false
        visible: controlBar.col(itemId) >= 0
        Layout.row: controlBar.wrapRow(controlBar.col(itemId))
        Layout.column: controlBar.wrapColumn(controlBar.col(itemId))
        icon.width: Theme.sz(Settings.controlBarIconSize)
        icon.height: Theme.sz(Settings.controlBarIconSize)
        icon.color: "transparent"
        icon.source: visible ? Theme.icon(Items.iconFor(itemId)) : ""
        display: AbstractButton.IconOnly
        ToolTip.text: Items.labelFor(itemId)
        ToolTip.visible: hovered && ToolTip.text !== ""
        ToolTip.delay: 700

        Canvas {
            visible: cbtn.menuIndicator
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

    // Popup menus for the menu-kind catalog items (mirrors MainToolBar.qml's
    // own popups exactly; "favorites"/"audiotrack"/"subtitletrack" are not
    // included -- those pre-date the editable-catalog rework and were never
    // wired to the control bar, an older, separate gap this fix doesn't
    // touch).
    FavoritesMenu {
        id: tvPopup
        controller: controlBar.controller
        model: controlBar.controller.tvChannels
        itemIcon: Theme.icon("open_tv")
        showActions: true
        onEditRequested: controlBar.editTvChannelsRequested()
        onAddCurrentRequested: controlBar.controller.addCurrentTo(controlBar.controller.tvChannels)
    }
    FavoritesMenu {
        id: radioPopup
        controller: controlBar.controller
        model: controlBar.controller.radioChannels
        itemIcon: Theme.icon("open_radio")
        showActions: true
        onEditRequested: controlBar.editRadioChannelsRequested()
        onAddCurrentRequested: controlBar.controller.addCurrentTo(controlBar.controller.radioChannels)
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
            onTriggered: controlBar.adjustSpeed(0.5)
        }
        AppMenuItem {
            text: qsTr("Double speed")
            icon.source: Theme.icon("speed-x200")
            onTriggered: controlBar.adjustSpeed(2)
        }
        MenuSeparator {}
        AppMenuItem {
            text: qsTr("Speed -10%")
            icon.source: Theme.icon("speed-10")
            onTriggered: controlBar.adjustSpeedStep(-0.1)
        }
        AppMenuItem {
            text: qsTr("Speed +10%")
            icon.source: Theme.icon("speed+10")
            onTriggered: controlBar.adjustSpeedStep(0.1)
        }
        MenuSeparator {}
        AppMenuItem { text: qsTr("Speed -4%"); icon.source: Theme.icon("speed-04"); onTriggered: controlBar.adjustSpeedStep(-0.04) }
        AppMenuItem { text: qsTr("Speed +4%"); icon.source: Theme.icon("speed+04"); onTriggered: controlBar.adjustSpeedStep(0.04) }
        MenuSeparator {}
        AppMenuItem { text: qsTr("Speed -1%"); icon.source: Theme.icon("speed-01"); onTriggered: controlBar.adjustSpeedStep(-0.01) }
        AppMenuItem { text: qsTr("Speed +1%"); icon.source: Theme.icon("speed+01"); onTriggered: controlBar.adjustSpeedStep(0.01) }
        MenuSeparator {}
        AppMenuItem {
            text: qsTr("Pitch compensation")
            checkable: true
            checked: Settings.pitchCompensation
            enabled: controlBar.player.pitchCompensationAvailability === MediaPlayer.Available
            onTriggered: {
                Settings.pitchCompensation = checked
                checked = Qt.binding(() => Settings.pitchCompensation)
            }
        }
    }
    AppMenu {
        id: videoTrackPopup
        Instantiator {
            model: controlBar.controller.videoTrackLabels.length === 0 ? 1 : 0
            delegate: AppMenuItem { text: qsTr("<empty>"); enabled: false }
            onObjectAdded: (index, object) => videoTrackPopup.insertItem(0, object)
            onObjectRemoved: (index, object) => videoTrackPopup.removeItem(object)
        }
        Instantiator {
            model: controlBar.controller.videoTrackLabels
            delegate: AppMenuItem {
                required property int index
                required property string modelData
                text: modelData
                checkable: true
                checked: controlBar.controller.activeVideoTrack === index
                onTriggered: {
                    controlBar.controller.activeVideoTrack = index
                    checked = Qt.binding(() => controlBar.controller.activeVideoTrack === index)
                }
            }
            onObjectAdded: (index, object) => videoTrackPopup.insertItem(index, object)
            onObjectRemoved: (index, object) => videoTrackPopup.removeItem(object)
        }
    }
    AppMenu {
        id: aspectPopup
        Instantiator {
            model: controlBar.aspectRatios
            delegate: AppMenuItem {
                required property int index
                required property var modelData
                text: modelData.label
                checkable: true
                checked: Math.abs(controlBar.controller.videoAspect - modelData.value) < 0.001
                onTriggered: {
                    controlBar.controller.videoAspect = modelData.value
                    checked = Qt.binding(() => Math.abs(
                        controlBar.controller.videoAspect - modelData.value) < 0.001)
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
            checked: controlBar.controller.videoRotation === 0
            onTriggered: {
                controlBar.controller.videoRotation = 0
                checked = Qt.binding(() => controlBar.controller.videoRotation === 0)
            }
        }
        AppMenuItem {
            text: qsTr("&Rotate by 90° clockwise")
            checkable: true
            checked: controlBar.controller.videoRotation === 90
            onTriggered: {
                controlBar.controller.videoRotation = 90
                checked = Qt.binding(() => controlBar.controller.videoRotation === 90)
            }
        }
        AppMenuItem {
            text: qsTr("Rotate by 90° &counterclockwise")
            checkable: true
            checked: controlBar.controller.videoRotation === 270
            onTriggered: {
                controlBar.controller.videoRotation = 270
                checked = Qt.binding(() => controlBar.controller.videoRotation === 270)
            }
        }
        AppMenuItem {
            text: qsTr("Rotate by &180°")
            checkable: true
            checked: controlBar.controller.videoRotation === 180
            onTriggered: {
                controlBar.controller.videoRotation = 180
                checked = Qt.binding(() => controlBar.controller.videoRotation === 180)
            }
        }
    }
    AppMenu {
        id: videoSizePopup
        AppMenuItem { text: qsTr("50%"); onTriggered: controlBar.resizeToVideoPercentRequested(50) }
        AppMenuItem { text: qsTr("100%"); onTriggered: controlBar.resizeToVideoPercentRequested(100) }
        AppMenuItem { text: qsTr("200%"); onTriggered: controlBar.resizeToVideoPercentRequested(200) }
    }
    AppMenu {
        id: titlesPopup
        Instantiator {
            model: controlBar.controller.dvdPlayback ? controlBar.controller.dvdTitles
                 : controlBar.controller.blurayPlayback ? controlBar.controller.blurayTitles
                 : []
            delegate: AppMenuItem {
                required property int index
                required property var modelData
                text: modelData.label
                checkable: true
                checked: controlBar.controller.dvdPlayback
                         ? modelData.number === controlBar.controller.dvdCurrentTitle
                         : modelData.index === controlBar.controller.blurayCurrentTitle
                onTriggered: controlBar.controller.dvdPlayback
                             ? controlBar.controller.playDvdTitle(modelData.number)
                             : controlBar.controller.playBlurayTitle(modelData.index)
            }
            onObjectAdded: (index, object) => titlesPopup.insertItem(index, object)
            onObjectRemoved: (index, object) => titlesPopup.removeItem(object)
        }
        Instantiator {
            model: (controlBar.controller.dvdTitles.length === 0
                    && controlBar.controller.blurayTitles.length === 0) ? 1 : 0
            delegate: AppMenuItem { text: qsTr("<empty>"); enabled: false }
            onObjectAdded: (index, object) => titlesPopup.insertItem(0, object)
            onObjectRemoved: (index, object) => titlesPopup.removeItem(object)
        }
    }
    AppMenu {
        id: chaptersPopup
        Instantiator {
            model: controlBar.controller.chapters
            delegate: AppMenuItem {
                required property int index
                required property var modelData
                text: modelData.label
                onTriggered: controlBar.controller.playChapter(index)
            }
            onObjectAdded: (index, object) => chaptersPopup.insertItem(index, object)
            onObjectRemoved: (index, object) => chaptersPopup.removeItem(object)
        }
        Instantiator {
            model: controlBar.controller.chapters.length === 0 ? 1 : 0
            delegate: AppMenuItem { text: qsTr("<empty>"); enabled: false }
            onObjectAdded: (index, object) => chaptersPopup.insertItem(0, object)
            onObjectRemoved: (index, object) => chaptersPopup.removeItem(object)
        }
    }
    AppMenu {
        id: bookmarksPopup
        readonly property var entries: {
            controlBar.controller.bookmarks.revision // establish dependency
            return controlBar.controller.bookmarks.entries()
        }
        AppMenuItem {
            text: qsTr("&Add new bookmark")
            enabled: controlBar.player.source.toString() !== ""
            onTriggered: controlBar.addBookmarkRequested()
        }
        AppMenuItem {
            text: qsTr("&Edit bookmarks…")
            enabled: controlBar.player.source.toString() !== ""
            onTriggered: controlBar.editBookmarksRequested()
        }
        MenuSeparator {}
        Instantiator {
            model: bookmarksPopup.entries
            delegate: AppMenuItem {
                required property int index
                required property var modelData
                text: modelData.label
                onTriggered: controlBar.controller.goToBookmark(modelData.time)
            }
            onObjectAdded: (index, object) => bookmarksPopup.insertItem(index + 3, object)
            onObjectRemoved: (index, object) => bookmarksPopup.removeItem(object)
        }
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
            columnSpacing: 2
            rowSpacing: 0

            Repeater {
                model: controlBar.positionsOf("separator")
                delegate: ToolSeparator {
                    required property int modelData
                    Layout.row: controlBar.wrapRow(modelData)
                    Layout.column: controlBar.wrapColumn(modelData)
                }
            }
            Repeater {
                model: controlBar.positionsOf("spacer")
                delegate: Item {
                    required property int modelData
                    Layout.row: controlBar.wrapRow(modelData)
                    Layout.column: controlBar.wrapColumn(modelData)
                    Layout.columnSpan: controlBar.wrapSpan(modelData)
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
                // Fullscreen is forced on Android (no real windowed mode
                // to toggle out of there), so the button has nothing
                // meaningful left to do -- hidden regardless of the
                // user's own toolbar layout.
                visible: controlBar.col(itemId) >= 0 && Qt.platform.os !== "android"
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
            CBtn { itemId: "openbluray"; onClicked: controlBar.openBlurayRequested() }
            CBtn {
                itemId: "tv"; menuIndicator: true
                onClicked: tvPopup.popup(this, 0, height)
            }
            CBtn {
                itemId: "radio"; menuIndicator: true
                onClicked: radioPopup.popup(this, 0, height)
            }
            CBtn {
                itemId: "youtubecache"
                enabled: controlBar.youtubeCacheCount > 0 && Settings.youtubeEnabled
                         && Settings.youtubeMode === 1
                onClicked: controlBar.youtubeCacheRequested()
            }
            CBtn { itemId: "cast"; onClicked: controlBar.castRequested() }
            CBtn {
                itemId: "speed"; menuIndicator: true
                onClicked: speedPopup.popup(this, 0, height)
            }
            CBtn { itemId: "speedhalve"; onClicked: controlBar.adjustSpeed(0.5) }
            CBtn { itemId: "speednormal"; onClicked: Settings.playbackRate = 1 }
            CBtn { itemId: "speeddouble"; onClicked: controlBar.adjustSpeed(2) }
            CBtn { itemId: "speeddec10"; onClicked: controlBar.adjustSpeedStep(-0.1) }
            CBtn { itemId: "speedinc10"; onClicked: controlBar.adjustSpeedStep(0.1) }
            CBtn { itemId: "abmarkera"; onClicked: controlBar.controller.setAMarker() }
            CBtn { itemId: "abmarkerb"; onClicked: controlBar.controller.setBMarker() }
            CBtn { itemId: "abclear"; onClicked: controlBar.controller.clearABMarkers() }
            CBtn {
                itemId: "videotrack"; menuIndicator: true
                enabled: controlBar.controller.videoTrackLabels.length > 0
                onClicked: videoTrackPopup.popup(this, 0, height)
            }
            CBtn { itemId: "equalizer"; onClicked: controlBar.videoEqualizerRequested() }
            CBtn {
                itemId: "aspectratio"; menuIndicator: true
                enabled: controlBar.player.hasVideo
                onClicked: aspectPopup.popup(this, 0, height)
            }
            CBtn {
                itemId: "rotate"; menuIndicator: true
                enabled: controlBar.player.hasVideo
                onClicked: rotatePopup.popup(this, 0, height)
            }
            CBtn {
                itemId: "videosize"; menuIndicator: true
                enabled: controlBar.player.hasVideo
                onClicked: videoSizePopup.popup(this, 0, height)
            }
            CBtn {
                itemId: "flip"
                enabled: controlBar.player.hasVideo
                checkable: true
                checked: controlBar.controller.videoFlip
                onClicked: controlBar.controller.videoFlip = !controlBar.controller.videoFlip
            }
            CBtn {
                itemId: "mirror"
                enabled: controlBar.player.hasVideo
                checkable: true
                checked: controlBar.controller.videoMirror
                onClicked: controlBar.controller.videoMirror = !controlBar.controller.videoMirror
            }
            CBtn {
                itemId: "audiodelaydec"
                onClicked: controlBar.controller.adjustFileAudioDelay(-100)
            }
            CBtn {
                itemId: "audiodelayinc"
                onClicked: controlBar.controller.adjustFileAudioDelay(100)
            }
            CBtn { itemId: "audiodelayset"; onClicked: controlBar.setAudioDelayRequested() }
            CBtn { itemId: "loadsubtitles"; onClicked: controlBar.loadSubtitlesRequested() }
            CBtn { itemId: "findsubtitles"; onClicked: controlBar.findSubtitlesRequested() }
            CBtn { itemId: "unloadsubtitles"; onClicked: controlBar.controller.unloadSubtitles() }
            CBtn {
                itemId: "subtitledelaydec"
                onClicked: controlBar.controller.adjustSubtitleDelay(-100)
            }
            CBtn {
                itemId: "subtitledelayinc"
                onClicked: controlBar.controller.adjustSubtitleDelay(100)
            }
            CBtn { itemId: "subtitledelayset"; onClicked: controlBar.setSubtitleDelayRequested() }
            CBtn {
                itemId: "dvdmenu"
                enabled: controlBar.controller.dvdHasMenu
                onClicked: controlBar.controller.showDvdMenu()
            }
            CBtn {
                itemId: "titles"; menuIndicator: true
                onClicked: titlesPopup.popup(this, 0, height)
            }
            CBtn {
                itemId: "chaptersmenu"; menuIndicator: true
                onClicked: chaptersPopup.popup(this, 0, height)
            }
            CBtn {
                itemId: "bookmarksmenu"; menuIndicator: true
                onClicked: bookmarksPopup.popup(this, 0, height)
            }
            CBtn {
                itemId: "addbookmark"
                enabled: controlBar.player.source.toString() !== ""
                onClicked: controlBar.addBookmarkRequested()
            }

            SeekSlider {
                // Inline (Basic/Mini): shown only when the layout lists it
                // and it is not on the Mpc dedicated row. Deliberately NOT
                // additionally gated on "!dvd || dvdTitleDurationMs > 0"
                // (removed 2026-08-16) -- that duplicated the exact same
                // condition the component's own default `visible` used to
                // have (see SeekSlider's own comment), and overriding it
                // here is what kept the slider disappearing during a DVD
                // menu even after that fix, since THIS binding wins for
                // every instance built from this object literal.
                visible: !controlBar.seekOnOwnRow
                         && controlBar.col("seekslider") >= 0
                Layout.row: controlBar.wrapRow(controlBar.col("seekslider"))
                Layout.column: controlBar.wrapColumn(controlBar.col("seekslider"))
                Layout.columnSpan: controlBar.wrapSpan(controlBar.col("seekslider"))
                Layout.fillWidth: true
            }
            WinSlider {
                id: volumeSlider
                visible: controlBar.col("volumeslider") >= 0
                Layout.row: controlBar.wrapRow(controlBar.col("volumeslider"))
                Layout.column: controlBar.wrapColumn(controlBar.col("volumeslider"))
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
                // DVD/Blu-ray titles: total from the disc's own declared
                // structure (the demuxer's own duration estimate is
                // meaningless across VOB cells / BD clips -- see
                // BlurayDisc::ClipRun's own doc comment for the BD case).
                text: {
                    if (controlBar.controller.dvdPlayback) {
                        // Chapter jumps restart the stream; the offset keeps
                        // the displayed time global to the title.
                        const pos = controlBar.controller.smoothPosition
                                    + controlBar.controller.dvdPositionOffsetMs
                        const total = controlBar.controller.dvdTitleDurationMs
                        return total > 0 ? controlBar.timeDisplay(pos, total)
                                         : controlBar.formatTime(pos)
                    }
                    if (controlBar.controller.blurayPlayback) {
                        const pos = controlBar.controller.smoothPosition
                                    + controlBar.controller.blurayPositionOffsetMs
                        const total = controlBar.controller.blurayTitleDurationMs
                        return total > 0 ? controlBar.timeDisplay(pos, total)
                                         : controlBar.formatTime(pos)
                    }
                    return controlBar.timeDisplay(controlBar.controller.smoothPosition,
                                                  controlBar.player.duration)
                }
                font.pixelSize: 12
                color: controlBar.statusColor
            }
        }
    }
}
