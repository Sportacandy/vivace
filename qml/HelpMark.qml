/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    A small circled "?" that reveals its help text on hover (desktop) OR on tap
    (touch), so the same help is reachable with or without a mouse pointer. Place
    it next to a label or field:

        RowLayout {
            Label { text: qsTr("Volume step") }
            HelpMark { text: qsTr("How much each volume key press changes the level.") }
        }

    On a desktop the tip appears while hovering; on a tablet a tap pins it (and it
    auto-dismisses after a few seconds so it never lingers). Both work everywhere,
    which is the point — no separate "tablet mode" needed for help to be usable.
*/

import QtQuick
import QtQuick.Controls

Control {
    id: root

    // The help text shown in the popup. Plain or simple rich text.
    property string text
    // Maximum width of the help bubble before the text wraps.
    property int maxPopupWidth: 320

    implicitWidth: Theme.sz(18)
    implicitHeight: Theme.sz(18)
    padding: 0

    // True while the tip is pinned open by a tap (touch has no hover).
    property bool _pinned: false
    readonly property bool _active: hover.hovered || root._pinned

    background: Rectangle {
        radius: width / 2
        color: root._active ? root.palette.highlight : root.palette.mid
    }

    contentItem: Label {
        text: "?"
        font.bold: true
        font.pixelSize: Math.round(root.height * 0.7)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: root._active ? root.palette.highlightedText : root.palette.buttonText
    }

    HoverHandler { id: hover }

    TapHandler {
        // Tap toggles the pinned tip (works for touch and mouse click).
        onTapped: {
            root._pinned = !root._pinned
            if (root._pinned)
                dismiss.restart()
            else
                dismiss.stop()
        }
    }

    // Auto-dismiss a tap-opened tip so it doesn't stay on screen on touch.
    Timer { id: dismiss; interval: 6000; onTriggered: root._pinned = false }

    ToolTip {
        parent: root
        visible: root._active && root.text.length > 0
        delay: hover.hovered && !root._pinned ? 400 : 0
        timeout: -1
        contentItem: Label {
            text: root.text
            wrapMode: Text.WordWrap
            // Wrap once the natural text would exceed the max bubble width.
            width: Math.min(implicitWidth, root.maxPopupWidth)
        }
    }
}
