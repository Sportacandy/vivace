/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Preferences > Subtitles. The external-subtitle renderer arrives in
    Phase 4; these options are stored now and consumed then. Options tied
    to capabilities Vivace will not have are disabled.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    spacing: 10

    readonly property string helpText: qsTr(
        "<h1>Subtitles</h1>"
        + "<p>Autoload picks up a subtitle file sitting next to the video. "
        + "The font, size and vertical position apply to the external "
        + "subtitle renderer (SRT/VTT/basic ASS) planned for a later phase.</p>"
        + "<p>Subtitles embedded in the media are drawn by the playback "
        + "backend and cannot be restyled here.</p>")

    GroupBox {
        Layout.fillWidth: true
        title: qsTr("Autoload")

        ColumnLayout {
            anchors.fill: parent
            spacing: 6

            RowLayout {
                spacing: 6
                CheckBox {
                    text: qsTr("Autoload subtitle files next to the video")
                    checked: Settings.subtitlesAutoload
                    onToggled: Settings.subtitlesAutoload = checked
                }
                HelpMark {
                    text: qsTr("On opening a video, load a subtitle file (SRT/VTT/"
                               + "basic ASS) sharing its name in the same folder.")
                }
                Item { Layout.fillWidth: true }
            }
            RowLayout {
                spacing: 8
                enabled: false
                Label { text: qsTr("Autoload rule:") }
                ComboBox {
                    Layout.fillWidth: true
                    model: [qsTr("Same name as the video")]
                }
            }
        }
    }

    GroupBox {
        Layout.fillWidth: true
        title: qsTr("Font and position")

        GridLayout {
            anchors.fill: parent
            columns: 3
            columnSpacing: 8
            rowSpacing: 6

            Label { text: qsTr("Font:") }
            TextField {
                Layout.fillWidth: true
                placeholderText: qsTr("Default application font")
                text: Settings.subFontFamily
                onEditingFinished: Settings.subFontFamily = text
            }
            Item { width: 1 }

            Label { text: qsTr("Size:") }
            SpinBox {
                from: 10; to: 80
                value: Settings.subFontSize
                onValueModified: Settings.subFontSize = value
            }
            Label { text: qsTr("px") }

            Label { text: qsTr("Vertical position:") }
            SpinBox {
                from: 0; to: 100
                value: Settings.subPosition
                onValueModified: Settings.subPosition = value
            }
            Label { text: qsTr("% from the top") }
        }
    }

    RowLayout {
        spacing: 8
        enabled: false
        Label { text: qsTr("Encoding:") }
        ComboBox { Layout.fillWidth: true; model: [qsTr("UTF-8 (autodetected)")] }
    }

    Label {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        opacity: 0.7
        text: qsTr("Font, size and position apply to Vivace's external subtitle renderer (SRT/VTT/basic ASS), loaded via Subtitles ▸ Load subtitles… or autoloaded next to the video. Subtitles embedded in the media are rendered by the playback backend and cannot be styled.")
    }

    Item { Layout.fillHeight: true }
}
