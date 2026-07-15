/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Preferences > TV and radio. Vivace plays IPTV / web-radio streams from
    m3u/m3u8 lists (Open ▸ TV / Open ▸ Radio); the connection timeout for
    those streams lives here. DVB tuner scanning/EPG needs the mplayer/mpv
    engine Vivace does not use, so those options are shown but disabled.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    spacing: 10

    readonly property string helpText: qsTr(
        "<h1>TV and radio</h1>"
        + "<p>Vivace plays IPTV/web-radio streams from an m3u/m3u8 playlist via "
        + "Open ▸ TV and Open ▸ Radio, editable like Favorites.</p>"
        + "<p><b>Connection timeout</b> is how long to wait for stream data "
        + "before giving up. Live TV tuners often stall for several seconds "
        + "while they lock the channel; if it is too short, the stream fails "
        + "with \"Could not open file\". Raise it for slow tuners.</p>"
        + "<p>The scanning and EPG options belong to DVB tuner hardware, which "
        + "the Qt Multimedia backend does not support, so they are disabled.</p>")

    GroupBox {
        Layout.fillWidth: true
        title: qsTr("Streaming")

        RowLayout {
            anchors.fill: parent
            spacing: 6
            Label { text: qsTr("Connection timeout:") }
            SpinBox {
                from: 5; to: 300
                value: Settings.networkTimeout
                onValueModified: Settings.networkTimeout = value
            }
            Label { text: qsTr("seconds") }
            HelpMark { text: qsTr("How long to wait for network stream data before "
                                  + "giving up. Live TV tuners can stall for several "
                                  + "seconds while locking the channel — if this is too "
                                  + "short the stream fails with \"Could not open file\". "
                                  + "Applies to the FFmpeg backend.") }
            Item { Layout.fillWidth: true }
        }
    }

    GroupBox {
        Layout.fillWidth: true
        title: qsTr("DVB tuner")

        ColumnLayout {
            anchors.fill: parent
            spacing: 6

            CheckBox { text: qsTr("Rescan TV channels on startup"); enabled: false }
            CheckBox { text: qsTr("Save TV channels in favorites"); enabled: false }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                enabled: false
                Label { text: qsTr("TV standard:") }
                ComboBox { Layout.fillWidth: true; model: [] }
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                opacity: 0.7
                text: qsTr("TV and radio capture (DVB/V4L) is not supported by Vivace: it would require the mplayer/mpv engine that Vivace deliberately does not use.")
            }
        }
    }

    Item { Layout.fillHeight: true }
}
