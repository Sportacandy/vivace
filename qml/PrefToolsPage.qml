/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Preferences > Tools: Toolbar / Control bar / Status bar. Embeds the same
    editing capability as Options > Toolbars > Edit main toolbar / Edit
    control bar (ToolbarEditor.qml) and Options > Status bar directly in
    Preferences, so they stay reachable even when the menu bar itself is
    hidden (Settings.showMenuBar) — the toolbar's own "preferences" button
    is the one thing guaranteed to always be reachable. Unlike the
    standalone ToolbarEditor.qml dialog (which stages edits and commits on
    OK), every control here applies instantly to Settings, matching every
    other Preferences page's own convention; Preferences' own OK/Apply/
    Cancel still governs reverting via Settings.snapshot()/restore().
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "ToolbarItems.js" as Items

ColumnLayout {
    spacing: 8

    readonly property string helpText: qsTr(
        "<h1>Toolbars</h1>"
        + "<p><b>Toolbar</b> and <b>Control bar</b> let you show/hide each "
        + "bar and choose which actions appear on it, in the same way as "
        + "Options ▸ Toolbars ▸ Edit main toolbar / Edit control bar.</p>"
        + "<p><b>Status bar</b> mirrors Options ▸ Status bar: which info "
        + "fields are shown and how the time is displayed.</p>"
        + "<p>Everything on this page applies immediately, like the rest "
        + "of Preferences.</p>")

    // Shared dual-list editor for one bar's item layout. Every change
    // applies immediately (itemsEdited/iconSizeEdited), no local staging
    // and no OK button — matches this page's own instant-apply convention
    // rather than ToolbarEditor.qml's stage-then-commit one.
    component ItemRow: ItemDelegate {
        required property int index
        required property var modelData
        width: ListView.view.width
        highlighted: ListView.isCurrentItem
        icon.source: Items.iconFor(modelData) !== ""
                     ? Theme.icon(Items.iconFor(modelData)) : ""
        icon.color: "transparent"
        text: Items.labelFor(modelData)
    }

    component BarEditor: ColumnLayout {
        id: editorRoot
        spacing: 8

        required property string barTarget // "main" or "control"
        required property var currentItems
        required property var defaultItems
        required property int iconSize
        // The OTHER bar's own current (effective) item list, so this editor
        // can refuse to strip "preferences" from itself when the other bar
        // doesn't have it either -- Preferences > Toolbars is the only way
        // to reach any hidden feature at all once the menu bar is off (this
        // matters most on Android, but the same lockout risk exists on
        // desktop too), so it must never be removable from BOTH bars. Moot
        // whenever the menu bar itself is shown -- Options > Preferences is
        // then always reachable regardless of what's on either bar.
        property var otherBarItems: []
        signal itemsEdited(var newItems)
        signal iconSizeEdited(int newSize)

        function wouldStrandPreferences(newItems) {
            if (Settings.showMenuBar)
                return false
            return newItems.indexOf("preferences") < 0
                   && editorRoot.otherBarItems.indexOf("preferences") < 0
        }

        // Which item kinds this bar can host: the main toolbar has no
        // sliders (no predefined seek/volume slider instance exists
        // there); the control bar supports every kind.
        function allowsKind(kind) {
            if (barTarget === "main")
                return kind !== "slider"
            return true
        }

        function addItem(id) {
            const at = currentList.currentIndex >= 0
                       ? currentList.currentIndex + 1 : editorRoot.currentItems.length
            const copy = editorRoot.currentItems.slice()
            copy.splice(at, 0, id)
            editorRoot.itemsEdited(copy)
            currentList.currentIndex = at
            availableList.currentIndex = -1
        }
        function removeAt(index) {
            if (index < 0 || index >= editorRoot.currentItems.length)
                return
            const copy = editorRoot.currentItems.slice()
            copy.splice(index, 1)
            if (editorRoot.wouldStrandPreferences(copy)) {
                ToolTip.show(qsTr("Can't remove: Preferences must stay on the "
                                  + "Toolbar or Control bar, so it's always reachable."),
                             3000)
                return
            }
            editorRoot.itemsEdited(copy)
            currentList.currentIndex = -1
        }
        function move(index, delta) {
            const to = index + delta
            if (index < 0 || to < 0 || to >= editorRoot.currentItems.length)
                return
            const copy = editorRoot.currentItems.slice()
            const v = copy[index]
            copy.splice(index, 1)
            copy.splice(to, 0, v)
            editorRoot.itemsEdited(copy)
            currentList.currentIndex = to
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            // Explicit 0 (Android, huge system font sizes) -- see
            // PreferencesDialog.qml's own comment on the equivalent fix.
            Layout.minimumHeight: 0
            spacing: 8

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6
                    Label { text: qsTr("Available"); font.bold: true }
                    HelpMark {
                        text: qsTr("Actions you can add. Double-click one or select it "
                                   + "and press Add → to place it on the bar; "
                                   + "separators and spacers can be reused.")
                    }
                    Item { Layout.fillWidth: true }
                }
                Frame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    padding: 1
                    ListView {
                        id: availableList
                        anchors.fill: parent
                        clip: true
                        currentIndex: -1
                        model: Items.catalog.map(e => e.id).filter(
                                   id => editorRoot.allowsKind(Items.kindFor(id))
                                         && (id === "separator" || id === "spacer"
                                             || editorRoot.currentItems.indexOf(id) < 0))
                        ScrollBar.vertical: ScrollBar {}
                        delegate: ItemRow {
                            onDoubleClicked: editorRoot.addItem(modelData)
                            onClicked: availableList.currentIndex = index
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignVCenter
                spacing: 6
                Button {
                    text: qsTr("Add →")
                    enabled: availableList.currentIndex >= 0
                    onClicked: editorRoot.addItem(
                                   availableList.model[availableList.currentIndex])
                }
                Button {
                    text: qsTr("← Remove")
                    enabled: currentList.currentIndex >= 0
                    onClicked: editorRoot.removeAt(currentList.currentIndex)
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Label {
                    text: editorRoot.barTarget === "main" ? qsTr("Toolbar")
                                                          : qsTr("Control bar")
                    font.bold: true
                }
                Frame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    padding: 1
                    ListView {
                        id: currentList
                        anchors.fill: parent
                        clip: true
                        currentIndex: -1
                        model: editorRoot.currentItems
                        ScrollBar.vertical: ScrollBar {}
                        delegate: ItemRow {
                            onDoubleClicked: editorRoot.removeAt(index)
                            onClicked: currentList.currentIndex = index
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignVCenter
                spacing: 6
                Button {
                    text: qsTr("Up")
                    enabled: currentList.currentIndex > 0
                    onClicked: editorRoot.move(currentList.currentIndex, -1)
                }
                Button {
                    text: qsTr("Down")
                    enabled: currentList.currentIndex >= 0
                             && currentList.currentIndex < editorRoot.currentItems.length - 1
                    onClicked: editorRoot.move(currentList.currentIndex, 1)
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Label { text: qsTr("Icon size:") }
            SpinBox {
                id: iconSizeSpin
                from: 16; to: 48; stepSize: 2
                editable: true
                value: editorRoot.iconSize
                onValueModified: editorRoot.iconSizeEdited(value)
            }
            Button {
                text: qsTr("Restore defaults")
                onClicked: {
                    const defaults = editorRoot.defaultItems.slice()
                    if (editorRoot.wouldStrandPreferences(defaults)) {
                        ToolTip.show(qsTr("Can't restore defaults: Preferences must stay "
                                          + "on the Toolbar or Control bar, so it's always "
                                          + "reachable."), 3000)
                        return
                    }
                    editorRoot.itemsEdited(defaults)
                    editorRoot.iconSizeEdited(24)
                    currentList.currentIndex = -1
                }
            }
            Item { Layout.fillWidth: true }
        }
    }

    TabBar {
        id: tabs
        Layout.fillWidth: true
        TabButton { text: qsTr("Toolbar") }
        TabButton { text: qsTr("Control bar") }
        TabButton { text: qsTr("Status bar") }
    }

    StackLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 0
        currentIndex: tabs.currentIndex

        // ------------------------------------------------------- Toolbar
        // A plain fill-height layout, not a ScrollView: the whole point of
        // BarEditor is to grow into whatever vertical space Preferences
        // actually has (like the standalone ToolbarEditor.qml dialog does)
        // -- a ScrollView's content sizes to its own natural height, so
        // anything inside it has nothing real to "fill" and just collapses
        // to a small fixed size, leaving a big blank gap below. The two
        // ListViews already scroll internally if the catalog/bar list is
        // longer than the available height.
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 0
            spacing: 10

            RowLayout {
                spacing: 6
                CheckBox {
                    text: qsTr("Show toolbar")
                    checked: Settings.showToolbar
                    onToggled: Settings.showToolbar = checked
                }
                HelpMark {
                    text: qsTr("The toolbar sits below the menu bar.")
                }
            }
            GroupBox {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 0
                title: qsTr("Toolbar items")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        opacity: 0.8
                        font.pixelSize: 12
                        text: qsTr("Choose which actions appear on Toolbar, and in what order.")
                    }
                    BarEditor {
                        id: mainBarEditor
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: 0
                        barTarget: "main"
                        currentItems: Settings.mainToolbarItems.length > 0
                                      ? Settings.mainToolbarItems : Items.defaultMainToolbar
                        defaultItems: Items.defaultMainToolbar
                        iconSize: Settings.mainToolbarIconSize
                        otherBarItems: controlBarEditor.currentItems
                        onItemsEdited: newItems => Settings.mainToolbarItems = newItems
                        onIconSizeEdited: newSize => Settings.mainToolbarIconSize = newSize
                    }
                }
            }
        }

        // ---------------------------------------------------- Control bar
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 0
            spacing: 10

            RowLayout {
                spacing: 6
                CheckBox {
                    text: qsTr("Show control bar")
                    checked: Settings.showControlBar
                    onToggled: Settings.showControlBar = checked
                }
                HelpMark {
                    text: qsTr("The control bar sits at the bottom of the window, with "
                               + "the seek slider and playback buttons.")
                }
            }
            GroupBox {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: 0
                title: qsTr("Control bar items")

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8

                    Label {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        opacity: 0.8
                        font.pixelSize: 12
                        text: qsTr("Choose which actions appear on Control bar, and in what order.")
                    }
                    BarEditor {
                        id: controlBarEditor
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: 0
                        barTarget: "control"
                        currentItems: Settings.controlBarItems.length > 0
                                      ? Settings.controlBarItems : Items.defaultControlBar
                        defaultItems: Items.defaultControlBar
                        iconSize: Settings.controlBarIconSize
                        otherBarItems: mainBarEditor.currentItems
                        onItemsEdited: newItems => Settings.controlBarItems = newItems
                        onIconSizeEdited: newSize => Settings.controlBarIconSize = newSize
                    }
                }
            }
        }

        // ------------------------------------------------------ Status bar
        ScrollView {
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 10

                RowLayout {
                    spacing: 6
                    CheckBox {
                        text: qsTr("Show status bar")
                        checked: Settings.showStatusBar
                        onToggled: Settings.showStatusBar = checked
                    }
                    HelpMark {
                        text: qsTr("The status bar sits below the control bar and shows "
                                   + "information about the currently playing file.")
                    }
                }
                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Status bar fields")

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8

                        Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            opacity: 0.8
                            font.pixelSize: 12
                            text: qsTr("Choose which fields are shown and how the time "
                                       + "is displayed.")
                        }
                        CheckBox {
                            text: qsTr("Video info")
                            enabled: Settings.showStatusBar
                            checked: Settings.statusVideoInfo
                            onToggled: Settings.statusVideoInfo = checked
                        }
                        CheckBox {
                            text: qsTr("Audio info")
                            enabled: Settings.showStatusBar
                            checked: Settings.statusAudioInfo
                            onToggled: Settings.statusAudioInfo = checked
                        }
                        CheckBox {
                            text: qsTr("Format info")
                            enabled: Settings.showStatusBar
                            checked: Settings.statusFormatInfo
                            onToggled: Settings.statusFormatInfo = checked
                        }
                        CheckBox {
                            text: qsTr("Bitrate info")
                            enabled: Settings.showStatusBar
                            checked: Settings.statusBitrateInfo
                            onToggled: Settings.statusBitrateInfo = checked
                        }
                        CheckBox {
                            text: qsTr("Frame counter")
                            enabled: Settings.showStatusBar
                            checked: Settings.statusFrameCounter
                            onToggled: Settings.statusFrameCounter = checked
                        }
                        GroupBox {
                            Layout.fillWidth: true
                            title: qsTr("Time display")

                            RowLayout {
                                anchors.fill: parent
                                spacing: 12
                                ButtonGroup { id: timeDisplayGroup }
                                RadioButton {
                                    text: qsTr("Display total time")
                                    ButtonGroup.group: timeDisplayGroup
                                    checked: !Settings.timeDisplayRemaining
                                    onToggled: Settings.timeDisplayRemaining = false
                                }
                                RadioButton {
                                    text: qsTr("Display remaining time")
                                    ButtonGroup.group: timeDisplayGroup
                                    checked: Settings.timeDisplayRemaining
                                    onToggled: Settings.timeDisplayRemaining = true
                                }
                            }
                        }
                        CheckBox {
                            text: qsTr("Show the current time with milliseconds")
                            checked: Settings.showMilliseconds
                            onToggled: Settings.showMilliseconds = checked
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }
    }
}
