/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef UIHELPERS_H
#define UIHELPERS_H

#include <QObject>
#include <QVariant>
#include <QtQml/qqmlregistration.h>

/*  Formatting helpers for the QML menu delegates: Qt Quick Controls does
    not render mnemonic underlines or shortcut hints, so AppMenuItem draws
    them itself using these.
*/
class UiHelpers : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit UiHelpers(QObject *parent = nullptr);

    // "Ctrl+O" in the platform's notation; accepts whatever Action.shortcut
    // holds (key sequence, StandardKey enum value, or string).
    Q_INVOKABLE QString shortcutText(const QVariant &shortcut) const;

    // "&Open" -> "<u>O</u>pen" (StyledText), "&&" -> "&", HTML escaped.
    Q_INVOKABLE QString mnemonicLabel(const QString &text) const;

    // file:///C:/x -> C:\x (for showing folder-dialog results in the UI).
    Q_INVOKABLE QString toLocalPath(const QUrl &url) const;

    // Hide/show the pointer immediately via the application override cursor
    // (a stationary MouseArea.cursorShape won't re-apply). The caller scopes
    // it to the video so the menu bar and dialogs are never affected.
    Q_INVOKABLE void setCursorHidden(bool hidden);

    // A Windows ".url" internet shortcut is an INI file with
    // [InternetShortcut] / URL=<target>. Returns the target URL string, or an
    // empty string if the file isn't a readable ".url" shortcut. Lets a dropped
    // saved link (e.g. a YouTube page) be played like any other URL.
    Q_INVOKABLE QString readInternetShortcut(const QUrl &fileUrl) const;

    // Reads a UTF-8 text file (used for the bundled Markdown help docs);
    // accepts a qrc/file URL or a plain resource path. Empty on failure.
    Q_INVOKABLE QString readTextFile(const QUrl &url) const;

    // Folder holding vivace's ini files (for the Advanced prefs page).
    Q_INVOKABLE QUrl configFolderUrl() const;

    // The troubleshooting log file (vivace.log in the config folder). Empty
    // when it doesn't exist yet (nothing has been logged this session).
    Q_INVOKABLE QUrl logFileUrl() const;

    // Qt versions for the About dialog: runtime (qVersion) and build-time.
    Q_INVOKABLE QString qtRuntimeVersion() const;
    Q_INVOKABLE QString qtBuildVersion() const;

    // UI-language choices for the Preferences combo: one row per embedded
    // vivace_<lang>.qm (qrc:/i18n), each { code, label } with the language's
    // native name, sorted by label. Excludes "System default"/English, which
    // the QML adds. This keeps the selector in sync with the shipped catalogs.
    Q_INVOKABLE QVariantList availableUiLanguages() const;

    // Supported demuxers/decoders for the media-properties tabs.
    // Rows: { name, description, value (QMediaFormat enum as int) }.
    Q_INVOKABLE QVariantList supportedFileFormats() const;
    Q_INVOKABLE QVariantList supportedVideoCodecs() const;
    Q_INVOKABLE QVariantList supportedAudioCodecs() const;

    // Windows file associations (user-scope, HKCU\Software\Classes; no admin).
    // Whether associations can be managed on this platform at all.
    Q_INVOKABLE bool fileAssociationsSupported() const;
    // The subset of `extensions` (without the dot) currently pointing at
    // Vivace's ProgID.
    Q_INVOKABLE QStringList associatedExtensions(
            const QStringList &extensions) const;
    // Associates `toAssociate` and detaches `toRemove` (extensions without a
    // dot). Returns true when the registry writes succeeded.
    Q_INVOKABLE bool setFileAssociations(const QStringList &toAssociate,
                                         const QStringList &toRemove) const;

private:
    bool m_cursorHidden = false; // application override cursor active?
};

#endif // UIHELPERS_H
