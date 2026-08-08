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

    // Windows-menu hover look, as in SMPlayer: a light (or, in Dark mode, a
    // deliberately darker teal -- see Theme.highlight) fill; text uses
    // palette.text below rather than a fixed color, so it's always dark on
    // the light fill and always light on the dark one. No fixed
    // implicitWidth so each menu sizes to its own widest item (content-fit).
    background: Rectangle {
        implicitHeight: 32
        radius: 4
        color: menuItem.highlighted ? Theme.highlight : "transparent"
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
            // Theme.highlight now adapts to light/dark itself (see
            // background above), so the arrow no longer needs a separate
            // "always dark on the highlight" branch -- palette.text is
            // already dark on the light fill and light on the dark fill,
            // in every state.
            color: menuItem.palette.text
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
            // palette.text (not a fixed color) so this follows Fusion's own
            // light/dark palette instead of going near-invisible in Dark
            // mode; dimmed via opacity when disabled, matching the icon
            // Image's own convention just above, rather than a second fixed
            // gray that would need its own light/dark pair.
            color: menuItem.palette.text
            opacity: menuItem.enabled ? 1.0 : 0.4
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
            // Same palette-based approach as mainLabel above; dimmed further
            // even when enabled (0.65 vs. mainLabel's 1.0) to keep the hint
            // visually secondary to the label, as the old #505050-vs-#1a1a1a
            // pairing did.
            color: menuItem.palette.text
            opacity: menuItem.enabled ? 0.65 : 0.4
        }
    }
}
