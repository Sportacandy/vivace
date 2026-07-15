/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    MenuItem that renders what the built-in styles leave out: an icon
    column, the mnemonic underline and the keyboard shortcut hint.
*/

import QtQuick
import QtQuick.Controls
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
