/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Modeless separate-window variant of the playlist editor, as in
    SMPlayer. Settings.playlistAsWindow picks between this and the
    docked drawer (PlaylistPanel.qml).
*/

import QtQuick
import QtQuick.Controls
import "GeometryUtils.js" as GeometryUtils

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

    // Remembered like the main window's geometry (same rememberGeometry
    // toggle, own storage key) -- including the same safety check: only the
    // SIZE is restored unconditionally, the POSITION only if it still lands
    // on a currently connected screen, so a display layout change since the
    // last run (a monitor unplugged or rearranged) can't strand this window
    // off in now-empty virtual-desktop space the way the main window once
    // could.
    Component.onCompleted: {
        if (!Settings.rememberGeometry)
            return
        const g = Settings.playlistWindowGeometry
        if (g.width < playlistWindow.minimumWidth || g.height < playlistWindow.minimumHeight)
            return
        playlistWindow.width = g.width
        playlistWindow.height = g.height
        if (GeometryUtils.rectVisibleOnAnyScreen(g.x, g.y, g.width, g.height)) {
            playlistWindow.x = g.x
            playlistWindow.y = g.y
        }
    }
    onClosing: {
        if (Settings.rememberGeometry && visibility === Window.Windowed)
            Settings.playlistWindowGeometry =
                    Qt.rect(playlistWindow.x, playlistWindow.y,
                            playlistWindow.width, playlistWindow.height)
    }

    PlaylistEditor {
        anchors.fill: parent
        controller: playlistWindow.controller
    }
}
