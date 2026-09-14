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
    required property YoutubeSupportDialog youtubeInstallDialog

    title: qsTr("Preferences")
    modality: Qt.WindowModal
    flags: Qt.Dialog
    // Android (2026-09-07, user report: most dialogs on Android only show
    // a small area of content near the center) -- Qt's Android backend
    // does not give a secondary top-level Window real, independent screen
    // geometry the way desktop does, so a fixed desktop-sized width/
    // height renders into the wrong-sized/positioned area. Fill the
    // transientParent's own bounds EXACTLY (no guessed margin subtracted
    // -- see the REAL fix below, using SafeArea instead) instead of a
    // fixed desktop size.
    width: Qt.platform.os === "android" && transientParent
           ? transientParent.width : 760
    height: Qt.platform.os === "android" && transientParent
            ? transientParent.height : 620
    minimumWidth: Qt.platform.os === "android" ? 0 : 560
    minimumHeight: Qt.platform.os === "android" ? 0 : 420
    color: palette.window

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
        if (Qt.platform.os === "android") {
            // Fill the transientParent's own bounds exactly -- no
            // centering math needed since width/height already match it.
            x = 0
            y = 0
        } else if (transientParent) {
            x = transientParent.x + (transientParent.width - width) / 2
            y = transientParent.y + (transientParent.height - height) / 2
        }
        visible = true
        // Without this, the OS can leave the main window "active" even
        // though this dialog is what's visually shown, so the main
        // window's own Qt.WindowShortcut-scoped shortcuts (e.g. volume
        // Up/Down) keep firing instead of reaching this dialog.
        raise()
        requestActivate()
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
        // REAL fix (2026-09-08, user: "Height must be calculated from the
        // actual environmental values", correctly rejecting the earlier
        // guessed-pixel-constant approach): SafeArea (QtQuick, Qt 6.9+) is
        // Qt's own cross-platform mechanism reporting the ACTUAL platform-
        // determined unsafe-area inset (status bar / navigation bar / notch)
        // at runtime, rather than a hardcoded number that can't possibly be
        // correct across different devices/OS versions/orientations. On
        // desktop this reports 0 on every side, so it's a no-op there --
        // no platform check needed. The window itself now fills the parent
        // EXACTLY (see width/height above); only the CONTENT is inset from
        // the unsafe edges, so there's no gap where the background behind
        // this window could ever show through.
        anchors.topMargin: 12 + SafeArea.margins.top
        anchors.leftMargin: 12 + SafeArea.margins.left
        anchors.rightMargin: 12 + SafeArea.margins.right
        anchors.bottomMargin: 12 + SafeArea.margins.bottom
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            // Explicit 0 (Android, huge system font sizes): a Layout.
            // fillHeight item's minimum otherwise defaults to its own
            // implicitHeight -- and pagesStack's (a StackLayout) implicit
            // height is the MAX of all 11 pages' own implicit heights, one
            // of which can genuinely exceed the whole dialog's available
            // height there. Without this, that forces the WHOLE
            // ColumnLayout's total content taller than the window itself,
            // pushing the Help/OK/Apply/Cancel row below the bottom edge
            // entirely -- rather than just being clipped/needing a
            // scrollbar within its own space. With this, the row instead
            // shrinks to whatever's actually left after the button row
            // below reserves its own height, and each page's own internal
            // ScrollView (added earlier) handles the rest.
            Layout.minimumHeight: 0
            spacing: 12

            Frame {
                Layout.preferredWidth: 170
                Layout.fillHeight: true
                // Same reasoning as pagesStack's own Layout.minimumHeight
                // override just below -- this Frame's default minimum
                // (its own implicitHeight) could otherwise also force the
                // outer RowLayout back past its already-compressed
                // allocation. The ListView inside is already clip: true
                // and has no real content of its own to lose by shrinking.
                Layout.minimumHeight: 0
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
                        ListElement { name: qsTr("Toolbars"); iconFile: "toolbar" }
                        ListElement { name: qsTr("Keyboard and mouse"); iconFile: "mouse" }
                        ListElement { name: qsTr("Playlist"); iconFile: "pref_playlist" }
                        ListElement { name: qsTr("TV and radio"); iconFile: "pref_tv" }
                        ListElement { name: qsTr("File types"); iconFile: "pref_associations" }
                        ListElement { name: qsTr("Updates"); iconFile: "pref_updates" }
                        ListElement { name: qsTr("Network"); iconFile: "pref_network" }
                        ListElement { name: qsTr("Advanced"); iconFile: "pref_advanced" }
                    }

                    // Plain default (AsNeeded) policy: shows only when the
                    // 11 sections genuinely don't fit the dialog's own
                    // height (e.g. Android's larger system font on a
                    // shorter screen), hidden otherwise -- correct on both
                    // desktop and a tall-enough Android screen alike, with
                    // no platform check needed. Horizontal scrolling is
                    // never needed, since each delegate's own width already
                    // matches the list's fixed column width.
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
                // REAL FIX (Android, huge system font sizes): the outer
                // RowLayout's own Layout.minimumHeight: 0 (above) only
                // controls how much space the OUTER ColumnLayout gives
                // that RowLayout -- it says nothing about pagesStack's OWN
                // minimum as seen by ITS parent (this RowLayout). Without
                // this, pagesStack's default minimum (its implicitHeight,
                // the tallest of all 11 pages) could still force the
                // RowLayout to grow back past the compressed size the
                // outer ColumnLayout already allocated for it -- which is
                // exactly what caused the button row to render
                // overlapping the content instead of cleanly below it,
                // even with clip: true already in place (clip only hides
                // overflow, it doesn't fix the sizing conflict that
                // caused this element to disagree with its parent about
                // how tall it's allowed to be in the first place).
                Layout.minimumHeight: 0
                clip: true
                currentIndex: sections.currentIndex

                PrefGeneralPage { controller: prefsDialog.controller }
                PrefDrivesPage {}
                PrefSubtitlesPage {}
                PrefInterfacePage {}
                PrefToolsPage {}
                PrefInputPage {}
                PrefPlaylistPage {}
                PrefTVPage {}
                PrefFileTypesPage {}
                PrefUpdatesPage {}
                PrefNetworkPage {
                    youtubeInstallDialog: prefsDialog.youtubeInstallDialog
                }
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
        // Android: fill prefsDialog's own bounds instead of a fixed
        // desktop size (see prefsDialog's own width/height comment above
        // for why -- this nested window needs the identical treatment,
        // confirmed broken 2026-09-07: "shown as too small window").
        // prefsDialog now fills its own transientParent EXACTLY (no
        // guessed-constant subtraction -- see SafeArea-based content
        // margins below instead), so simply matching its height here is
        // correct with no further adjustment.
        width: Qt.platform.os === "android" ? prefsDialog.width : 460
        height: Qt.platform.os === "android" ? prefsDialog.height : 520
        color: palette.window

        function showText(html) {
            helpText.text = html
            if (Qt.platform.os === "android") {
                x = 0
                y = 0
            } else if (prefsDialog) {
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
            // Real platform-reported inset, not a guessed constant -- see
            // prefsDialog's own outer ColumnLayout comment above for why.
            anchors.topMargin: 8 + SafeArea.margins.top
            anchors.leftMargin: 8 + SafeArea.margins.left
            anchors.rightMargin: 8 + SafeArea.margins.right
            anchors.bottomMargin: 8 + SafeArea.margins.bottom
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
