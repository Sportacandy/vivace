/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Preferences > TV and radio. Vivace plays IPTV / web-radio streams from
    m3u/m3u8 lists (Open ▸ TV / Open ▸ Radio); the connection timeout for
    those streams lives here. (DVB/V4L tuner hardware needs the mplayer/mpv
    engine Vivace deliberately does not use, so there is no tuner section
    here at all -- removed rather than shown disabled, user directive.)
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: page

    Layout.fillWidth: true
    Layout.fillHeight: true
    // Content pane vertical-scroll fix (Android, huge system font sizes can
    // make even a single-subtab page taller than the Preferences window) --
    // no horizontal scrolling is ever needed since content.width is bound
    // to the viewport's own available width.
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    readonly property string helpText: qsTr(
        "<h1>TV and radio</h1>"
        + "<p>Vivace plays IPTV/web-radio streams from an m3u/m3u8 playlist via "
        + "Open ▸ TV and Open ▸ Radio, editable like Favorites.</p>"
        + "<p><b>Connection timeout</b> is how long to wait for stream data "
        + "before giving up. Live TV tuners often stall for several seconds "
        + "while they lock the channel; if it is too short, the stream fails "
        + "with \"Could not open file\". Raise it for slow tuners.</p>")

    ColumnLayout {
        width: page.availableWidth
        spacing: 10

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

    } // ColumnLayout
}
