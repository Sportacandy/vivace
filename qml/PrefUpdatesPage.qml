/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Preferences > Updates — enable/disable the automatic update check and set
    how often it runs. Mirrors SMPlayer's prefupdates (Check for updates +
    check interval in days).
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    spacing: 10

    readonly property string helpText: qsTr(
        "<h1>Updates</h1>"
        + "<p><b>Check for updates</b>: when enabled, Vivace checks for a newer "
        + "release on startup and notifies you if one is available.</p>"
        + "<p><b>Check interval</b>: how many days to wait between automatic "
        + "checks.</p>"
        + "<p>You can also check at any time from Help &gt; Check for updates.</p>")

    RowLayout {
        spacing: 6
        CheckBox {
            text: qsTr("Check for updates")
            checked: Settings.updateCheckEnabled
            onToggled: Settings.updateCheckEnabled = checked
        }
        HelpMark { text: qsTr("When enabled, Vivace checks for a newer release on startup and notifies you only if one is available.") }
        Item { Layout.fillWidth: true }
    }

    RowLayout {
        spacing: 8
        enabled: Settings.updateCheckEnabled
        Label { text: qsTr("Check interval:") }
        SpinBox {
            from: 1
            to: 90
            value: Settings.updateCheckIntervalDays
            onValueModified: Settings.updateCheckIntervalDays = value
        }
        Label { text: qsTr("days") }
        Item { Layout.fillWidth: true }
    }

    Label {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        opacity: 0.7
        text: Settings.updateLastCheck.length > 0
              ? qsTr("Last checked: %1").arg(Settings.updateLastCheck)
              : qsTr("Not checked yet.")
    }

    Item { Layout.fillHeight: true }
}
