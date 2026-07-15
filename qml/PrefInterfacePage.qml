/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Preferences > Interface, with SMPlayer's subtabs: Interface / Seeking /
    Instances / Fullscreen / Privacy / High DPI.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ColumnLayout {
    spacing: 8

    readonly property string helpText: qsTr(
        "<h1>Interface</h1>"
        + "<p><b>Interface</b> — the GUI layout (Basic / Mini / Mpc), icon set, "
        + "Qt Quick Controls style and application font, plus main-window "
        + "behaviour (auto-resize, centre, keep on screen, remember geometry, "
        + "hide the video area for audio-only files), the toolbar gradient, "
        + "the native file dialog toggle and OSD options.</p>"
        + "<p><b>Seeking</b> — the jump lengths for the seek buttons and the "
        + "mouse wheel, and whether the time slider seeks while dragging or on "
        + "release.</p>"
        + "<p><b>Instances</b> — reuse a single running instance so files open "
        + "in the existing window (takes effect next launch).</p>"
        + "<p><b>Fullscreen</b> — hide the mouse pointer after inactivity.</p>"
        + "<p><b>Privacy</b> — how many recent files and URLs to remember, and "
        + "whether to remember the last folder.</p>"
        + "<p><b>High DPI</b> — override the interface scale factor (Vivace "
        + "scales automatically otherwise).</p>"
        + "<p>Style, font and scale-factor changes take effect after "
        + "restarting Vivace.</p>")

    FontDialog {
        id: fontDialog
        onAccepted: {
            Settings.uiFontFamily = selectedFont.family
            Settings.uiFontSize = selectedFont.pointSize
        }
    }

    TabBar {
        id: tabs
        Layout.fillWidth: true
        TabButton { text: qsTr("Interface") }
        TabButton { text: qsTr("Seeking") }
        TabButton { text: qsTr("Instances") }
        TabButton { text: qsTr("Fullscreen") }
        TabButton { text: qsTr("Privacy") }
        TabButton { text: qsTr("High DPI") }
    }

    StackLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tabs.currentIndex

        // ----------------------------------------------- Interface
        ScrollView {
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 10

                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Main window")

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        RowLayout {
                            spacing: 8
                            Label { text: qsTr("Auto-resize:") }
                            ComboBox {
                                Layout.fillWidth: true
                                // 0 Never / 1 Whenever needed / 2 After loading
                                model: [
                                    qsTr("Never"),
                                    qsTr("Whenever it's needed"),
                                    qsTr("Only after loading a new video")
                                ]
                                currentIndex: Settings.mainwindowResizeMode
                                onActivated: Settings.mainwindowResizeMode = currentIndex
                            }
                        }
                        CheckBox {
                            text: qsTr("Center window")
                            checked: Settings.centerWindow
                            onToggled: Settings.centerWindow = checked
                        }
                        CheckBox {
                            text: qsTr("Prevent the window from getting outside of the screen")
                            checked: Settings.preventOutsideScreen
                            onToggled: Settings.preventOutsideScreen = checked
                        }
                        CheckBox {
                            text: qsTr("Remember size and position of the main window")
                            checked: Settings.rememberGeometry
                            onToggled: Settings.rememberGeometry = checked
                        }
                        CheckBox {
                            text: qsTr("Hide the video window when playing audio files")
                            checked: Settings.hideVideoOnAudio
                            onToggled: Settings.hideVideoOnAudio = checked
                        }
                    }
                }

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("GUI:") }
                    ComboBox {
                        id: guiCombo
                        Layout.fillWidth: true
                        textRole: "label"
                        valueRole: "value"
                        // Skinnable is listed but disabled: Vivace has no skin
                        // engine yet (SMPlayer likewise disables it with no
                        // skins installed). Applies live, no restart.
                        model: [
                            { label: qsTr("Basic GUI"),     value: "Basic", available: true },
                            { label: qsTr("Mini GUI"),      value: "Mini",  available: true },
                            { label: qsTr("Mpc GUI"),       value: "Mpc",   available: true },
                            { label: qsTr("Skinnable GUI"), value: "Skin",  available: false }
                        ]
                        currentIndex: Settings.gui === "Mini" ? 1
                                      : Settings.gui === "Mpc" ? 2
                                      : Settings.gui === "Skin" ? 3 : 0
                        delegate: ItemDelegate {
                            required property int index
                            required property var modelData
                            width: guiCombo.width
                            text: modelData.label
                            enabled: modelData.available
                            highlighted: guiCombo.highlightedIndex === index
                        }
                        onActivated: {
                            if (model[currentIndex].available)
                                Settings.gui = currentValue
                            else
                                currentIndex = guiCombo.indexOfValue(Settings.gui)
                        }
                    }
                }
                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Icon set:") }
                    ComboBox {
                        Layout.fillWidth: true
                        // Values are the folder names under icons/; labels are
                        // friendlier. Applies live.
                        textRole: "label"
                        valueRole: "value"
                        model: [
                            { label: qsTr("Default"), value: "Default" },
                            { label: qsTr("Classic"), value: "Classic" }
                        ]
                        currentIndex: Settings.iconSet === "Classic" ? 1 : 0
                        onActivated: Settings.iconSet = currentValue
                    }
                }
                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Language:") }
                    ComboBox {
                        id: languageCombo
                        Layout.fillWidth: true
                        textRole: "label"
                        valueRole: "code"
                        // "System default" + English + one row per embedded
                        // vivace_<lang>.qm (native names, from UiHelpers).
                        model: [
                            { label: qsTr("System default"), code: "" },
                            { label: "English", code: "en" }
                        ].concat(UiHelpers.availableUiLanguages())
                        Component.onCompleted:
                            currentIndex = Math.max(0, indexOfValue(Settings.uiLanguage))
                        onActivated: Settings.uiLanguage = currentValue
                    }
                }
                Label {
                    Layout.fillWidth: true
                    Layout.leftMargin: 8
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                    font.pixelSize: 12
                    text: qsTr("Language changes take effect after restarting Vivace. Untranslated text falls back to English.")
                }

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Style:") }
                    ComboBox {
                        id: styleCombo
                        Layout.fillWidth: true
                        model: Settings.availableQtStyles()
                        currentIndex: Math.max(0, model.indexOf(Settings.qtStyle))
                        onActivated: Settings.qtStyle = currentValue
                    }
                }
                Label {
                    Layout.fillWidth: true
                    Layout.leftMargin: 8
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                    font.pixelSize: 12
                    text: qsTr("Style changes take effect after restarting Vivace. Fusion is recommended: other styles may not render the custom menus and sliders correctly.")
                }

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Application font:") }
                    Label {
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        text: Settings.uiFontFamily === ""
                              ? qsTr("System default")
                              : Settings.uiFontFamily
                                + (Settings.uiFontSize > 0
                                   ? " " + Settings.uiFontSize + "pt" : "")
                    }
                    Button {
                        text: qsTr("Change…")
                        onClicked: {
                            if (Settings.uiFontFamily !== "")
                                fontDialog.selectedFont =
                                    Qt.font({ family: Settings.uiFontFamily,
                                              pointSize: Settings.uiFontSize > 0
                                                         ? Settings.uiFontSize : 12 })
                            fontDialog.open()
                        }
                    }
                    Button {
                        text: qsTr("Reset")
                        enabled: Settings.uiFontFamily !== "" || Settings.uiFontSize > 0
                        onClicked: {
                            Settings.uiFontFamily = ""
                            Settings.uiFontSize = 0
                        }
                    }
                }
                Label {
                    Layout.fillWidth: true
                    Layout.leftMargin: 8
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                    font.pixelSize: 12
                    text: qsTr("Font changes take effect after restarting Vivace. (The default font already renders Japanese and other scripts; a custom font only needs to be set for preference.)")
                }

                CheckBox {
                    text: qsTr("Gradient background for the toolbar and control bar")
                    checked: Settings.toolbarGradient
                    onToggled: Settings.toolbarGradient = checked
                }
                CheckBox {
                    text: qsTr("Use the system native file dialog")
                    checked: Settings.useNativeFileDialog
                    onToggled: Settings.useNativeFileDialog = checked
                }
                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("OSD")

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        CheckBox {
                            text: qsTr("Show OSD messages")
                            checked: Settings.osdEnabled
                            onToggled: Settings.osdEnabled = checked
                        }
                        RowLayout {
                            spacing: 8
                            Label { text: qsTr("Duration of OSD messages:") }
                            SpinBox {
                                from: 500; to: 10000; stepSize: 250
                                value: Settings.osdDuration
                                onValueModified: Settings.osdDuration = value
                            }
                            Label { text: qsTr("ms") }
                            Item { Layout.fillWidth: true }
                        }
                        RowLayout {
                            spacing: 8
                            Label { text: qsTr("OSD font size:") }
                            SpinBox {
                                from: 10; to: 60
                                value: Settings.osdFontSize
                                onValueModified: Settings.osdFontSize = value
                            }
                            Label { text: qsTr("px") }
                            Item { Layout.fillWidth: true }
                        }
                    }
                }

                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Touch")

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        RowLayout {
                            spacing: 6
                            CheckBox {
                                text: qsTr("Touch-friendly controls (larger fonts and icons)")
                                checked: Settings.touchMode
                                onToggled: Settings.touchMode = checked
                            }
                            HelpMark {
                                text: qsTr("Enlarges the interface for finger use on a "
                                           + "tablet. Toolbar icons resize immediately; "
                                           + "the larger fonts take effect after you "
                                           + "restart Vivace.")
                            }
                            Item { Layout.fillWidth: true }
                        }
                        RowLayout {
                            spacing: 6
                            CheckBox {
                                text: qsTr("Swipe across the video to seek")
                                checked: Settings.gestureSeek
                                onToggled: Settings.gestureSeek = checked
                            }
                            HelpMark {
                                text: qsTr("Drag left or right on the video (with a "
                                           + "finger or the mouse) to jump backward or "
                                           + "forward. An on-screen message previews "
                                           + "the target while you drag.")
                            }
                            Item { Layout.fillWidth: true }
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        // ----------------------------------------------- Seeking
        ScrollView {
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 10

                // Jump lengths (SMPlayer's Short/Medium/Long/Mouse-wheel
                // SeekWidgets): icon + label + a seconds setter.
                component SeekRow: RowLayout {
                    property alias iconName: rowIcon.iconName
                    property string label
                    property int seconds
                    property int minimum: 1
                    property int maximum: 3600
                    signal edited(int value)
                    spacing: 8
                    Item {
                        implicitWidth: 32; implicitHeight: 32
                        Image {
                            id: rowIcon
                            property string iconName: ""
                            anchors.centerIn: parent
                            source: iconName !== "" ? Theme.icon(iconName) : ""
                            sourceSize: Qt.size(32, 32)
                        }
                    }
                    Label { text: parent.label; Layout.preferredWidth: 140 }
                    SpinBox {
                        from: parent.minimum; to: parent.maximum
                        value: parent.seconds
                        editable: true
                        onValueModified: parent.edited(value)
                    }
                    Label { text: qsTr("seconds") }
                    Item { Layout.fillWidth: true }
                }

                SeekRow {
                    iconName: "forward10s"
                    label: qsTr("Short jump:")
                    seconds: Settings.seekShortStep
                    minimum: 1; maximum: 60
                    onEdited: value => Settings.seekShortStep = value
                }
                SeekRow {
                    iconName: "forward1m"
                    label: qsTr("Medium jump:")
                    seconds: Settings.seekMediumStep
                    minimum: 5; maximum: 600
                    onEdited: value => Settings.seekMediumStep = value
                }
                SeekRow {
                    iconName: "forward10m"
                    label: qsTr("Long jump:")
                    seconds: Settings.seekLongStep
                    minimum: 30; maximum: 3600
                    onEdited: value => Settings.seekLongStep = value
                }
                SeekRow {
                    label: qsTr("Mouse wheel jump:")
                    seconds: Settings.seekWheelStep
                    minimum: 1; maximum: 600
                    onEdited: value => Settings.seekWheelStep = value
                }

                MenuSeparator { Layout.fillWidth: true }

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Behaviour of time slider:") }
                    ComboBox {
                        Layout.fillWidth: true
                        model: [
                            qsTr("Seek to position while dragging"),
                            qsTr("Seek to position when released")
                        ]
                        currentIndex: Settings.seekOnDrag ? 0 : 1
                        onActivated: Settings.seekOnDrag = currentIndex === 0
                    }
                }
                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                    font.pixelSize: 12
                    text: qsTr("Absolute/relative seeking method and precise seeking are mplayer/mpv-specific and do not apply to Vivace's Qt Multimedia backend (seeking is already position-based).")
                }

                Item { Layout.fillHeight: true }
            }
        }

        // ----------------------------------------------- Instances
        ColumnLayout {
            id: instancesTab
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Image {
                    // instance1 when single-instance is on, instance2 when off
                    // (SMPlayer's changeInstanceImages), shown at native size.
                    source: Theme.icon(Settings.singleInstance ? "instance1"
                                                               : "instance2")
                    fillMode: Image.PreserveAspectFit
                }
                CheckBox {
                    text: qsTr("Use only one running instance of Vivace")
                    checked: Settings.singleInstance
                    onToggled: Settings.singleInstance = checked
                }
                Item { Layout.fillWidth: true }
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                opacity: 0.7
                font.pixelSize: 12
                text: qsTr("When enabled, opening a file while Vivace is already running hands it to the existing window instead of starting a new one. Takes effect on the next launch.")
            }

            Item { Layout.fillHeight: true }
        }

        // ---------------------------------------------- Fullscreen
        ColumnLayout {
            spacing: 10

            RowLayout {
                spacing: 6
                CheckBox {
                    id: hideMouseBox
                    text: qsTr("Hide the mouse pointer after inactivity")
                    checked: Settings.hideMouseInFullscreen
                    onToggled: Settings.hideMouseInFullscreen = checked
                }
                HelpMark {
                    text: qsTr("While a video is playing, hide the mouse pointer "
                               + "over the video after the delay below — in the "
                               + "normal window as well as in fullscreen.")
                }
            }
            RowLayout {
                spacing: 8
                enabled: hideMouseBox.checked
                Label { text: qsTr("Hide after:") }
                SpinBox {
                    from: 500; to: 10000; stepSize: 250
                    value: Settings.mouseHideDelay
                    onValueModified: Settings.mouseHideDelay = value
                }
                Label { text: qsTr("ms") }
                Item { Layout.fillWidth: true }
            }
            RowLayout {
                spacing: 8
                enabled: false
                Label { text: qsTr("Time to hide the control bar:") }
                SpinBox { from: 500; to: 5000; value: 3000 }
                Label { text: qsTr("ms") }
                Item { Layout.fillWidth: true }
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                opacity: 0.7
                text: qsTr("The fullscreen control bar currently shows while the pointer is near the bottom edge; a time-based autohide is planned.")
            }

            Item { Layout.fillHeight: true }
        }

        // ----------------------------------------------- Privacy
        ColumnLayout {
            spacing: 10

            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Recent files")

                RowLayout {
                    anchors.fill: parent
                    spacing: 8
                    Label { text: qsTr("Max. items:") }
                    SpinBox {
                        from: 1; to: 50
                        value: Settings.recentsMaxItems
                        onValueModified: Settings.recentsMaxItems = value
                    }
                    Item { Layout.fillWidth: true }
                }
            }
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("URLs")

                RowLayout {
                    anchors.fill: parent
                    spacing: 8
                    Label { text: qsTr("Max. items:") }
                    SpinBox {
                        from: 1; to: 100
                        value: Settings.urlHistoryMax
                        onValueModified: Settings.urlHistoryMax = value
                    }
                    Item { Layout.fillWidth: true }
                }
            }
            CheckBox {
                text: qsTr("Remember last directory")
                checked: Settings.rememberLastDir
                onToggled: Settings.rememberLastDir = checked
            }

            Item { Layout.fillHeight: true }
        }

        // ----------------------------------------------- High DPI
        ColumnLayout {
            spacing: 10

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Vivace scales the interface on high-DPI screens automatically. You can override the scale factor here if needed.")
            }
            RowLayout {
                spacing: 8
                Label { text: qsTr("Scale factor:") }
                ComboBox {
                    id: scaleCombo
                    Layout.fillWidth: true
                    textRole: "label"
                    valueRole: "value"
                    model: [
                        { label: qsTr("Automatic"), value: 0.0 },
                        { label: "1.0", value: 1.0 },
                        { label: "1.25", value: 1.25 },
                        { label: "1.5", value: 1.5 },
                        { label: "1.75", value: 1.75 },
                        { label: "2.0", value: 2.0 }
                    ]
                    currentIndex: Math.max(0, model.findIndex(
                            e => Math.abs(e.value - Settings.interfaceScaleFactor) < 0.001))
                    onActivated: Settings.interfaceScaleFactor = currentValue
                }
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                opacity: 0.7
                font.pixelSize: 12
                text: qsTr("Scale factor changes take effect after restarting Vivace.")
            }

            Item { Layout.fillHeight: true }
        }
    }
}
