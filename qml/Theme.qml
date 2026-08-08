/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Icon-set resolver. icon("play") returns the current set's PNG; because
    the function reads Settings.iconSet, bindings using it re-evaluate live
    when the set changes in Preferences. Icon sets live under icons/<set>/;
    "Default" (SMPlayer's H2O) and "Classic" (SMPlayer's default-theme).
*/

pragma Singleton

import QtQuick

QtObject {
    // Live, reactive: Qt.styleHints.colorScheme has a NOTIFY signal, so any
    // binding that reads this re-evaluates automatically when the OS theme
    // changes. Used only where a custom (non-palette) color pair is wanted
    // -- e.g. the menu hover highlight below, which is deliberately NOT the
    // same as palette.highlight (the system accent color) in either mode.
    // Prefer a Control's own .palette property instead of this, wherever
    // one is available; it already tracks light/dark via Fusion.
    readonly property bool dark: Qt.styleHints.colorScheme === Qt.ColorScheme.Dark

    // Selected/hovered item fill, shared by menus (AppMenuItem, MainMenuBar)
    // and list rows (Bookmarks/Favorites/Media info) -- light mode reused
    // the exact same "#cce8ff" literal in both places already, so this just
    // turns that duplication into one token. Light value is the long-
    // standing Windows-classic-menu look (unchanged); dark value is a
    // deliberately darker, desaturated teal rather than a brighter blue, so
    // the highlight doesn't glare against an otherwise dark menu/list (user
    // feedback, 2026-08-08). Text drawn on top should use palette.text, not
    // a fixed dark color, since palette.text is already dark on the light
    // fill and light on the dark fill.
    readonly property color highlight: dark ? "#1d545c" : "#cce8ff"

    readonly property string base: "qrc:/qt/qml/Vivace/icons/"

    function icon(name) {
        const set = Settings.iconSet === "Classic" ? "Classic" : "Default"
        return base + set + "/" + name + ".png"
    }

    // Touch-friendly sizing multiplier: 1.0 normally, larger in touch mode so
    // icons / hit targets grow for finger use. Font scaling is applied at the
    // window root (Main.qml); this covers icon-sized elements that don't follow
    // the font (toolbar/control-bar buttons, the "?" help mark, slider handles).
    // A binding, so it re-evaluates live when Settings.touchMode toggles.
    readonly property real touchScale: Settings.touchMode ? 1.5 : 1.0

    // Scale an icon/target dimension for the current touch state.
    function sz(base) { return Math.round(base * touchScale) }
}
