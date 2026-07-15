/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Preferences > General, with SMPlayer's subtabs: General / Video /
    Audio / Preferred audio and subtitles. Items without a Qt Multimedia
    equivalent are shown disabled.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ColumnLayout {
    id: page

    property PlayerController controller

    readonly property string helpText: qsTr(
        "<h1>General</h1>"
        + "<p>Core playback options, grouped into tabs.</p>"
        + "<p><b>General</b> — remember playback position and per-file track "
        + "choices, and where screenshots are saved (folder and image format).</p>"
        + "<p><b>Video</b> — the video output; the FFmpeg backend decodes in "
        + "software (there are no mplayer/mpv video filters).</p>"
        + "<p><b>Audio</b> — the output device (from the system's devices), the "
        + "startup/remembered volume and the volume step.</p>"
        + "<p><b>Preferred audio and subtitles</b> — the languages Vivace "
        + "auto-selects among a file's embedded tracks, and whether subtitles "
        + "are shown by default.</p>")

    spacing: 8

    TabBar {
        id: tabs
        Layout.fillWidth: true
        TabButton { text: qsTr("General") }
        TabButton { text: qsTr("Video") }
        TabButton { text: qsTr("Audio") }
        TabButton { text: qsTr("Preferred audio and subtitles") }
    }

    StackLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tabs.currentIndex

        // ------------------------------------------------- General
        ScrollView {
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 10

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Multimedia engine:") }
                    TextField {
                        Layout.fillWidth: true
                        readOnly: true
                        enabled: false
                        text: qsTr("Qt Multimedia (FFmpeg)")
                    }
                }

                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Media settings")

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        RowLayout {
                            spacing: 6
                            CheckBox {
                                text: qsTr("Remember settings for all files (audio and subtitle tracks)")
                                checked: Settings.rememberFileSettings
                                onToggled: Settings.rememberFileSettings = checked
                            }
                            HelpMark {
                                text: qsTr("Store each file's chosen audio and "
                                           + "subtitle track, and restore them the "
                                           + "next time that file is opened.")
                            }
                            Item { Layout.fillWidth: true }
                        }
                        CheckBox {
                            text: qsTr("Remember time position of files")
                            checked: Settings.resumePlayback
                            onToggled: Settings.resumePlayback = checked
                        }
                        CheckBox {
                            text: qsTr("Close the main window when the playlist finishes")
                            checked: Settings.closeOnFinish
                            onToggled: Settings.closeOnFinish = checked
                        }
                        CheckBox {
                            text: qsTr("Pause when the window is minimized")
                            checked: Settings.pauseWhenMinimized
                            onToggled: Settings.pauseWhenMinimized = checked
                        }
                        RowLayout {
                            spacing: 6
                            CheckBox {
                                text: qsTr("Disable screensaver while playing video")
                                checked: Settings.disableScreensaver
                                onToggled: Settings.disableScreensaver = checked
                            }
                            HelpMark {
                                text: qsTr("Keep the screen awake only while a video "
                                           + "is actually playing; audio-only playback "
                                           + "and paused/stopped states are unaffected.")
                            }
                            Item { Layout.fillWidth: true }
                        }
                    }
                }

                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Screenshots")

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        RowLayout {
                            spacing: 8
                            Label { text: qsTr("Folder:") }
                            TextField {
                                Layout.fillWidth: true
                                text: Settings.screenshotFolder
                                onEditingFinished: Settings.screenshotFolder = text
                            }
                            Button {
                                text: qsTr("Browse…")
                                onClicked: screenshotFolderDialog.open()
                            }
                        }
                        RowLayout {
                            spacing: 8
                            Label { text: qsTr("Image format:") }
                            ComboBox {
                                implicitWidth: 100
                                model: ["png", "jpg"]
                                currentIndex: Settings.screenshotFormat === "jpg" ? 1 : 0
                                onActivated: Settings.screenshotFormat = currentText
                            }
                            Item { Layout.fillWidth: true }
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        // --------------------------------------------------- Video
        ScrollView {
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 10

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Output driver:") }
                    ComboBox {
                        Layout.fillWidth: true
                        enabled: false
                        model: [qsTr("Auto (Qt RHI)")]
                    }
                }

                CheckBox {
                    text: qsTr("Start videos in fullscreen")
                    checked: Settings.startInFullscreen
                    onToggled: Settings.startInFullscreen = checked
                }

                CheckBox {
                    text: qsTr("Use software video equalizer")
                    enabled: false
                }
                RowLayout {
                    spacing: 8
                    enabled: false
                    Label { text: qsTr("Deinterlace by default:") }
                    ComboBox {
                        Layout.fillWidth: true
                        model: [qsTr("None (not supported by the backend)")]
                    }
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                    font.pixelSize: 12
                    text: qsTr("Video equalizer and zoom/aspect controls are planned for Phase 4 (ShaderEffect / item transforms). Deinterlacing and driver selection are not available with Qt Multimedia.")
                }

                Item { Layout.fillHeight: true }
            }
        }

        // --------------------------------------------------- Audio
        ScrollView {
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 10

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Output device:") }
                    ComboBox {
                        id: audioDeviceBox
                        Layout.fillWidth: true
                        model: page.controller ? page.controller.audioDevices : []
                        textRole: "description"
                        valueRole: "id"
                        onActivated: Settings.audioDevice = currentValue
                        Component.onCompleted:
                            currentIndex = Math.max(0, indexOfValue(Settings.audioDevice))
                        Connections {
                            target: page.controller
                            function onAudioDevicesChanged() {
                                audioDeviceBox.currentIndex = Math.max(
                                    0, audioDeviceBox.indexOfValue(Settings.audioDevice))
                            }
                        }
                    }
                }

                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Volume")

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        RowLayout {
                            spacing: 8
                            Label { text: qsTr("Volume step:") }
                            HelpMark {
                                text: qsTr("How much each volume up/down key press "
                                           + "or mouse-wheel notch changes the level.")
                            }
                            SpinBox {
                                from: 1; to: 25
                                value: Settings.volumeStep
                                onValueModified: Settings.volumeStep = value
                            }
                            Label { text: qsTr("%") }
                            Item { Layout.fillWidth: true }
                        }
                        CheckBox {
                            id: rememberVolumeBox
                            text: qsTr("Remember volume between sessions")
                            checked: Settings.rememberVolume
                            onToggled: Settings.rememberVolume = checked
                        }
                        RowLayout {
                            spacing: 8
                            enabled: !rememberVolumeBox.checked
                            Label { text: qsTr("Initial volume:") }
                            HelpMark {
                                text: qsTr("The volume level set at startup when the "
                                           + "volume is not remembered between sessions.")
                            }
                            SpinBox {
                                from: 0; to: 100
                                value: Settings.initialVolume
                                onValueModified: Settings.initialVolume = value
                            }
                            Label { text: qsTr("%") }
                            Item { Layout.fillWidth: true }
                        }
                    }
                }

                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Synchronization")

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        RowLayout {
                            spacing: 8
                            Label { text: qsTr("Global audio delay:") }
                            SpinBox {
                                id: globalDelaySpin
                                from: -10000; to: 0; stepSize: 100
                                editable: true
                                // Shows/edits the value for the active device.
                                onValueModified: Settings.setAudioDelayForDevice(
                                        page.controller.currentAudioDeviceId, value)
                                Binding on value {
                                    value: Settings.audioDeviceDelaysRevision >= 0
                                           ? Settings.audioDelayForDevice(
                                                 page.controller.currentAudioDeviceId)
                                           : 0
                                    when: !globalDelaySpin.activeFocus
                                }
                            }
                            Label { text: qsTr("ms") }
                            Item { Layout.fillWidth: true }
                        }
                        Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            opacity: 0.7
                            font.pixelSize: 12
                            text: qsTr("Delay for the current audio device — “%1” — "
                                       + "applied to every file (e.g. to compensate "
                                       + "Bluetooth output latency). Remembered per "
                                       + "device, so switching devices restores the "
                                       + "right value. A negative value holds the "
                                       + "video back to match late audio. Each file "
                                       + "can add its own delay via Audio > Set delay.")
                                  .arg(page.controller
                                       ? page.controller.currentAudioDeviceDescription
                                       : "")
                        }
                    }
                }

                CheckBox { text: qsTr("Use software volume control"); enabled: false }
                RowLayout {
                    spacing: 8
                    enabled: false
                    Label { text: qsTr("Max. amplification:") }
                    SpinBox { from: 100; to: 400; value: 100 }
                    Label { text: qsTr("%") }
                    Item { Layout.fillWidth: true }
                }
                CheckBox { text: qsTr("Volume normalization by default"); enabled: false }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                    font.pixelSize: 12
                    text: qsTr("Amplification above 100% and volume normalization require an audio pipeline filter, which Qt Multimedia does not provide.")
                }

                Item { Layout.fillHeight: true }
            }
        }

        // ------------------------- Preferred audio and subtitles
        ScrollView {
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 10

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: qsTr("Comma-separated language codes or names in order of preference (e.g. \"ja, en\").")
                }

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Preferred audio language:") }
                    TextField {
                        Layout.fillWidth: true
                        placeholderText: qsTr("e.g. ja, en")
                        text: Settings.preferredAudioLanguage
                        onEditingFinished: Settings.preferredAudioLanguage = text
                    }
                }
                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Preferred subtitle language:") }
                    TextField {
                        Layout.fillWidth: true
                        placeholderText: qsTr("e.g. ja, en")
                        text: Settings.preferredSubtitleLanguage
                        onEditingFinished: Settings.preferredSubtitleLanguage = text
                    }
                }
                RowLayout {
                    spacing: 6
                    CheckBox {
                        text: qsTr("Show subtitles by default")
                        checked: Settings.subtitlesByDefault
                        onToggled: Settings.subtitlesByDefault = checked
                    }
                    HelpMark {
                        text: qsTr("When a file has embedded subtitles, turn one on "
                                   + "automatically (preferring the languages above) "
                                   + "instead of starting with subtitles off.")
                    }
                    Item { Layout.fillWidth: true }
                }

                RowLayout {
                    spacing: 8
                    enabled: false
                    Label { text: qsTr("Preferred audio track number:") }
                    SpinBox { from: 1; to: 25 }
                }
                RowLayout {
                    spacing: 8
                    enabled: false
                    Label { text: qsTr("Preferred subtitle track number:") }
                    SpinBox { from: 1; to: 25 }
                }

                Item { Layout.fillHeight: true }
            }
        }
    }

    FolderDialog {
        id: screenshotFolderDialog
        onAccepted: Settings.screenshotFolder = UiHelpers.toLocalPath(selectedFolder)
    }
}
