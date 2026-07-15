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

    function childPath(index) {
        return path === "" ? String(index) : path + "/" + index
    }

    function rebuild() {
        while (count > 0)
            takeItem(0)

        if (!submenuComponent)
            submenuComponent = Qt.createComponent("FavoritesMenu.qml")

        // Fixed actions first (SMPlayer order): Edit / Add current media stay
        // reachable at the top however long the list grows.
        if (showActions) {
            addItem(editComponent.createObject(null))
            addItem(addCurrentComponent.createObject(null))
            addItem(separatorComponent.createObject(null))
        }

        // createObject(null): addItem/addMenu adopt the object; passing the
        // Menu as parent instead triggers "not placed in graphics scene".
        const rows = model.items(path)
        for (let i = 0; i < rows.length; ++i) {
            if (rows[i].isSubmenu) {
                const menu = submenuComponent.createObject(
                        null, { title: rows[i].name, controller: controller,
                                model: model, itemIcon: itemIcon,
                                path: childPath(i),
                                "icon.source": Theme.icon("open_favorites") })
                addMenu(menu)
            } else {
                const item = itemComponent.createObject(
                        null, { text: rows[i].name, entryIndex: i })
                addItem(item)
            }
        }
        if (rows.length === 0)
            addItem(emptyComponent.createObject(null))
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
