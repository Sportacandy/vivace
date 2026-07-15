/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Preferences > Playlist, with SMPlayer's subtabs: Playlist / Misc.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    spacing: 8

    readonly property string helpText: qsTr(
        "<h1>Playlist</h1>"
        + "<p><b>Playlist</b> tab — playback options (play files from start, "
        + "start playing after loading a playlist, play the next file "
        + "automatically, ignore playback errors, repeat, shuffle); how opening "
        + "one file adds its folder siblings (the \"Add files from folder\" "
        + "mode: none, video, audio, both or consecutive; recursive folders); "
        + "whether to show each item's title instead of its file name, and "
        + "whether the playlist is a docked panel or a separate window.</p>"
        + "<p><b>Misc</b> tab — auto-sort the list, case-sensitive search, and "
        + "save a copy of the playlist on exit.</p>")

    TabBar {
        id: tabs
        Layout.fillWidth: true
        TabButton { text: qsTr("Playlist") }
        TabButton { text: qsTr("Misc") }
    }

    StackLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tabs.currentIndex

        // ----------------------------------------------- Playlist
        ScrollView {
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 10

                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Playback")

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        RowLayout {
                            spacing: 6
                            CheckBox {
                                text: qsTr("Play files from start")
                                checked: Settings.playFilesFromStart
                                onToggled: Settings.playFilesFromStart = checked
                            }
                            HelpMark {
                                text: qsTr("Always begin at the beginning, ignoring any "
                                           + "remembered resume position for the file.")
                            }
                            Item { Layout.fillWidth: true }
                        }
                        CheckBox {
                            text: qsTr("Start playback after loading a playlist")
                            checked: Settings.playOnLoadPlaylist
                            onToggled: Settings.playOnLoadPlaylist = checked
                        }
                        CheckBox {
                            text: qsTr("Play next file automatically")
                            checked: Settings.autoPlayNext
                            onToggled: Settings.autoPlayNext = checked
                        }
                        RowLayout {
                            spacing: 6
                            CheckBox {
                                text: qsTr("Ignore playback errors")
                                checked: Settings.ignorePlaybackErrors
                                onToggled: Settings.ignorePlaybackErrors = checked
                            }
                            HelpMark {
                                text: qsTr("When a file fails to play, skip on to the "
                                           + "next item instead of stopping the playlist.")
                            }
                            Item { Layout.fillWidth: true }
                        }
                        CheckBox {
                            text: qsTr("Repeat playlist")
                            checked: Settings.playlistRepeat
                            onToggled: Settings.playlistRepeat = checked
                        }
                        CheckBox {
                            text: qsTr("Shuffle")
                            checked: Settings.playlistShuffle
                            onToggled: Settings.playlistShuffle = checked
                        }
                    }
                }

                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Adding files")

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        RowLayout {
                            spacing: 6
                            CheckBox {
                                id: autoAddCheck
                                text: qsTr("Add files to the playlist automatically")
                                checked: Settings.autoAddFolderFiles
                                onToggled: Settings.autoAddFolderFiles = checked
                            }
                            HelpMark {
                                text: qsTr("When you open a single file, also add the "
                                           + "other media files from its folder to the "
                                           + "playlist.")
                            }
                            Item { Layout.fillWidth: true }
                        }
                        RowLayout {
                            spacing: 8
                            enabled: autoAddCheck.checked
                            Label { text: qsTr("Add files from folder:") }
                            HelpMark {
                                text: qsTr("Which sibling files to add: none, only "
                                           + "video, only audio, both, or just the "
                                           + "files consecutive to the opened one "
                                           + "(matching its name pattern).")
                            }
                            ComboBox {
                                Layout.fillWidth: true
                                // 0 none / 1 video / 2 audio / 3 both / 4 consecutive
                                model: [
                                    qsTr("None"),
                                    qsTr("Video files"),
                                    qsTr("Audio files"),
                                    qsTr("Video and audio files"),
                                    qsTr("Consecutive files")
                                ]
                                currentIndex: Settings.mediaToAdd
                                onActivated: Settings.mediaToAdd = currentIndex
                            }
                        }
                        CheckBox {
                            text: qsTr("Add files in directories recursively")
                            checked: Settings.addDirectoriesRecursively
                            onToggled: Settings.addDirectoriesRecursively = checked
                        }
                        CheckBox {
                            // Async metadata probing for every added file is a
                            // separate feature; left off for now.
                            text: qsTr("Get info automatically about files added (slow)")
                            enabled: false
                        }
                    }
                }

                RowLayout {
                    spacing: 6
                    CheckBox {
                        text: qsTr("Display title name instead of filename")
                        checked: Settings.displayTitleName
                        onToggled: Settings.displayTitleName = checked
                    }
                    HelpMark {
                        text: qsTr("Show each entry's stored title (from the playlist "
                                   + "or media metadata) rather than its file name.")
                    }
                    Item { Layout.fillWidth: true }
                }
                RowLayout {
                    spacing: 8
                    // Vivace's playlist can't dock/undock interactively, so a
                    // clear "where does it appear" selector is used instead of
                    // SMPlayer's "dockable" checkbox.
                    Label { text: qsTr("Playlist style:") }
                    ComboBox {
                        implicitWidth: 200
                        model: [qsTr("Docked panel"), qsTr("Separate window")]
                        currentIndex: Settings.playlistAsWindow ? 1 : 0
                        onActivated: index => Settings.playlistAsWindow = index === 1
                    }
                }

                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Sessions")

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        CheckBox {
                            text: qsTr("Remember the playlist between sessions")
                            checked: Settings.restorePlaylist
                            onToggled: Settings.restorePlaylist = checked
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        // ----------------------------------------------- Misc
        ColumnLayout {
            spacing: 10

            RowLayout {
                spacing: 6
                CheckBox {
                    text: qsTr("Auto sort")
                    checked: Settings.playlistAutoSort
                    onToggled: Settings.playlistAutoSort = checked
                }
                HelpMark {
                    text: qsTr("Keep the playlist sorted by name as items are added, "
                               + "rather than in the order they were added.")
                }
                Item { Layout.fillWidth: true }
            }
            CheckBox {
                text: qsTr("Case sensitive search")
                checked: Settings.caseSensitiveSearch
                onToggled: Settings.caseSensitiveSearch = checked
            }
            CheckBox {
                text: qsTr("Save a copy of the playlist on exit")
                checked: Settings.autosavePlaylistOnExit
                onToggled: Settings.autosavePlaylistOnExit = checked
            }

            Item { Layout.fillHeight: true }
        }
    }
}
