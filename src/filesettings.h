/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef FILESETTINGS_H
#define FILESETTINGS_H

#include <QObject>
#include <QSettings>
#include <QUrl>

/*  Per-file state for resume playback (SMPlayer's filesettings concept,
    storage redesigned): kept in a separate ini ("vivace_files"), keyed by
    a hash of the url, pruned oldest-first past maxEntries.
*/
class FileSettings : public QObject
{
    Q_OBJECT

public:
    static constexpr int maxEntries = 500;

    explicit FileSettings(QObject *parent = nullptr);

    // Milliseconds to resume from, or -1 when nothing usable is stored.
    qint64 position(const QUrl &url) const;
    void setPosition(const QUrl &url, qint64 positionMs);

    // Track selections; notStored (-2) when nothing is recorded
    // (-1 is a valid subtitle selection meaning "off").
    static constexpr int notStored = -2;
    int audioTrack(const QUrl &url) const;
    int subtitleTrack(const QUrl &url) const;
    void setTracks(const QUrl &url, int audioTrack, int subtitleTrack);

    void remove(const QUrl &url);
    void clearAll();

private:
    static QString keyFor(const QUrl &url);
    void prune();

    mutable QSettings m_store;
};

#endif // FILESETTINGS_H
