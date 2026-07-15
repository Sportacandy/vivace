/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Offline help window, laid out like Qt Creator's help: a table of contents
    on the left, the selected document (rendered Markdown) on the right. Each
    main-menu topic is its own bundled Markdown file; TOC entries and in-page
    links both navigate here, external links open in the browser. A non-modal
    top-level window, so it can stay open while using the player.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: dialog

    title: qsTr("Vivace Help")
    flags: Qt.Dialog
    color: palette.window
    width: 820
    height: 620
    minimumWidth: 480
    minimumHeight: 340

    readonly property string helpRoot: "qrc:/qt/qml/Vivace/help/"
    // Per-language document folder (help/<lang>/), resolved at startup with an
    // English fallback. When a UI-language preference is added, use it here in
    // place of the system locale.
    property string helpBase: helpRoot + "en/"

    function resolveHelpBase() {
        // Follow the UI-language setting; "" means follow the system locale.
        const full = (Settings.uiLanguage !== "" ? Settings.uiLanguage
                                                  : Qt.locale().name)
        // Try the full code first (e.g. pt_BR, zh_CN — help/<lang>/ uses these),
        // then the 2-letter prefix (e.g. de_DE -> de), then fall back to English.
        const candidates = []
        if (full) {
            candidates.push(full)
            const short = full.substring(0, 2)
            if (short !== full)
                candidates.push(short)
        }
        for (var i = 0; i < candidates.length; ++i) {
            const c = candidates[i]
            if (c && c !== "en") {
                const base = helpRoot + c + "/"
                if (UiHelpers.readTextFile(base + "overview.md") !== "")
                    return base // that language has bundled docs
            }
        }
        return helpRoot + "en/"
    }

    Component.onCompleted: helpBase = resolveHelpBase()

    // Table of contents: label shown on the left, and its Markdown file.
    readonly property var topics: [
        { title: qsTr("Getting Started"), file: "overview.md" },
        { title: qsTr("Open"),      file: "menu-open.md" },
        { title: qsTr("Play"),      file: "menu-play.md" },
        { title: qsTr("Video"),     file: "menu-video.md" },
        { title: qsTr("Audio"),     file: "menu-audio.md" },
        { title: qsTr("Subtitles"), file: "menu-subtitles.md" },
        { title: qsTr("Browse"),    file: "menu-browse.md" },
        { title: qsTr("View"),      file: "menu-view.md" },
        { title: qsTr("Options"),   file: "menu-options.md" },
        { title: qsTr("Help"),      file: "menu-help.md" },
        { title: qsTr("Command-line options"), file: "command-line.md" }
    ]

    function topicIndex(file) {
        for (var i = 0; i < topics.length; ++i)
            if (topics[i].file === file)
                return i
        return -1
    }

    // Show a topic by its file name; also selects it in the TOC. Falls back to
    // the English document per file, so a partly-translated language still
    // shows every topic (translated where available, English otherwise).
    function showFile(file) {
        var text = UiHelpers.readTextFile(helpBase + file)
        if (text === "" && helpBase !== helpRoot + "en/")
            text = UiHelpers.readTextFile(helpRoot + "en/" + file)
        if (text === "")
            return
        content.text = text
        if (docScroll.contentItem)
            docScroll.contentItem.contentY = 0 // scroll back to the top
        const idx = topicIndex(file)
        if (idx >= 0)
            toc.currentIndex = idx
    }

    function open() {
        if (transientParent) {
            x = transientParent.x + (transientParent.width - width) / 2
            y = transientParent.y + (transientParent.height - height) / 2
        }
        visible = true // show first, so the content items are realized
        raise()
        requestActivate()
        if (content.text === "")
            showFile(topics[0].file) // first open -> Getting Started
    }

    Shortcut {
        sequences: [StandardKey.Cancel]
        onActivated: dialog.close()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        SplitView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal

            // ---- Table of contents ----
            Frame {
                SplitView.preferredWidth: 200
                SplitView.minimumWidth: 130
                padding: 1
                // Fill the whole pane (incl. the empty space below the items)
                // with the list/item-view background, not the grey frame colour.
                background: Rectangle { color: palette.base }

                ListView {
                    id: toc
                    anchors.fill: parent
                    clip: true
                    model: dialog.topics
                    currentIndex: 0
                    boundsBehavior: Flickable.StopAtBounds
                    ScrollBar.vertical: ScrollBar {}

                    delegate: ItemDelegate {
                        required property int index
                        required property var modelData
                        width: ListView.view.width
                        text: modelData.title
                        highlighted: ListView.isCurrentItem
                        onClicked: dialog.showFile(modelData.file)
                    }
                }
            }

            // ---- Document view ----
            ScrollView {
                id: docScroll
                SplitView.fillWidth: true
                clip: true
                contentWidth: availableWidth // wrap, never scroll horizontally
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                Text {
                    id: content
                    width: docScroll.availableWidth - 8
                    leftPadding: 4
                    rightPadding: 4
                    bottomPadding: 8
                    textFormat: Text.MarkdownText
                    wrapMode: Text.WordWrap
                    color: palette.text
                    onLinkActivated: link => {
                        // In-page links to other help pages open here; every
                        // other link opens in the web browser.
                        if (link.endsWith(".md"))
                            dialog.showFile(link.replace(/^.*\//, ""))
                        else
                            Qt.openUrlExternally(link)
                    }
                    HoverHandler {
                        cursorShape: content.hoveredLink !== ""
                                     ? Qt.PointingHandCursor : Qt.ArrowCursor
                    }
                }
            }
        }

        MenuSeparator { Layout.fillWidth: true }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("Close")
                onClicked: dialog.close()
            }
        }
    }
}
