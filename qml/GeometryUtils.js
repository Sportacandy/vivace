/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Shared window-geometry helpers, used by both Main.qml (the main window)
    and PlaylistWindow.qml (the separate-window playlist), so a remembered
    position is validated the same way in both places.
*/
.pragma library

// True if enough of rect (x,y,w,h) -- in virtual-desktop coordinates --
// overlaps some CURRENTLY CONNECTED screen to be seen and dragged back into
// view (a title bar's worth, not just one stray pixel). Checked against
// every screen in Qt.application.screens rather than relying on whichever
// screen a Window's own Screen attached property currently resolves to,
// since a remembered rect from a monitor that has since been unplugged or
// rearranged can otherwise still read back as "on screen" on some platforms
// until the window is actually moved.
function rectVisibleOnAnyScreen(x, y, w, h) {
    const minVisibleWidth = 100
    const minVisibleHeight = 30
    const screens = Qt.application.screens
    for (let i = 0; i < screens.length; ++i) {
        const s = screens[i]
        const overlapW = Math.min(x + w, s.virtualX + s.width) - Math.max(x, s.virtualX)
        const overlapH = Math.min(y + h, s.virtualY + s.height) - Math.max(y, s.virtualY)
        if (overlapW >= minVisibleWidth && overlapH >= minVisibleHeight)
            return true
    }
    return false
}
