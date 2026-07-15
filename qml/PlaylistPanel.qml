/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Docked variant of the playlist editor (overlay drawer). The separate-
    window variant is PlaylistWindow.qml; Settings.playlistAsWindow picks.
*/

import QtQuick
import QtQuick.Controls

Drawer {
    id: panel

    required property PlayerController controller

    edge: Qt.RightEdge
    modal: false
    dim: false
    interactive: false
    closePolicy: Popup.NoAutoClose

    background: Rectangle {
        color: "#f0181818"
    }

    PlaylistEditor {
        anchors.fill: parent
        controller: panel.controller
    }
}
