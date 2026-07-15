/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Toolbar editor (SMPlayer's ToolbarEditor): move actions between an
    "available" list and the bar's "current" list, reorder and restore
    defaults. Emits accepted(list) with the new item-id ordering.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "ToolbarItems.js" as Items

Window {
    id: dialog

    property var defaultItems: []
    property string target: "" // which bar is being edited (caller-defined)
    property int defaultIconSize: 24
    // Working copy of the current layout (JS array of ids). Reassigned
    // wholesale after each edit so the ListView refreshes.
    property var work: []

    signal accepted(string target, var items, int iconSize)

    title: qsTr("Toolbar editor")
    flags: Qt.Dialog
    modality: Qt.WindowModal
    width: 620
    height: 460
    minimumWidth: 520
    minimumHeight: 360
    color: "#f0f0f0"

    function openFor(titleText, targetKey, currentItems, defaults, iconSize) {
        title = titleText
        target = targetKey
        defaultItems = defaults
        iconSizeSpin.value = iconSize
        work = (currentItems && currentItems.length > 0 ? currentItems : defaults).slice()
        availableList.currentIndex = -1
        currentList.currentIndex = -1
        if (transientParent) {
            x = transientParent.x + (transientParent.width - width) / 2
            y = transientParent.y + (transientParent.height - height) / 2
        }
        visible = true
    }

    // Which item kinds each bar can host: the main toolbar has no sliders,
    // the control bar has no popup (track) menus.
    function allowsKind(kind) {
        if (target === "main") return kind !== "slider"
        if (target === "control") return kind !== "menu"
        return true
    }

    Shortcut { sequence: "Escape"; onActivated: dialog.close() }

    component Row: ItemDelegate {
        required property int index
        required property var modelData
        width: ListView.view.width
        highlighted: ListView.isCurrentItem
        icon.source: Items.iconFor(modelData) !== ""
                     ? Theme.icon(Items.iconFor(modelData)) : ""
        icon.color: "transparent"
        text: Items.labelFor(modelData)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            // Available items (the full catalog).
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 6
                    Label { text: qsTr("Available"); font.bold: true }
                    HelpMark {
                        text: qsTr("Actions you can add. Double-click one or select it "
                                   + "and press Add → to place it on the toolbar; "
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
                        // Only kinds the target bar can host; already-chosen
                        // actions drop off (separator/spacer stay, being
                        // repeatable). Reads dialog.work so it recomputes on
                        // every edit.
                        model: Items.catalog.map(e => e.id).filter(
                                   id => dialog.allowsKind(Items.kindFor(id))
                                         && (id === "separator" || id === "spacer"
                                             || dialog.work.indexOf(id) < 0))
                        ScrollBar.vertical: ScrollBar {}
                        delegate: Row {
                            onDoubleClicked: dialog.addItem(modelData)
                            onClicked: availableList.currentIndex = index
                        }
                    }
                }
            }

            // Add/remove between the two lists.
            ColumnLayout {
                Layout.alignment: Qt.AlignVCenter
                spacing: 6
                Button {
                    text: qsTr("Add →")
                    enabled: availableList.currentIndex >= 0
                    onClicked: dialog.addItem(
                                   availableList.model[availableList.currentIndex])
                }
                Button {
                    text: qsTr("← Remove")
                    enabled: currentList.currentIndex >= 0
                    onClicked: dialog.removeAt(currentList.currentIndex)
                }
            }

            // Current bar layout.
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Label { text: qsTr("Toolbar"); font.bold: true }
                Frame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    padding: 1
                    ListView {
                        id: currentList
                        anchors.fill: parent
                        clip: true
                        currentIndex: -1
                        model: dialog.work
                        ScrollBar.vertical: ScrollBar {}
                        delegate: Row {
                            onDoubleClicked: dialog.removeAt(index)
                            onClicked: currentList.currentIndex = index
                        }
                    }
                }
            }

            // Reorder.
            ColumnLayout {
                Layout.alignment: Qt.AlignVCenter
                spacing: 6
                Button {
                    text: qsTr("Up")
                    enabled: currentList.currentIndex > 0
                    onClicked: dialog.move(currentList.currentIndex, -1)
                }
                Button {
                    text: qsTr("Down")
                    enabled: currentList.currentIndex >= 0
                             && currentList.currentIndex < dialog.work.length - 1
                    onClicked: dialog.move(currentList.currentIndex, 1)
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label { text: qsTr("&Icon size:") }
            HelpMark {
                text: qsTr("Size in pixels of this toolbar's button icons.")
            }
            SpinBox {
                id: iconSizeSpin
                from: 16
                to: 48
                stepSize: 2
                value: dialog.defaultIconSize
            }

            Button {
                text: qsTr("Restore defaults")
                onClicked: {
                    dialog.work = dialog.defaultItems.slice()
                    iconSizeSpin.value = 24
                    currentList.currentIndex = -1
                }
            }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("OK")
                onClicked: {
                    dialog.accepted(dialog.target, dialog.work.slice(),
                                    iconSizeSpin.value)
                    dialog.close()
                }
            }
            Button {
                text: qsTr("Cancel")
                onClicked: dialog.close()
            }
        }
    }

    function addItem(id) {
        const at = currentList.currentIndex >= 0 ? currentList.currentIndex + 1
                                                 : work.length
        const copy = work.slice()
        copy.splice(at, 0, id)
        work = copy
        currentList.currentIndex = at
        availableList.currentIndex = -1 // its model just changed
    }
    function removeAt(index) {
        if (index < 0 || index >= work.length)
            return
        const copy = work.slice()
        copy.splice(index, 1)
        work = copy
        currentList.currentIndex = -1
    }
    function move(index, delta) {
        const to = index + delta
        if (index < 0 || to < 0 || to >= work.length)
            return
        const copy = work.slice()
        const v = copy[index]
        copy.splice(index, 1)
        copy.splice(to, 0, v)
        work = copy
        currentList.currentIndex = to
    }
}
