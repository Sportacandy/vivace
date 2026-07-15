/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Preferences window in SMPlayer's layout: sections list on the left,
    pages (some with subtabs) on the right. Sections whose features
    cannot exist without mplayer/mpv (Drives, TV) are placeholders that
    say so; sections whose features are planned (File types, Updates,
    Network, shortcut editor) are mocks until the feature lands.
    SMPlayer's Performance section is intentionally omitted.
    All live controls apply immediately (they write to Settings); Cancel
    reverts to the snapshot taken on open. Help (like SMPlayer) opens a
    window with the current section's context help (each page's helpText).
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: prefsDialog

    required property PlayerController controller

    title: qsTr("Preferences")
    modality: Qt.WindowModal
    flags: Qt.Dialog
    width: 760
    height: 620
    minimumWidth: 560
    minimumHeight: 420
    color: "#f0f0f0"

    // Settings snapshot taken on open / last Apply; Cancel restores it.
    // The pages apply changes instantly, so this is how reverting works.
    property var baseline: ({})
    property string baselineJson: ""
    // Guards onClosing: only revert when the window is dismissed via Cancel,
    // Escape or the window's close button — not via OK/Apply.
    property bool reverting: true

    // Bumped on every settings change so `dirty` re-evaluates; the `>= 0` term
    // (always true) forces QML to actually track it.
    property int settingsRevision: 0
    readonly property bool dirty: settingsRevision >= 0
                                  && JSON.stringify(Settings.snapshot()) !== baselineJson

    Connections {
        target: Settings
        function onChanged() { prefsDialog.settingsRevision++ }
    }

    function setBaseline() {
        baseline = Settings.snapshot()
        baselineJson = JSON.stringify(baseline)
    }

    function open() {
        setBaseline()
        reverting = true
        if (transientParent) {
            x = transientParent.x + (transientParent.width - width) / 2
            y = transientParent.y + (transientParent.height - height) / 2
        }
        visible = true
    }

    function accept() { reverting = false; close() }        // OK: keep changes
    function cancel() { reverting = true; close() }          // revert on close
    function applyChanges() { setBaseline() }                // new baseline

    // Context help for the current section (SMPlayer's per-page help window):
    // each page exposes a `helpText` HTML string.
    function showHelp() {
        const page = pagesStack.children[sections.currentIndex]
        helpWindow.showText(page && page.helpText
                            ? page.helpText
                            : qsTr("<h1>Help</h1><p>No help is available for "
                                   + "this section.</p>"))
    }

    onClosing: {
        if (reverting)
            Settings.restore(baseline)
        reverting = true // reset for the next open / window-button close
    }

    Shortcut {
        sequence: "Escape"
        onActivated: prefsDialog.cancel()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            Frame {
                Layout.preferredWidth: 170
                Layout.fillHeight: true
                padding: 1
                // Fill the whole pane (incl. the empty space below the items)
                // with the list/item-view background, not the grey frame colour.
                background: Rectangle { color: palette.base }

                ListView {
                    id: sections
                    anchors.fill: parent
                    clip: true
                    currentIndex: 0
                    model: ListModel {
                        ListElement { name: qsTr("General"); iconFile: "pref_general" }
                        ListElement { name: qsTr("Drives"); iconFile: "pref_devices" }
                        ListElement { name: qsTr("Subtitles"); iconFile: "pref_subtitles" }
                        ListElement { name: qsTr("Interface"); iconFile: "pref_gui" }
                        ListElement { name: qsTr("Keyboard and mouse"); iconFile: "mouse" }
                        ListElement { name: qsTr("Playlist"); iconFile: "pref_playlist" }
                        ListElement { name: qsTr("TV and radio"); iconFile: "pref_tv" }
                        ListElement { name: qsTr("File types"); iconFile: "pref_associations" }
                        ListElement { name: qsTr("Updates"); iconFile: "pref_updates" }
                        ListElement { name: qsTr("Network"); iconFile: "pref_network" }
                        ListElement { name: qsTr("Advanced"); iconFile: "pref_advanced" }
                    }

                    ScrollBar.vertical: ScrollBar {}

                    delegate: ItemDelegate {
                        required property int index
                        required property string name
                        required property string iconFile
                        width: ListView.view.width
                        text: name
                        icon.source: Theme.icon(iconFile)
                        icon.width: 22
                        icon.height: 22
                        icon.color: "transparent"
                        highlighted: ListView.isCurrentItem
                        onClicked: sections.currentIndex = index
                    }
                }
            }

            StackLayout {
                id: pagesStack
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: sections.currentIndex

                PrefGeneralPage { controller: prefsDialog.controller }
                PrefDrivesPage {}
                PrefSubtitlesPage {}
                PrefInterfacePage {}
                PrefInputPage {}
                PrefPlaylistPage {}
                PrefTVPage {}
                PrefFileTypesPage {}
                PrefUpdatesPage {}
                PrefNetworkPage {}
                PrefAdvancedPage { controller: prefsDialog.controller }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: qsTr("&Help")
                onClicked: prefsDialog.showHelp()
            }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("OK")
                onClicked: prefsDialog.accept()
            }
            Button {
                text: qsTr("Apply")
                // Only enabled while there are unsaved changes.
                enabled: prefsDialog.dirty
                onClicked: prefsDialog.applyChanges()
            }
            Button {
                text: qsTr("Cancel")
                onClicked: prefsDialog.cancel()
            }
        }
    }

    // Context-help window (SMPlayer's InfoWindow), shown by the Help button.
    Window {
        id: helpWindow
        title: qsTr("Vivace — Help")
        flags: Qt.Dialog
        width: 460
        height: 520
        color: "#f0f0f0"

        function showText(html) {
            helpText.text = html
            if (prefsDialog) {
                x = prefsDialog.x + 40
                y = prefsDialog.y + 40
            }
            visible = true
            raise()
            requestActivate()
        }

        Shortcut { sequence: "Escape"; onActivated: helpWindow.close() }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 8

            ScrollView {
                id: helpScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                ScrollBar.vertical.policy: ScrollBar.AsNeeded
                ScrollBar.horizontal.policy: ScrollBar.AsNeeded
                TextArea {
                    id: helpText
                    // Wrap to the viewport width so long lines never need a
                    // horizontal scrollbar; the vertical one shows as needed.
                    width: helpScroll.availableWidth
                    readOnly: true
                    wrapMode: Text.WordWrap
                    textFormat: Text.RichText
                    background: null
                    onLinkActivated: link => Qt.openUrlExternally(link)
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("Close")
                    onClicked: helpWindow.close()
                }
            }
        }
    }
}
