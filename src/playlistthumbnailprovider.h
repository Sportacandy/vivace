/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Resolves/generates a thumbnail image for a local playlist entry, for
    PlaylistEditor's row thumbnails. Mirrors YoutubeResolver's cache-
    thumbnail convention: a sibling "<basename>.jpg" next to the media file
    is used if present (a real file, on disk, belonging to the user's own
    media -- untouched by any of the below); otherwise a frame is grabbed
    via ffmpeg and the resulting JPEG bytes are stored as a BLOB in a small
    SQLite database (keyed by the file's path+size+mtime, so a file
    replaced at the same path regenerates instead of showing a stale
    thumbnail), returned to QML as a "data:image/jpeg;base64,..." URL. A
    QML singleton since no per-window configuration is needed -- every
    PlaylistEditor instance (docked panel or separate window) shares the
    same cache and in-flight request queue.

    Deliberately NOT one file per generated thumbnail in a cache directory:
    with a cap of up to kMaxCacheEntries (~20000), that would mean a single
    flat directory holding up to 20000 files, which isn't guaranteed to be
    a fast (or even sub-linear) lookup on every filesystem/mount a user
    might have their profile on, and multiplies the cache into thousands of
    loose files a backup/cleanup tool has to reason about. Keeping the
    image bytes in the database instead means the whole cache is one file
    with normal B-tree-indexed lookups by cache_key, regardless of
    filesystem. ffmpeg still needs a real path to write its output to, so
    a short-lived temp file is used per generation and deleted immediately
    after its bytes are read into the database.

    Unlike the YouTube cache (a small, download-heavy FIFO-style eviction --
    see YoutubeResolver::enforceCacheLimit), the playlist can reference a
    huge and very repetitive set of files across many playlists, so
    generated thumbnails are managed as an LRU cache (last_used timestamps
    keyed by the same path+size+mtime hash) capped at maxCacheEntries --
    the least-recently-shown thumbnails are evicted first once the cap is
    exceeded, not the oldest-created ones. maxCacheEntries is bound from
    Settings.playlistThumbnailCacheMaxEntries (Preferences > Playlist >
    Misc), user-configurable rather than a fixed constant.
*/

#ifndef PLAYLISTTHUMBNAILPROVIDER_H
#define PLAYLISTTHUMBNAILPROVIDER_H

#include <QFileInfo>
#include <QObject>
#include <QSet>
#include <QStringList>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

class PlaylistThumbnailProvider : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Reuses the same ffmpeg location as YouTube download mode (the app's
    // one "where's ffmpeg" setting); bound from Settings.youtubeFfmpegLocation
    // in Main.qml. Empty = resolve "ffmpeg"/"ffmpeg.exe" on the system PATH.
    Q_PROPERTY(QString ffmpegLocation READ ffmpegLocation WRITE setFfmpegLocation
                       NOTIFY ffmpegLocationChanged)

    // Bound from Settings.playlistThumbnailCacheMaxEntries in Main.qml
    // (Preferences > Playlist > Misc). Defaults to 20000 (matching
    // Settings' own default) until that binding is established.
    Q_PROPERTY(int maxCacheEntries READ maxCacheEntries WRITE setMaxCacheEntries
                       NOTIFY maxCacheEntriesChanged)

public:
    explicit PlaylistThumbnailProvider(QObject *parent = nullptr);

    QString ffmpegLocation() const { return m_ffmpegLocation; }
    void setFfmpegLocation(const QString &location);

    int maxCacheEntries() const { return m_maxCacheEntries; }
    void setMaxCacheEntries(int maxEntries);

    // A URL string for mediaUrl's thumbnail if already available -- a
    // sibling .jpg next to the media file (a real file:// URL), or a
    // previously generated cache entry (a "data:image/jpeg;base64,..."
    // URL built from the BLOB stored in the database); empty if not yet
    // available -- the caller should also call requestThumbnail() to start
    // generating one. Touches the LRU entry (bumps last_used) whenever it
    // returns a generated-cache hit, since this is effectively "the
    // thumbnail was just shown".
    Q_INVOKABLE QString thumbnailFor(const QUrl &mediaUrl) const;

    // Starts an async ffmpeg frame-grab, stored into the database on
    // success, unless mediaUrl already has a thumbnail (see thumbnailFor())
    // or is already queued/running. Emits thumbnailReady() on success; does
    // nothing on failure (the row simply keeps showing no thumbnail).
    // ffmpeg is resolved via Settings.youtubeFfmpegLocation if set (the
    // app's one "where's ffmpeg" setting), else the system PATH.
    Q_INVOKABLE void requestThumbnail(const QUrl &mediaUrl);

signals:
    void thumbnailReady(const QUrl &mediaUrl, const QString &thumbnailUrl);
    void ffmpegLocationChanged();
    void maxCacheEntriesChanged();

private:
    void processQueue();
    static QString siblingJpgPath(const QString &mediaPath);
    static QString cacheKeyFor(const QFileInfo &mediaInfo);
    static QString dataUrlFor(const QByteArray &jpegBytes);
    QString ffmpegBinary() const;

    void openDatabase();
    QByteArray blobFor(const QString &key) const;
    bool touchCacheEntry(const QString &key) const;
    void storeCacheEntry(const QString &key, const QByteArray &jpegBytes);
    void enforceCacheLimit();

    QString m_ffmpegLocation;
    QString m_dbPath;
    QString m_dbConnectionName;
    QSet<QString> m_resolved; // media paths already found/generated/failed
    QStringList m_queue;      // media paths waiting to be generated
    QSet<QString> m_inFlight; // media paths currently running ffmpeg
    int m_running = 0;
    static constexpr int kMaxConcurrent = 2;
    // "around 20000 or so" (user's original spec, now the Settings
    // default too) -- generous enough for a very large aggregate playlist
    // history without the database growing without bound.
    int m_maxCacheEntries = 20000;
};

#endif // PLAYLISTTHUMBNAILPROVIDER_H
