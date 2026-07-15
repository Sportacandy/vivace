/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Preferences > Network, split into subtabs: OpenSubtitles, YouTube
    (yt-dlp streaming + external HD downloader), Proxy (application-wide;
    see NetworkProxyController), and Cast (the Play > Cast > Smartphone/
    tablet server's port; see CastServer). The port lives here rather than
    only in the Cast dialog so a fixed, known port can be opened in advance
    in the OS/router firewall — every desktop OS's own firewall (Windows
    Firewall, Linux ufw/firewalld, macOS's Application Firewall) blocks a
    freshly-listening app on an arbitrary port until the user allows it.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ColumnLayout {
    spacing: 8

    readonly property string helpText: qsTr(
        "<h1>Network</h1>"
        + "<p><b>OpenSubtitles</b> search uses the REST API, which requires a "
        + "free per-application API key: register at opensubtitles.com, create "
        + "an API consumer, and paste the key here. An account login (username "
        + "and password) is optional but raises the daily download limit.</p>"
        + "<p><b>YouTube</b>: the optional resolver uses an external yt-dlp "
        + "program — when enabled, opening a YouTube URL runs yt-dlp to obtain a "
        + "directly-playable stream. Install yt-dlp yourself and, if it is not on "
        + "the PATH, set its full path. Because QMediaPlayer plays a single muxed "
        + "stream, streaming tops out at the best progressive format (about 720p "
        + "on YouTube). Cookies are deliberately NOT used for streaming: an "
        + "authenticated (cookie) session returns URLs a plain player cannot open.</p>"
        + "<p>For <b>HD</b> (and for cookies), enable the external downloader and "
        + "point it at your own tool (a program or .bat/.cmd). HD formats are "
        + "separate video and audio streams that cannot be streamed muxed, so "
        + "Vivace runs the tool to download and merge the video, then plays the "
        + "file it writes into the download folder. The tool keeps its own format "
        + "selection and cookies (add e.g. --cookies to its arguments); Vivace "
        + "only runs it and plays the result.</p>"
        + "<p>See <b>Help ▸ Contents ▸ Options</b> for step-by-step "
        + "instructions on exporting a cookies.txt file from your browser.</p>"
        + "<p>The stream <b>connection timeout</b> (used mainly by live TV "
        + "tuners) has moved to <i>Preferences ▸ TV and radio</i>.</p>"
        + "<p>The <b>Proxy</b> tab applies application-wide: both HTTP and "
        + "SOCKS5 cover OpenSubtitles search and the update check; only HTTP "
        + "additionally covers media playback and yt-dlp (they read the "
        + "http_proxy/https_proxy convention directly instead of going "
        + "through Qt's network stack, and there is no SOCKS5 equivalent for "
        + "that convention).</p>"
        + "<p>The <b>Cast</b> tab sets the port Play ▸ Cast ▸ Smartphone/"
        + "tablet listens on. It's here (rather than only in the Cast dialog) "
        + "so it stays fixed — set it once, allow that port through your "
        + "firewall/router once, and casting keeps working without a new "
        + "firewall prompt every time.</p>")

    TabBar {
        id: tabs
        Layout.fillWidth: true
        TabButton { text: qsTr("OpenSubtitles") }
        TabButton { text: qsTr("YouTube") }
        TabButton { text: qsTr("Proxy") }
        TabButton { text: qsTr("Cast") }
    }

    StackLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        currentIndex: tabs.currentIndex

        // ------------------------------------------- OpenSubtitles
        ScrollView {
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 10

                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("OpenSubtitles")

                    GridLayout {
                        anchors.fill: parent
                        columns: 2
                        columnSpacing: 8
                        rowSpacing: 6

                        RowLayout {
                            spacing: 6
                            Label { text: qsTr("API key:") }
                            HelpMark { text: qsTr("A free per-application key from opensubtitles.com (create an API consumer); subtitle search will not work until this is set.") }
                        }
                        TextField {
                            Layout.fillWidth: true
                            text: Settings.opensubtitlesApiKey
                            placeholderText: qsTr("from opensubtitles.com → API consumers")
                            onEditingFinished: Settings.opensubtitlesApiKey = text
                        }
                        RowLayout {
                            spacing: 6
                            Label { text: qsTr("Username:") }
                            HelpMark { text: qsTr("Optional opensubtitles.com account login; signing in is not required but raises the daily download limit.") }
                        }
                        TextField {
                            Layout.fillWidth: true
                            text: Settings.opensubtitlesUsername
                            onEditingFinished: Settings.opensubtitlesUsername = text
                        }
                        Label { text: qsTr("Password:") }
                        TextField {
                            Layout.fillWidth: true
                            echoMode: TextInput.Password
                            text: Settings.opensubtitlesPassword
                            onEditingFinished: Settings.opensubtitlesPassword = text
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                    font.pixelSize: 12
                    text: qsTr("The account password is stored securely using your "
                               + "operating system's credential manager.")
                }

                Item { Layout.fillHeight: true }
            }
        }

        // ------------------------------------------------- YouTube
        ScrollView {
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 10

                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("YouTube (yt-dlp)")

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        RowLayout {
                            spacing: 6
                            CheckBox {
                                id: ytEnable
                                text: qsTr("Play YouTube URLs with yt-dlp")
                                checked: Settings.youtubeEnabled
                                onToggled: Settings.youtubeEnabled = checked
                            }
                            HelpMark { text: qsTr("When on, opening a YouTube URL runs yt-dlp instead of trying to play the page URL as-is.") }
                            Item { Layout.fillWidth: true }
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            enabled: ytEnable.checked
                            columns: 2
                            columnSpacing: 8
                            rowSpacing: 6

                            RowLayout {
                                spacing: 6
                                Label { text: qsTr("Open YouTube URLs by:") }
                                HelpMark { text: qsTr("Streaming is fastest but caps at ~720p "
                                                      + "and can't use cookies. Downloading gets "
                                                      + "full HD (uses cookies + ffmpeg), then "
                                                      + "plays and deletes the file. Or hand off "
                                                      + "to your own external downloader tool.") }
                            }
                            ComboBox {
                                id: modeCombo
                                Layout.fillWidth: true
                                model: [
                                    qsTr("Streaming (fast, up to ~720p)"),
                                    qsTr("Downloading then playing (HD, cookies)"),
                                    qsTr("An external downloader tool")
                                ]
                                currentIndex: Settings.youtubeMode
                                onActivated: Settings.youtubeMode = currentIndex
                            }

                            RowLayout {
                                spacing: 6
                                Label { text: qsTr("yt-dlp path:") }
                                HelpMark { text: qsTr("Leave as \"yt-dlp\" if it is on your system PATH; otherwise enter the full path to the yt-dlp executable.") }
                            }
                            TextField {
                                Layout.fillWidth: true
                                text: Settings.ytdlPath
                                placeholderText: qsTr("yt-dlp (on PATH) or a full path")
                                onEditingFinished: Settings.ytdlPath = text
                            }
                            RowLayout {
                                spacing: 6
                                Label { text: qsTr("Maximum quality:") }
                                HelpMark { text: qsTr("Upper limit on resolution. Streaming is "
                                                      + "capped at ~720p regardless; downloading "
                                                      + "can reach this height in full HD.") }
                            }
                            ComboBox {
                                id: qualityCombo
                                Layout.fillWidth: true
                                textRole: "label"
                                valueRole: "height"
                                model: [
                                    { label: qsTr("360p"),  height: 360 },
                                    { label: qsTr("480p"),  height: 480 },
                                    { label: qsTr("720p"),  height: 720 },
                                    { label: qsTr("1080p"), height: 1080 },
                                    { label: qsTr("1440p"), height: 1440 },
                                    { label: qsTr("2160p (4K)"), height: 2160 },
                                    { label: qsTr("Best available"), height: 0 }
                                ]
                                Component.onCompleted:
                                    currentIndex = indexOfValue(Settings.youtubeQuality)
                                onActivated: Settings.youtubeQuality = currentValue
                            }
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                    font.pixelSize: 12
                    text: qsTr("yt-dlp is a separate program and is not bundled with Vivace.")
                }

                // ---- Download & play (mode 1) ----
                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Download & play")
                    visible: ytEnable.checked && Settings.youtubeMode === 1

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            opacity: 0.75
                            font.pixelSize: 12
                            text: qsTr("Downloads the video (merging HD video and audio "
                                       + "with ffmpeg), plays it, and keeps it in a cache "
                                       + "folder so replaying it is instant. The cache holds "
                                       + "the most recent downloads up to the size below; "
                                       + "older ones are removed.")
                        }
                        GridLayout {
                            Layout.fillWidth: true
                            columns: 2
                            columnSpacing: 8
                            rowSpacing: 6

                            RowLayout {
                                spacing: 6
                                Label { text: qsTr("Cookies file:") }
                                HelpMark { text: qsTr("Optional cookies.txt exported from your "
                                                      + "browser (yt-dlp --cookies); unlocks HD, "
                                                      + "members-only and age-restricted videos. "
                                                      + "Safe here — cookies only affect the "
                                                      + "download, not a stream a player must open. "
                                                      + "See Help ▸ Contents ▸ Options for "
                                                      + "step-by-step export instructions.") }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6
                                TextField {
                                    Layout.fillWidth: true
                                    text: Settings.youtubeCookiesFile
                                    placeholderText: qsTr("optional cookies.txt")
                                    onEditingFinished: Settings.youtubeCookiesFile = text
                                }
                                Button { text: qsTr("Browse…"); onClicked: cookiesFileDialog.open() }
                            }

                            RowLayout {
                                spacing: 6
                                Label { text: qsTr("ffmpeg location:") }
                                HelpMark { text: qsTr("Folder containing ffmpeg (yt-dlp needs it "
                                                      + "to merge HD video+audio). Leave empty to "
                                                      + "use ffmpeg from the system PATH.") }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6
                                TextField {
                                    Layout.fillWidth: true
                                    text: Settings.youtubeFfmpegLocation
                                    placeholderText: qsTr("empty = use PATH")
                                    onEditingFinished: Settings.youtubeFfmpegLocation = text
                                }
                                Button { text: qsTr("Browse…"); onClicked: ffmpegFolderDialog.open() }
                            }

                            RowLayout {
                                spacing: 6
                                Label { text: qsTr("Cache folder:") }
                                HelpMark { text: qsTr("Where downloaded videos are kept for "
                                                      + "reuse. A video already here is replayed "
                                                      + "without downloading again.") }
                            }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6
                                TextField {
                                    Layout.fillWidth: true
                                    text: Settings.youtubeCacheDir
                                    onEditingFinished: Settings.youtubeCacheDir = text
                                }
                                Button { text: qsTr("Browse…"); onClicked: cacheFolderDialog.open() }
                            }

                            RowLayout {
                                spacing: 6
                                Label { text: qsTr("Cache size:") }
                                HelpMark { text: qsTr("How many downloaded videos to keep. When "
                                                      + "the limit is reached, the "
                                                      + "least-recently-played one is deleted.") }
                            }
                            RowLayout {
                                spacing: 6
                                SpinBox {
                                    from: 1; to: 10000
                                    value: Settings.youtubeCacheSize
                                    onValueModified: Settings.youtubeCacheSize = value
                                }
                                Label { text: qsTr("files") }
                                Item { Layout.fillWidth: true }
                            }

                            RowLayout {
                                spacing: 6
                                Label { text: qsTr("Thumbnail fallback at:") }
                                HelpMark { text: qsTr("The cache thumbnail is normally YouTube's "
                                                      + "own poster image. Only if that is missing "
                                                      + "or black does Vivace grab a video frame — "
                                                      + "this is how many seconds in (past a black "
                                                      + "intro).") }
                            }
                            RowLayout {
                                spacing: 6
                                SpinBox {
                                    from: 0; to: 3600
                                    value: Settings.youtubeThumbnailOffset
                                    onValueModified: Settings.youtubeThumbnailOffset = value
                                }
                                Label { text: qsTr("seconds") }
                                Item { Layout.fillWidth: true }
                            }
                        }
                    }
                }

                // ---- External downloader tool (mode 2) ----
                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("External downloader tool")
                    visible: ytEnable.checked && Settings.youtubeMode === 2

                    GridLayout {
                        anchors.fill: parent
                        columns: 2
                        columnSpacing: 8
                        rowSpacing: 6

                        RowLayout {
                            spacing: 6
                            Label { text: qsTr("Downloader command:") }
                            HelpMark { text: qsTr("The program or .bat/.cmd script that "
                                                  + "downloads the video; it receives the "
                                                  + "URL as an argument.") }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6
                            TextField {
                                Layout.fillWidth: true
                                text: Settings.youtubeDownloaderCommand
                                placeholderText: qsTr("e.g. C:\\Tools\\YouTubeDL.bat")
                                onEditingFinished: Settings.youtubeDownloaderCommand = text
                            }
                            Button { text: qsTr("Browse…"); onClicked: downloaderFileDialog.open() }
                        }

                        RowLayout {
                            spacing: 6
                            Label { text: qsTr("Arguments:") }
                            HelpMark { text: qsTr("Arguments passed to the tool; the token "
                                                  + "{url} is replaced by the video URL "
                                                  + "(if omitted, the URL is appended). Add "
                                                  + "any cookies/format flags your tool needs "
                                                  + "here, e.g. --cookies C:\\path\\cookies.txt.") }
                        }
                        TextField {
                            Layout.fillWidth: true
                            text: Settings.youtubeDownloaderArgs
                            placeholderText: "{url}"
                            onEditingFinished: Settings.youtubeDownloaderArgs = text
                        }

                        RowLayout {
                            spacing: 6
                            Label { text: qsTr("Download folder:") }
                            HelpMark { text: qsTr("The folder the tool writes the finished "
                                                  + "file into; Vivace plays the newest "
                                                  + "media file that appears there, so set "
                                                  + "it to match your tool's output folder.") }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6
                            TextField {
                                Layout.fillWidth: true
                                text: Settings.youtubeDownloadFolder
                                placeholderText: qsTr("your tool's output folder")
                                onEditingFinished: Settings.youtubeDownloadFolder = text
                            }
                            Button { text: qsTr("Browse…"); onClicked: downloadFolderDialog.open() }
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        // --------------------------------------------------- Proxy
        ScrollView {
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 10

                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Proxy")

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 6

                        RowLayout {
                            spacing: 6
                            CheckBox {
                                id: proxyEnable
                                text: qsTr("Enable proxy")
                                checked: Settings.proxyEnabled
                                onToggled: Settings.proxyEnabled = checked
                            }
                            HelpMark { text: qsTr("Applies to OpenSubtitles search and the "
                                                  + "update check (both proxy types). Only an "
                                                  + "HTTP proxy also applies to media playback "
                                                  + "and yt-dlp — a SOCKS5 proxy does not reach "
                                                  + "either, since neither goes through Qt's "
                                                  + "network stack.") }
                            Item { Layout.fillWidth: true }
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            enabled: proxyEnable.checked
                            columns: 2
                            columnSpacing: 8
                            rowSpacing: 6

                            Label { text: qsTr("Host:") }
                            TextField {
                                Layout.fillWidth: true
                                text: Settings.proxyHost
                                onEditingFinished: Settings.proxyHost = text
                            }
                            Label { text: qsTr("Port:") }
                            SpinBox {
                                from: 0; to: 65535
                                editable: true
                                value: Settings.proxyPort
                                onValueModified: Settings.proxyPort = value
                            }
                            Label { text: qsTr("Username:") }
                            TextField {
                                Layout.fillWidth: true
                                text: Settings.proxyUsername
                                onEditingFinished: Settings.proxyUsername = text
                            }
                            Label { text: qsTr("Password:") }
                            TextField {
                                Layout.fillWidth: true
                                echoMode: TextInput.Password
                                text: Settings.proxyPassword
                                onEditingFinished: Settings.proxyPassword = text
                            }
                            Label { text: qsTr("Type:") }
                            ComboBox {
                                Layout.fillWidth: true
                                model: ["HTTP", "SOCKS5"]
                                currentIndex: Settings.proxyType
                                onActivated: Settings.proxyType = currentIndex
                            }
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                    font.pixelSize: 12
                    text: qsTr("The proxy password is stored securely using your "
                               + "operating system's credential manager.")
                }

                Item { Layout.fillHeight: true }
            }
        }

        // ----------------------------------------------------- Cast
        ScrollView {
            clip: true
            contentWidth: availableWidth

            ColumnLayout {
                width: parent.width
                spacing: 10

                GroupBox {
                    Layout.fillWidth: true
                    title: qsTr("Cast")

                    GridLayout {
                        anchors.fill: parent
                        columns: 2
                        columnSpacing: 8
                        rowSpacing: 6

                        RowLayout {
                            spacing: 6
                            Label { text: qsTr("Port:") }
                            HelpMark { text: qsTr("The port Play ▸ Cast ▸ Smartphone/"
                                                  + "tablet listens on. Kept fixed here "
                                                  + "(rather than editable each time in "
                                                  + "the Cast dialog) so you can allow it "
                                                  + "through your firewall/router once and "
                                                  + "it keeps working.") }
                        }
                        SpinBox {
                            Layout.fillWidth: true
                            from: 1024; to: 65535
                            editable: true
                            value: Settings.castPort
                            onValueModified: Settings.castPort = value
                        }
                    }
                }

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                    font.pixelSize: 12
                    text: qsTr("Your OS's firewall (and most routers) block an app "
                               + "listening on a fresh port until you allow it. Pick "
                               + "a port once here, allow Vivace through your "
                               + "firewall for it, and Cast will keep working "
                               + "without a new prompt each time.")
                }

                Item { Layout.fillHeight: true }
            }
        }
    }

    // Non-visual file/folder pickers (shared by the YouTube tab fields).
    FileDialog {
        id: cookiesFileDialog
        title: qsTr("Select the cookies.txt file")
        options: Settings.useNativeFileDialog ? 0 : FileDialog.DontUseNativeDialog
        onAccepted: Settings.youtubeCookiesFile = UiHelpers.toLocalPath(selectedFile)
    }
    FolderDialog {
        id: ffmpegFolderDialog
        title: qsTr("Select the folder containing ffmpeg")
        onAccepted: Settings.youtubeFfmpegLocation = UiHelpers.toLocalPath(selectedFolder)
    }
    FolderDialog {
        id: cacheFolderDialog
        title: qsTr("Select the download cache folder")
        onAccepted: Settings.youtubeCacheDir = UiHelpers.toLocalPath(selectedFolder)
    }
    FileDialog {
        id: downloaderFileDialog
        title: qsTr("Select the downloader program")
        options: Settings.useNativeFileDialog ? 0 : FileDialog.DontUseNativeDialog
        onAccepted: Settings.youtubeDownloaderCommand = UiHelpers.toLocalPath(selectedFile)
    }
    FolderDialog {
        id: downloadFolderDialog
        title: qsTr("Select the download folder")
        onAccepted: Settings.youtubeDownloadFolder = UiHelpers.toLocalPath(selectedFolder)
    }
}
