/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Preferences > Keyboard and mouse. The keyboard-shortcut editor lists every
    editable action (Shortcuts registry) and lets you rebind or reset each; the
    mouse-function combos are live.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: page
    spacing: 10

    readonly property string helpText: qsTr(
        "<h1>Keyboard and mouse</h1>"
        + "<p>Click a shortcut to record a new key combination for that action; "
        + "the change applies immediately. Use <b>Reset</b> on a row to restore "
        + "its default, or <b>Reset all</b> for every shortcut. If a combination "
        + "is already used, you'll be asked before it is reassigned.</p>"
        + "<p>Below, assign what the left, double, middle click and the mouse "
        + "wheel do over the video.</p>")

    // Rows for the list; re-reads on any shortcut change and on the filter.
    property string filterText: ""
    readonly property var rows: {
        Shortcuts.sequences // establish a dependency so this recomputes on change
        const all = Shortcuts.actionList()
        if (filterText === "")
            return all
        const f = filterText.toLowerCase()
        return all.filter(r => r.label.toLowerCase().indexOf(f) >= 0
                            || r.section.toLowerCase().indexOf(f) >= 0
                            || (r.sequence || "").toLowerCase().indexOf(f) >= 0)
    }

    GroupBox {
        Layout.fillWidth: true
        Layout.fillHeight: true
        title: qsTr("Keyboard shortcuts")

        ColumnLayout {
            anchors.fill: parent
            spacing: 6

            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                TextField {
                    id: filterField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Filter…")
                    onTextChanged: page.filterText = text
                }
                Button {
                    text: qsTr("Reset all")
                    onClicked: Shortcuts.resetAll()
                }
            }

            ListView {
                id: list
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: page.rows
                ScrollBar.vertical: ScrollBar {}

                delegate: Rectangle {
                    required property int index
                    required property var modelData
                    width: ListView.view.width
                    height: 34
                    color: index % 2 ? "transparent" : "#f4f4f4"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 6
                        anchors.rightMargin: 6
                        spacing: 8

                        Label {
                            text: modelData.section
                            opacity: 0.6
                            Layout.preferredWidth: 62
                        }
                        Label {
                            text: modelData.label
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                        Button {
                            Layout.preferredWidth: 150
                            text: modelData.sequence !== "" ? modelData.sequence
                                                            : qsTr("(none)")
                            font.bold: modelData.custom
                            onClicked: captureWindow.begin(modelData.id,
                                                           modelData.label)
                        }
                        ToolButton {
                            text: "⟲"
                            visible: modelData.custom
                            ToolTip.text: qsTr("Reset to default")
                            ToolTip.visible: hovered
                            onClicked: Shortcuts.reset(modelData.id)
                        }
                    }
                }
            }
        }
    }

    GroupBox {
        Layout.fillWidth: true
        title: qsTr("Mouse")

        GridLayout {
            anchors.fill: parent
            columns: 2
            columnSpacing: 8
            rowSpacing: 6

            Label { text: qsTr("Left click function:") }
            ComboBox {
                Layout.fillWidth: true
                model: [qsTr("No function"), qsTr("Play / Pause"),
                        qsTr("Toggle fullscreen")]
                currentIndex: Settings.leftClickFunction
                onActivated: index => Settings.leftClickFunction = index
            }

            Label { text: qsTr("Double click function:") }
            ComboBox {
                Layout.fillWidth: true
                model: [qsTr("Toggle fullscreen"), qsTr("Play / Pause"),
                        qsTr("No function")]
                currentIndex: Settings.doubleClickFunction
                onActivated: index => Settings.doubleClickFunction = index
            }

            Label { text: qsTr("Middle click function:") }
            ComboBox {
                Layout.fillWidth: true
                model: [qsTr("Mute"), qsTr("Play / Pause"),
                        qsTr("Toggle fullscreen"), qsTr("No function")]
                currentIndex: Settings.middleClickFunction
                onActivated: index => Settings.middleClickFunction = index
            }

            RowLayout {
                spacing: 6
                Label { text: qsTr("Wheel function:") }
                HelpMark { text: qsTr("Chooses what turning the mouse wheel over the video does: seek through the file or change the volume.") }
            }
            ComboBox {
                Layout.fillWidth: true
                model: [qsTr("Media seeking"), qsTr("Volume control"),
                        qsTr("No function")]
                currentIndex: Settings.wheelFunction
                onActivated: index => Settings.wheelFunction = index
            }
        }
    }

    // ---- shortcut capture dialog (records the next key combination) --------
    Window {
        id: captureWindow

        property string targetId: ""
        property string targetLabel: ""
        property string pending: ""
        property string conflictId: ""

        title: qsTr("Set shortcut")
        flags: Qt.Dialog
        modality: Qt.WindowModal
        color: palette.window
        width: 380
        height: capCol.implicitHeight + 24
        minimumWidth: 320

        function begin(id, label) {
            targetId = id
            targetLabel = label
            pending = Shortcuts.sequence(id)
            conflictId = ""
            if (transientParent) {
                x = transientParent.x + (transientParent.width - width) / 2
                y = transientParent.y + (transientParent.height - height) / 2
            }
            visible = true
            raise()
            requestActivate()
            keyCatcher.forceActiveFocus()
        }

        // Grabs key presses while the dialog is open.
        Item {
            id: keyCatcher
            anchors.fill: parent
            focus: true
            Keys.onPressed: (event) => {
                event.accepted = true
                // Plain Escape cancels the capture.
                if (event.key === Qt.Key_Escape
                        && (event.modifiers & ~Qt.KeypadModifier) === Qt.NoModifier) {
                    captureWindow.close()
                    return
                }
                const seq = Shortcuts.sequenceForKey(event.key, event.modifiers)
                if (seq === "")
                    return // lone modifier; keep waiting
                captureWindow.pending = seq
                captureWindow.conflictId = Shortcuts.conflict(seq, captureWindow.targetId)
            }
        }

        ColumnLayout {
            id: capCol
            anchors.fill: parent
            anchors.margins: 12
            spacing: 10

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Press the new shortcut for “%1”.").arg(captureWindow.targetLabel)
            }
            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 20
                font.bold: true
                text: captureWindow.pending !== "" ? captureWindow.pending
                                                   : qsTr("…")
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: "#b00000"
                visible: captureWindow.conflictId !== ""
                text: qsTr("Already used by “%1”. Assigning will clear it there.")
                      .arg(Shortcuts.label(captureWindow.conflictId))
            }

            MenuSeparator { Layout.fillWidth: true }

            RowLayout {
                Layout.fillWidth: true
                Button {
                    text: qsTr("Clear")
                    ToolTip.text: qsTr("Leave this action without a shortcut")
                    ToolTip.visible: hovered
                    onClicked: {
                        Shortcuts.setSequence(captureWindow.targetId, "")
                        captureWindow.close()
                    }
                }
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("Assign")
                    enabled: captureWindow.pending !== ""
                    onClicked: {
                        if (captureWindow.conflictId !== "")
                            Shortcuts.setSequence(captureWindow.conflictId, "")
                        Shortcuts.setSequence(captureWindow.targetId,
                                              captureWindow.pending)
                        captureWindow.close()
                    }
                }
                Button {
                    text: qsTr("Cancel")
                    onClicked: captureWindow.close()
                }
            }
        }
    }
}
