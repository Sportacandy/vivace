/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    MenuItem that renders what the built-in styles leave out: an icon
    column, the mnemonic underline and the keyboard shortcut hint.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Layouts

MenuItem {
    id: menuItem

    icon.width: 16
    icon.height: 16

    // Windows-menu hover look, as in SMPlayer: light blue fill, text stays
    // dark (styles switching to white-on-light-gray are unreadable). No fixed
    // implicitWidth so each menu sizes to its own widest item (content-fit).
    background: Rectangle {
        implicitHeight: 32
        radius: 4
        color: menuItem.highlighted ? "#cce8ff" : "transparent"
    }

    // Our own cascading-submenu arrow, replacing the Fusion style's stock
    // one (qtbase's Fusion/MenuItem.qml: `arrow: ColorImage { x: control.
    // mirrored ? control.padding : control.width - width - control.padding
    // ... } }`). That exact expression was flagged by Qt's own binding-loop
    // detector ("Binding loop detected for property x" / "Property depends
    // on itself!") during investigation of a defect where a cascading
    // submenu built entirely from addItem() rows (no addMenu()-created
    // child, e.g. Favorites/Radio's flat lists) could take an ever-growing
    // amount of time to open, or eventually stop opening at all (2026-07-31).
    // The actual root cause turned out to be the lifetime of the dynamically
    // created menu items, not this binding itself (see FavoritesMenu.qml's
    // _keepAlive) -- but the stock arrow's control.mirrored/control.width-
    // dependent binding still intermittently logs the same errors on its
    // own, so it's replaced here too, out of caution, with a version that
    // never references control.mirrored at all (always treated as false).
    arrow: Item {
        width: 20
        height: 20
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: menuItem.padding
        visible: menuItem.subMenu

        ColorImage {
            anchors.fill: parent
            source: "qrc:/qt-project.org/imports/QtQuick/Controls/Fusion/images/arrow.png"
            rotation: -90
            fillMode: Image.Pad
            color: menuItem.down || menuItem.hovered || menuItem.highlighted
                   ? "#1a1a1a" : "#505050"
        }
    }

    // Use the RowLayout directly as the contentItem so the Control reads its
    // implicitWidth (a wrapper Item with an anchor-filled Layout does not
    // propagate it, so the menu never grew to fit long labels).
    contentItem: RowLayout {
        id: itemRow
        spacing: 8

        Item {
            // Leading gap that clears the check indicator, as the default
            // styles do.
            Layout.preferredWidth: menuItem.indicator ? menuItem.indicator.width + 4 : 0
        }

        Item {
            // Fixed 16px column; submenu items inherit unsized icon
            // properties from their Menu, so icon.width can't be trusted.
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16

            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: menuItem.icon.source
                sourceSize: Qt.size(16, 16)
                opacity: menuItem.enabled ? 1.0 : 0.4
            }
        }

        Label {
            id: mainLabel
            // The mnemonic underline forces StyledText, whose implicitWidth
            // depends on the allocated width (it wraps), so it can't drive the
            // menu width. Measure the plain label with TextMetrics and pin the
            // preferred width to that, so the menu sizes to its widest label.
            text: UiHelpers.mnemonicLabel(menuItem.text)
            textFormat: Text.StyledText
            verticalAlignment: Text.AlignVCenter
            color: menuItem.enabled ? "#1a1a1a" : "#9d9d9d"
            Layout.preferredWidth: labelMetrics.advanceWidth + 2

            TextMetrics {
                id: labelMetrics
                // TextMetrics is non-visual, so `parent` is undefined here; use
                // the Label's id for the font.
                font: mainLabel.font
                // Drop the '&' mnemonic markers for measuring.
                text: menuItem.text.replace(/&/g, "")
            }
        }

        Item {
            // Pushes the shortcut to the right and sets the minimum gap between
            // the label and the shortcut hint.
            Layout.fillWidth: true
            Layout.minimumWidth: 24
        }

        Label {
            text: menuItem.action
                  ? UiHelpers.shortcutText(menuItem.action.shortcut) : ""
            visible: text !== ""
            verticalAlignment: Text.AlignVCenter
            color: menuItem.enabled ? "#505050" : "#9d9d9d"
        }
    }
}
