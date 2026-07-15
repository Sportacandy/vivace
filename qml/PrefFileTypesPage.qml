/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Preferences > File types — Windows file associations (user scope,
    HKCU\Software\Classes; no administrator rights needed).
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: page
    spacing: 10

    readonly property var extensions: [
        "mp4", "mkv", "avi", "mov", "webm", "wmv", "ts", "m2ts", "flv", "ogv",
        "mp3", "m4a", "flac", "ogg", "opus", "wav", "wma", "m3u", "m3u8"
    ]

    // ext -> checked (working copy); committed to the registry on Apply.
    property var checkedState: ({})
    property var associatedNow: []
    property bool dirty: false

    function reload() {
        associatedNow = UiHelpers.associatedExtensions(extensions)
        const s = {}
        for (let i = 0; i < extensions.length; ++i)
            s[extensions[i]] = associatedNow.indexOf(extensions[i]) >= 0
        checkedState = s
        dirty = false
    }

    function setAll(value) {
        const s = {}
        for (let i = 0; i < extensions.length; ++i)
            s[extensions[i]] = value
        checkedState = s
        dirty = true
    }

    function apply() {
        const toAssociate = []
        const toRemove = []
        for (let i = 0; i < extensions.length; ++i) {
            const ext = extensions[i]
            const was = associatedNow.indexOf(ext) >= 0
            const now = checkedState[ext] === true
            if (now && !was) toAssociate.push(ext)
            else if (!now && was) toRemove.push(ext)
        }
        UiHelpers.setFileAssociations(toAssociate, toRemove)
        reload()
    }

    Component.onCompleted: reload()

    Label {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        text: qsTr("Select the file types Vivace should be associated with:")
    }

    Frame {
        Layout.fillWidth: true
        Layout.fillHeight: true
        enabled: UiHelpers.fileAssociationsSupported()

        GridLayout {
            anchors.fill: parent
            columns: 4

            Repeater {
                model: page.extensions
                CheckBox {
                    required property string modelData
                    text: "." + modelData
                    checked: page.checkedState[modelData] === true
                    onToggled: {
                        const s = page.checkedState
                        s[modelData] = checked
                        page.checkedState = s
                        page.dirty = true
                    }
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 8
        enabled: UiHelpers.fileAssociationsSupported()
        Button {
            text: qsTr("Select all")
            onClicked: page.setAll(true)
        }
        Button {
            text: qsTr("Select none")
            onClicked: page.setAll(false)
        }
        Item { Layout.fillWidth: true }
        Button {
            text: qsTr("Apply")
            enabled: page.dirty
            onClicked: page.apply()
        }
    }

    Label {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        opacity: 0.7
        font.pixelSize: 12
        visible: UiHelpers.fileAssociationsSupported()
        text: qsTr("This registers Vivace for the selected types and adds it to the \"Open with\" list. Windows may still ask you to confirm the default app for a type in its Settings.")
    }
    Label {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        opacity: 0.7
        visible: !UiHelpers.fileAssociationsSupported()
        text: qsTr("File associations are only available on Windows.")
    }

    Item { Layout.fillHeight: true }
}
