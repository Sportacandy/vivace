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

    // macOS builds this menu as a real NSMenu, and Qt only carries an item's
    // text and state across when the menu is first created -- not for the ones
    // added later by an Instantiator or addItem(), which is how the Track
    // pickers, Recent files, Favorites/TV/Radio, titles and chapters are all
    // filled in. Nudge Qt to do it whenever the contents change; a no-op on
    // Windows/Linux. Deferred because the item count changes just *before* Qt
    // creates the native item, and to coalesce a burst of inserts into one
    // sync. Every menu in Vivace is an AppMenu, so this covers all of them.
    onCountChanged: Qt.callLater(() => UiHelpers.syncNativeMenu(menu))

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
