/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Recursive list menu over a FavoritesModel: items play, submenus expand
    into nested instances (SMPlayer's nestable favorites, also used for the
    TV and Radio lists). Built imperatively because a Menu must mix addItem
    (entries) and addMenu (submenus) in one ordered list.
*/

import QtQuick
import QtQuick.Controls

AppMenu {
    id: favMenu

    required property PlayerController controller
    required property FavoritesModel model
    property string path: "" // "" = root, "2/0" = nested
    // The root menu shows the Edit / Add-current actions at the top.
    property bool showActions: false
    property string itemIcon: Theme.icon("favorite")

    signal editRequested()
    signal addCurrentRequested()

    // Created at runtime (not a declared Component) so QML does not reject
    // FavoritesMenu referencing its own type as static recursion.
    property var submenuComponent: null

    // Items/submenus below are created with createObject(null, ...) --
    // parentless, because parenting them to this Menu directly triggers
    // "not placed in graphics scene" -- so addItem()/addMenu() are relied on
    // to adopt them. A parentless createObject() result gets JavaScript
    // ownership (Qt's default), and that adoption does not reliably flip it
    // to CppOwnership, so without this array the JS garbage collector
    // remained free to reclaim an item at any time, independent of it still
    // being part of the visible menu -- given enough GC pressure (which
    // video playback and repeated dialog churn both supply). This was the
    // root cause of a defect where a cascading submenu built entirely from
    // addItem() rows (e.g. Favorites/Radio's flat lists) took an
    // ever-growing amount of time to open, then stopped opening at all.
    // Keeping a plain JS reference to every created item/menu here, for the
    // lifetime of this FavoritesMenu, keeps them permanently reachable so
    // the GC can never collect one out from under the menu.
    property var _keepAlive: []

    function childPath(index) {
        return path === "" ? String(index) : path + "/" + index
    }

    function rebuild() {
        while (count > 0)
            takeItem(0)
        _keepAlive = []

        // Explicitly synchronous: Qt.createComponent()'s mode argument
        // defaults to Asynchronous for a URL that isn't already compiled/
        // cached, so without forcing PreferSynchronous this could hand back
        // a component that's still Loading, and createObject() on a
        // not-yet-Ready component silently returns null.
        if (!submenuComponent) {
            submenuComponent = Qt.createComponent("FavoritesMenu.qml",
                                                   Component.PreferSynchronous)
        }

        // Fixed actions first (SMPlayer order): Edit / Add current media stay
        // reachable at the top however long the list grows.
        if (showActions) {
            const editItem = editComponent.createObject(null)
            const addCurrentItem = addCurrentComponent.createObject(null)
            const sepItem = separatorComponent.createObject(null)
            _keepAlive.push(editItem, addCurrentItem, sepItem)
            addItem(editItem)
            addItem(addCurrentItem)
            addItem(sepItem)
        }

        const rows = model.items(path)
        for (let i = 0; i < rows.length; ++i) {
            if (rows[i].isSubmenu) {
                const menu = submenuComponent.createObject(
                        null, { title: rows[i].name, controller: controller,
                                model: model, itemIcon: itemIcon,
                                path: childPath(i),
                                "icon.source": Theme.icon("open_favorites") })
                _keepAlive.push(menu)
                addMenu(menu)
            } else {
                const item = itemComponent.createObject(
                        null, { text: rows[i].name, entryIndex: i })
                _keepAlive.push(item)
                addItem(item)
            }
        }
        if (rows.length === 0) {
            const empty = emptyComponent.createObject(null)
            _keepAlive.push(empty)
            addItem(empty)
        }
    }

    Component.onCompleted: rebuild()

    Connections {
        target: favMenu.model
        function onChanged() { favMenu.rebuild() }
    }

    Component {
        id: itemComponent
        AppMenuItem {
            property int entryIndex: -1
            icon.source: favMenu.itemIcon
            onTriggered: favMenu.controller.open(
                    [favMenu.model.urlAt(favMenu.path, entryIndex)])
        }
    }

    Component {
        id: emptyComponent
        AppMenuItem { text: qsTr("(empty)"); enabled: false }
    }

    Component {
        id: separatorComponent
        MenuSeparator {}
    }
    Component {
        id: editComponent
        AppMenuItem {
            text: qsTr("&Edit…")
            onTriggered: favMenu.editRequested()
        }
    }
    Component {
        id: addCurrentComponent
        AppMenuItem {
            text: qsTr("&Add current media")
            icon.source: Theme.icon("favorite-add")
            enabled: favMenu.controller.player.source.toString() !== ""
            onTriggered: favMenu.addCurrentRequested()
        }
    }
}
