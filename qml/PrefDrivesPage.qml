/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Preferences > Drives. Vivace plays unencrypted DVD-Video (its own IFO
    parser + a QIODevice into QMediaPlayer); there is no persistent device
    to configure — a disc or VIDEO_TS folder is chosen when opening. Menus,
    CSS decryption and Blu-ray are not supported.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    spacing: 10

    readonly property string helpText: qsTr(
        "<h1>Drives</h1>"
        + "<p>Vivace plays unencrypted DVD-Video. Open a disc or a VIDEO_TS "
        + "folder with Open ▸ Disc ▸ DVD… — the title list and chapters are "
        + "read from the disc's IFO structure, so there is no drive to "
        + "preselect.</p>"
        + "<p><b>DVD menus (experimental):</b> when enabled, opening a disc "
        + "shows its menu — buttons are clickable and can be navigated with the "
        + "arrow keys (Enter selects). Use Browse ▸ DVD menu to return to it. "
        + "If the menu is left idle for the timeout below, playback of the main "
        + "title starts automatically (set 0 to keep the menu open). Subpicture "
        + "highlight graphics and the First-Play sequence are not fully "
        + "emulated.</p>"
        + "<p>CSS-encrypted discs and Blu-ray are not supported. Audio CD "
        + "playback is being considered for a later phase.</p>")

    GroupBox {
        Layout.fillWidth: true
        title: qsTr("DVD")

        ColumnLayout {
            anchors.fill: parent
            spacing: 6

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Vivace plays unencrypted DVD-Video. Use Open ▸ Disc ▸ DVD… (or drop a disc/VIDEO_TS folder) — the title list and chapters are read from the disc structure. There is no drive to preselect here.")
            }
            CheckBox {
                id: useMenus
                text: qsTr("Show DVD menus (experimental)")
                checked: Settings.dvdUseMenus
                onToggled: Settings.dvdUseMenus = checked
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                enabled: useMenus.checked
                CheckBox {
                    text: qsTr("Play the First-Play sequence (intro / warnings)")
                    checked: Settings.dvdUseFirstPlay
                    onToggled: Settings.dvdUseFirstPlay = checked
                }
                HelpMark {
                    text: qsTr("Run the disc's on-insert sequence (studio logos, "
                               + "warnings or an intro that may lead to a menu or "
                               + "auto-play), as a set-top player does. Off: go "
                               + "straight to the disc's interactive menu.")
                }
                Item { Layout.fillWidth: true }
            }
            RowLayout {
                Layout.fillWidth: true
                enabled: useMenus.checked
                Label { text: qsTr("Return to playback after menu inactivity:") }
                SpinBox {
                    from: 0
                    to: 600
                    stepSize: 5
                    value: Settings.dvdMenuTimeout
                    onValueModified: Settings.dvdMenuTimeout = value
                    textFromValue: (v, loc) => v === 0 ? qsTr("Never")
                                                       : qsTr("%1 s").arg(v)
                    valueFromText: (t, loc) => t === qsTr("Never")
                                   ? 0 : parseInt(t)
                }
                HelpMark {
                    text: qsTr("Seconds a DVD menu may sit idle before Vivace "
                               + "plays the main title. Any menu activity resets "
                               + "the timer; 0 keeps the menu open indefinitely.")
                }
                Item { Layout.fillWidth: true }
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                opacity: 0.7
                font.pixelSize: 12
                text: qsTr("Experimental: menu buttons are clickable and arrow-key navigable, but subpicture highlight graphics and the First-Play sequence are not fully emulated. CSS-encrypted discs (libdvdcss) and Blu-ray are not supported.")
            }
        }
    }

    GroupBox {
        Layout.fillWidth: true
        title: qsTr("Audio CD")

        Label {
            anchors.fill: parent
            wrapMode: Text.WordWrap
            opacity: 0.7
            text: qsTr("Audio CD playback (libcdio) is being considered for a later phase.")
        }
    }

    Item { Layout.fillHeight: true }
}
