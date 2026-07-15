/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    OpenSubtitles search dialog (SMPlayer's Find Subtitles). Searches by the
    current file's movie hash and/or a text query, lists the matches, and
    downloads the chosen one next to the video, then loads it. Needs a
    user-supplied API key (Preferences > Network); the REST API requires one.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: dlg

    required property PlayerController controller

    title: qsTr("Find subtitles — Vivace")
    flags: Qt.Dialog
    width: 640
    height: 480
    minimumWidth: 480
    minimumHeight: 360
    color: palette.window

    readonly property url source: controller.player.source
    readonly property string videoPath: source.toString().startsWith("file:")
                                        ? UiHelpers.toLocalPath(source) : ""

    function openDialog() {
        osc.clearResults()
        // Prefill the query with the file name (no extension) as a fallback for
        // when the hash finds nothing.
        const s = source.toString()
        const base = decodeURIComponent(s.substring(s.lastIndexOf('/') + 1))
        const dot = base.lastIndexOf('.')
        queryField.text = dot > 0 ? base.substring(0, dot) : base
        langField.text = Settings.preferredSubtitleLanguage
        show()
        raise()
        requestActivate()
    }

    Shortcut { sequences: [StandardKey.Cancel]; onActivated: dlg.close() }

    OpenSubtitlesClient {
        id: osc
        apiKey: Settings.opensubtitlesApiKey
        username: Settings.opensubtitlesUsername
        password: Settings.opensubtitlesPassword
        onSubtitleReady: url => {
            dlg.controller.loadSubtitles(url)
            dlg.close()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Label { text: qsTr("Search:") }
            HelpMark { text: qsTr("Matches are found by the video's content hash "
                                  + "first, then by this text; leave it as the file "
                                  + "name unless the hash finds nothing.") }
            TextField {
                id: queryField
                Layout.fillWidth: true
                placeholderText: qsTr("movie or show name")
                onAccepted: searchButton.clicked()
            }
            Label { text: qsTr("Languages:") }
            HelpMark { text: qsTr("Comma-separated language codes to search for, "
                                  + "for example \"en, ja\".") }
            TextField {
                id: langField
                Layout.preferredWidth: 90
                placeholderText: qsTr("en, ja")
            }
            Button {
                id: searchButton
                text: qsTr("&Search")
                enabled: osc.configured && !osc.busy
                onClicked: osc.search(dlg.videoPath, queryField.text, langField.text)
            }
            HelpMark { text: qsTr("Searching needs a free OpenSubtitles API key, "
                                  + "set in Preferences > Network.") }
        }

        // Prompt to configure the API key when it is missing.
        Label {
            Layout.fillWidth: true
            visible: !osc.configured
            wrapMode: Text.WordWrap
            color: "#b00020"
            text: qsTr("Set your OpenSubtitles API key in Preferences > Network "
                       + "to search. Create a free key at opensubtitles.com "
                       + "(a login there also raises the download limit).")
        }

        Frame {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 1

            ListView {
                id: resultsView
                anchors.fill: parent
                clip: true
                model: osc.results
                currentIndex: -1
                ScrollBar.vertical: ScrollBar {}

                header: Rectangle {
                    width: resultsView.width
                    height: 24
                    color: "#e0e0e0"
                    z: 2
                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 6
                        anchors.rightMargin: 6
                        spacing: 8
                        Label { text: qsTr("Lang"); Layout.preferredWidth: 40; font.bold: true }
                        Label { text: qsTr("Release / file"); Layout.fillWidth: true; font.bold: true }
                        Label { text: qsTr("Downloads"); Layout.preferredWidth: 70; font.bold: true; horizontalAlignment: Text.AlignRight }
                        Label { text: qsTr("Rating"); Layout.preferredWidth: 46; font.bold: true; horizontalAlignment: Text.AlignRight }
                    }
                }

                delegate: ItemDelegate {
                    required property int index
                    required property var modelData
                    width: resultsView.width
                    highlighted: resultsView.currentIndex === index
                    onClicked: resultsView.currentIndex = index
                    onDoubleClicked: { resultsView.currentIndex = index; downloadButton.clicked() }
                    contentItem: RowLayout {
                        spacing: 8
                        Label { text: modelData.language; Layout.preferredWidth: 40 }
                        Label {
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                            text: modelData.release && modelData.release !== ""
                                  ? modelData.release : modelData.fileName
                        }
                        Label { text: modelData.downloads; Layout.preferredWidth: 70; horizontalAlignment: Text.AlignRight }
                        Label { text: (modelData.rating || 0).toFixed(1); Layout.preferredWidth: 46; horizontalAlignment: Text.AlignRight }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Label {
                Layout.fillWidth: true
                elide: Text.ElideRight
                opacity: 0.8
                text: osc.status
            }
            BusyIndicator { running: osc.busy; implicitWidth: 20; implicitHeight: 20 }
            Button {
                id: downloadButton
                text: qsTr("&Download && load")
                enabled: resultsView.currentIndex >= 0 && !osc.busy
                onClicked: osc.download(osc.results[resultsView.currentIndex].fileId,
                                        dlg.videoPath)
            }
            Button {
                text: qsTr("&Close")
                onClicked: dlg.close()
            }
        }
    }
}
