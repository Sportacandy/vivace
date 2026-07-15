/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Modeless separate-window variant of the playlist editor, as in
    SMPlayer. Settings.playlistAsWindow picks between this and the
    docked drawer (PlaylistPanel.qml).
*/

import QtQuick
import QtQuick.Controls

Window {
    id: playlistWindow

    required property PlayerController controller

    title: qsTr("Playlist — Vivace")
    // Wide enough that the whole editor toolbar fits on a single row.
    width: 560
    height: 540
    minimumWidth: 540
    minimumHeight: 300
    color: "#181818"

    PlaylistEditor {
        anchors.fill: parent
        controller: playlistWindow.controller
    }
}
