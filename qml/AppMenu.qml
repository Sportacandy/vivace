/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Menu whose Action children are presented by AppMenuItem, so icons,
    mnemonics and shortcut hints show up regardless of the Qt style.
*/

import QtQuick
import QtQuick.Controls

Menu {
    id: menu
    delegate: AppMenuItem {}

    // QQuick's Menu does not grow to its widest item on its own (its width
    // comes from the style's default background), so long labels get clipped.
    // Size the menu to the widest item's implicit width. Re-evaluated when the
    // item count changes (covers Instantiator-populated menus).
    implicitWidth: {
        let widest = 0
        for (let i = 0; i < count; ++i) {
            const item = menu.itemAt(i)
            if (item)
                widest = Math.max(widest, item.implicitWidth)
        }
        return widest + leftPadding + rightPadding
    }
}
