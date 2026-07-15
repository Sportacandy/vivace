/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Video equalizer (SMPlayer's VideoEqualizer): contrast / brightness / hue /
    saturation / gamma sliders (-100..100), applied live through the shader.
    A normal top-level dialog (OS window frame); non-modal so the effect stays
    visible on the video while adjusting.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: dialog

    title: qsTr("Video equalizer")
    flags: Qt.Dialog
    color: palette.window
    width: 440
    height: eqCol.implicitHeight + 24
    minimumWidth: 360
    minimumHeight: eqCol.implicitHeight + 24

    function open() {
        if (transientParent) {
            x = transientParent.x + (transientParent.width - width) / 2
            y = transientParent.y + (transientParent.height - height) / 2
        }
        visible = true
        raise()
        requestActivate()
    }

    Shortcut {
        sequences: [StandardKey.Cancel]
        onActivated: dialog.close()
    }

    // Label + slider (-100..100) + value indicator. The Binding-on-value
    // (disabled while dragging) keeps the slider in sync with the setting,
    // so Reset moves the sliders back even after a manual drag.
    component EqRow: RowLayout {
        id: row
        property string label
        property string help
        property int value
        signal moved(int v)
        spacing: 8
        Label { text: row.label; Layout.preferredWidth: 84 }
        HelpMark { text: row.help }
        Slider {
            id: sld
            Layout.fillWidth: true
            from: -100
            to: 100
            stepSize: 1
            onMoved: row.moved(Math.round(value))
            // Keep in sync with the setting except while being dragged.
            Binding on value { value: row.value; when: !sld.pressed }
        }
        // Editable numeric input for setting an exact value.
        SpinBox {
            id: spin
            from: -100
            to: 100
            editable: true
            Layout.preferredWidth: 96
            onValueModified: row.moved(value)
            Binding on value { value: row.value; when: !spin.activeFocus }
        }
    }

    ColumnLayout {
        id: eqCol
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        EqRow {
            label: qsTr("Contrast")
            help: qsTr("Difference between the darkest and brightest parts of the picture.")
            value: Settings.eqContrast
            onMoved: v => Settings.eqContrast = v
        }
        EqRow {
            label: qsTr("Brightness")
            help: qsTr("Overall lightness of the picture.")
            value: Settings.eqBrightness
            onMoved: v => Settings.eqBrightness = v
        }
        EqRow {
            label: qsTr("Hue")
            help: qsTr("Shifts all colours around the colour wheel.")
            value: Settings.eqHue
            onMoved: v => Settings.eqHue = v
        }
        EqRow {
            label: qsTr("Saturation")
            help: qsTr("Colour intensity; the lowest value gives a grayscale picture.")
            value: Settings.eqSaturation
            onMoved: v => Settings.eqSaturation = v
        }
        EqRow {
            label: qsTr("Gamma")
            help: qsTr("Adjusts mid-tone brightness without changing black and white.")
            value: Settings.eqGamma
            onMoved: v => Settings.eqGamma = v
        }

        MenuSeparator { Layout.fillWidth: true }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Button {
                // Zero all values (SMPlayer's Reset resets to 0, not to the
                // saved defaults).
                text: qsTr("&Reset")
                onClicked: {
                    Settings.eqContrast = 0
                    Settings.eqBrightness = 0
                    Settings.eqHue = 0
                    Settings.eqSaturation = 0
                    Settings.eqGamma = 0
                }
            }
            HelpMark { text: qsTr("Sets all values back to zero (no adjustment).") }
            Button {
                // Save the current values as the defaults (SMPlayer's
                // "Set as default values").
                text: qsTr("Set as &default values")
                onClicked: {
                    Settings.eqDefContrast = Settings.eqContrast
                    Settings.eqDefBrightness = Settings.eqBrightness
                    Settings.eqDefHue = Settings.eqHue
                    Settings.eqDefSaturation = Settings.eqSaturation
                    Settings.eqDefGamma = Settings.eqGamma
                    savedInfo.showCentered()
                }
            }
            HelpMark { text: qsTr("Stores the current values as the defaults applied to each newly opened file.") }
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("&Close")
                onClicked: dialog.close()
            }
        }
    }

    // Confirmation shown after storing the defaults (SMPlayer's message box).
    Window {
        id: savedInfo
        title: qsTr("Information")
        flags: Qt.Dialog
        modality: Qt.WindowModal
        transientParent: dialog
        color: palette.window
        width: 360
        height: savedCol.implicitHeight + 24
        minimumWidth: 280
        minimumHeight: savedCol.implicitHeight + 24

        function showCentered() {
            x = dialog.x + (dialog.width - width) / 2
            y = dialog.y + (dialog.height - height) / 2
            visible = true
            raise()
            requestActivate()
        }

        Shortcut {
            sequences: [StandardKey.Cancel]
            onActivated: savedInfo.close()
        }

        ColumnLayout {
            id: savedCol
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("The current values have been stored to be used as default.")
            }
            RowLayout {
                Layout.fillWidth: true
                Item { Layout.fillWidth: true }
                Button {
                    text: qsTr("OK")
                    onClicked: savedInfo.close()
                }
            }
        }
    }
}
