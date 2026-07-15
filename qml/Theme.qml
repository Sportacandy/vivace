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
