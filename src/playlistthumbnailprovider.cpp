/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "playlistthumbnailprovider.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUuid>
#include <QVariant>

namespace {
constexpr auto kDbConnectionNamePrefix = "vivace_playlist_thumbnail_cache";
}

PlaylistThumbnailProvider::PlaylistThumbnailProvider(QObject *parent) : QObject(parent)
{
    const QString base =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    // A single file, not a subdirectory -- the cache used to be one file per
    // thumbnail (needing its own folder), but now it's one SQLite database
    // holding every thumbnail as a BLOB (see the class comment), so a
    // dedicated subdirectory would just be a redundant extra path segment.
    m_dbPath = base + QStringLiteral("/playlist_thumbnails.sqlite");

    // A unique-per-instance connection name: this class is a QML singleton
    // (one instance for the process lifetime), but naming it after the
    // object address avoids ever colliding with another QSqlDatabase
    // connection name elsewhere in the app.
    m_dbConnectionName = QStringLiteral("%1_%2").arg(
            QLatin1String(kDbConnectionNamePrefix)).arg(
            reinterpret_cast<quintptr>(this));

    openDatabase();
}

void PlaylistThumbnailProvider::setFfmpegLocation(const QString &location)
{
    if (location == m_ffmpegLocation)
        return;
    m_ffmpegLocation = location;
    emit ffmpegLocationChanged();
}

void PlaylistThumbnailProvider::setMaxCacheEntries(int maxEntries)
{
    if (maxEntries == m_maxCacheEntries)
        return;
    m_maxCacheEntries = maxEntries;
    emit maxCacheEntriesChanged();
    // The new cap might be smaller than the current count -- trim
    // immediately rather than waiting for the next generated thumbnail.
    enforceCacheLimit();
}

QString PlaylistThumbnailProvider::ffmpegBinary() const
{
    const QString name =
#if defined(Q_OS_WIN)
            QStringLiteral("ffmpeg.exe");
#else
            QStringLiteral("ffmpeg");
#endif
    if (m_ffmpegLocation.isEmpty())
        return QStringLiteral("ffmpeg"); // resolved on PATH
    const QFileInfo fi(m_ffmpegLocation);
    // The setting is normally the folder holding ffmpeg; accept the exe too.
    if (fi.isDir())
        return fi.absoluteFilePath() + QLatin1Char('/') + name;
    return m_ffmpegLocation;
}

// The sibling thumbnail convention shared with YoutubeResolver's cache: a
// "<basename>.jpg" next to the media file. Not part of the LRU cache below
// -- it lives next to the user's own media and is never generated/evicted
// by Vivace.
QString PlaylistThumbnailProvider::siblingJpgPath(const QString &mediaPath)
{
    const QFileInfo fi(mediaPath);
    return fi.absolutePath() + QLatin1Char('/') + fi.completeBaseName()
            + QStringLiteral(".jpg");
}

QString PlaylistThumbnailProvider::cacheKeyFor(const QFileInfo &mediaInfo)
{
    // Keyed by path+size+mtime so a file replaced at the same path
    // regenerates instead of showing a stale thumbnail.
    const QString key = mediaInfo.absoluteFilePath() + QLatin1Char('|')
            + QString::number(mediaInfo.size()) + QLatin1Char('|')
            + QString::number(mediaInfo.lastModified().toSecsSinceEpoch());
    return QString::fromLatin1(
            QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha1)
                    .toHex());
}

QString PlaylistThumbnailProvider::dataUrlFor(const QByteArray &jpegBytes)
{
    return QStringLiteral("data:image/jpeg;base64,")
            + QString::fromLatin1(jpegBytes.toBase64());
}

void PlaylistThumbnailProvider::openDatabase()
{
    auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_dbConnectionName);
    db.setDatabaseName(m_dbPath);
    if (!db.open()) {
        qWarning("PlaylistThumbnailProvider: failed to open thumbnail cache "
                  "database (%s) -- generated thumbnails will not persist "
                  "across restarts and cannot be LRU-managed this session.",
                  qUtf8Printable(db.lastError().text()));
        return;
    }
    QSqlQuery q(db);
    // The image itself is stored as a BLOB here rather than as one file per
    // thumbnail in a cache directory -- see the header comment for why (a
    // flat directory of up to maxCacheEntries files isn't a good fit for
    // every filesystem, whereas SQLite gives one indexed file regardless).
    q.exec(QStringLiteral(
            "CREATE TABLE IF NOT EXISTS thumbnails ("
            "  cache_key TEXT PRIMARY KEY,"
            "  image BLOB NOT NULL,"
            "  last_used INTEGER NOT NULL"
            ")"));
    q.exec(QStringLiteral(
            "CREATE INDEX IF NOT EXISTS idx_thumbnails_last_used "
            "ON thumbnails(last_used)"));
}

QByteArray PlaylistThumbnailProvider::blobFor(const QString &key) const
{
    auto db = QSqlDatabase::database(m_dbConnectionName);
    if (!db.isOpen())
        return {};
    QSqlQuery q(db);
    q.prepare(QStringLiteral("SELECT image FROM thumbnails WHERE cache_key = ?"));
    q.addBindValue(key);
    if (!q.exec() || !q.next())
        return {};
    return q.value(0).toByteArray();
}

bool PlaylistThumbnailProvider::touchCacheEntry(const QString &key) const
{
    auto db = QSqlDatabase::database(m_dbConnectionName);
    if (!db.isOpen())
        return false;
    QSqlQuery q(db);
    q.prepare(QStringLiteral("UPDATE thumbnails SET last_used = ? WHERE cache_key = ?"));
    q.addBindValue(QDateTime::currentSecsSinceEpoch());
    q.addBindValue(key);
    return q.exec() && q.numRowsAffected() > 0;
}

void PlaylistThumbnailProvider::storeCacheEntry(const QString &key, const QByteArray &jpegBytes)
{
    auto db = QSqlDatabase::database(m_dbConnectionName);
    if (!db.isOpen())
        return;
    QSqlQuery q(db);
    q.prepare(QStringLiteral(
            "INSERT INTO thumbnails (cache_key, image, last_used) VALUES (?, ?, ?) "
            "ON CONFLICT(cache_key) DO UPDATE SET image = excluded.image, "
            "last_used = excluded.last_used"));
    q.addBindValue(key);
    q.addBindValue(jpegBytes);
    q.addBindValue(QDateTime::currentSecsSinceEpoch());
    q.exec();

    enforceCacheLimit();
}

void PlaylistThumbnailProvider::enforceCacheLimit()
{
    auto db = QSqlDatabase::database(m_dbConnectionName);
    if (!db.isOpen())
        return;

    QSqlQuery countQuery(db);
    countQuery.exec(QStringLiteral("SELECT COUNT(*) FROM thumbnails"));
    if (!countQuery.next())
        return;
    const int total = countQuery.value(0).toInt();
    const int overflow = total - m_maxCacheEntries;
    if (overflow <= 0)
        return;

    // No filesystem cleanup needed -- the image bytes live in the row
    // being deleted, there is nothing else on disk to remove.
    QSqlQuery del(db);
    del.prepare(QStringLiteral(
            "DELETE FROM thumbnails WHERE cache_key IN ("
            "  SELECT cache_key FROM thumbnails ORDER BY last_used ASC LIMIT ?"
            ")"));
    del.addBindValue(overflow);
    del.exec();
}

QString PlaylistThumbnailProvider::thumbnailFor(const QUrl &mediaUrl) const
{
    if (!mediaUrl.isLocalFile())
        return {};
    const QString mediaPath = mediaUrl.toLocalFile();
    const QFileInfo mediaInfo(mediaPath);
    if (!mediaInfo.exists())
        return {};

    const QString sibling = siblingJpgPath(mediaPath);
    if (QFileInfo::exists(sibling))
        return QUrl::fromLocalFile(sibling).toString();

    const QString key = cacheKeyFor(mediaInfo);
    const QByteArray blob = blobFor(key);
    if (!blob.isEmpty()) {
        touchCacheEntry(key); // just shown -- keep it fresh in the LRU order
        return dataUrlFor(blob);
    }

    return {};
}

void PlaylistThumbnailProvider::requestThumbnail(const QUrl &mediaUrl)
{
    if (!mediaUrl.isLocalFile())
        return;
    const QString mediaPath = mediaUrl.toLocalFile();
    if (m_resolved.contains(mediaPath) || m_inFlight.contains(mediaPath)
        || m_queue.contains(mediaPath))
        return;
    if (!thumbnailFor(mediaUrl).isEmpty())
        return; // already have one (sibling or cached)

    m_queue.append(mediaPath);
    processQueue();
}

void PlaylistThumbnailProvider::processQueue()
{
    while (m_running < kMaxConcurrent && !m_queue.isEmpty()) {
        const QString mediaPath = m_queue.takeFirst();
        const QFileInfo mediaInfo(mediaPath);
        if (!mediaInfo.exists()) {
            m_resolved.insert(mediaPath);
            continue;
        }

        const QString key = cacheKeyFor(mediaInfo);
        const QUrl mediaUrl = QUrl::fromLocalFile(mediaPath);
        // ffmpeg needs a real path to write to; this is a short-lived scratch
        // file, deleted as soon as its bytes are read into the database --
        // never kept around like the old one-file-per-thumbnail cache was.
        const QString tempJpgPath = QDir::tempPath() + QStringLiteral("/vivace_thumb_")
                + QUuid::createUuid().toString(QUuid::WithoutBraces) + QStringLiteral(".jpg");

        auto *process = new QProcess(this);
        m_inFlight.insert(mediaPath);
        ++m_running;

        // Guarded against running twice: Qt emits both errorOccurred() and
        // finished() for some failure modes (e.g. a crash), and only
        // errorOccurred() for others (e.g. FailedToStart) -- checking
        // m_inFlight membership makes whichever signal arrives first the
        // one that actually runs the cleanup.
        auto finishOne = [this, process, mediaPath, key, tempJpgPath, mediaUrl] {
            if (!m_inFlight.contains(mediaPath))
                return;
            m_inFlight.remove(mediaPath);
            --m_running;
            m_resolved.insert(mediaPath);
            process->deleteLater();

            QFile file(tempJpgPath);
            if (file.open(QIODevice::ReadOnly)) {
                const QByteArray bytes = file.readAll();
                file.close();
                QFile::remove(tempJpgPath);
                if (!bytes.isEmpty()) {
                    storeCacheEntry(key, bytes);
                    emit thumbnailReady(mediaUrl, dataUrlFor(bytes));
                }
            }
            processQueue();
        };
        connect(process, &QProcess::finished, this,
                [finishOne](int, QProcess::ExitStatus) { finishOne(); });
        connect(process, &QProcess::errorOccurred, this,
                [finishOne](QProcess::ProcessError) { finishOne(); });

        // A short, fixed offset -- past a typical black intro on longer
        // clips, and safely within very short ones too (ffmpeg simply
        // produces no output if -ss lands past end of stream, which the
        // empty-file check above then treats as "no thumbnail" rather than
        // an error).
        //
        // -vf scale=320:-2 caps the grabbed frame at a real thumbnail size
        // instead of the source video's native resolution (confirmed by a
        // real-world repro: a 1920x1080 source produced a 1920x1080, ~280KB
        // thumbnail -- for a row displayed at well under 200px wide). Rows
        // only ever show this at up to editor.expandedThumbHeight (114px
        // tall, ~202px at 16:9), so 320px wide is comfortably above every
        // display size (including the wave-magnified row) with headroom for
        // high-DPI. Beyond the wasted database space, decoding and
        // uploading dozens of full-resolution images at once for a large
        // playlist appears to be what was stalling the row list's repaint
        // until the view was scrolled -- each row's Image had to downscale
        // a full HD/4K frame just to show a few dozen pixels.
        process->start(ffmpegBinary(),
                       { QStringLiteral("-y"), QStringLiteral("-ss"),
                         QStringLiteral("2"), QStringLiteral("-i"), mediaPath,
                         QStringLiteral("-frames:v"), QStringLiteral("1"),
                         QStringLiteral("-vf"), QStringLiteral("scale=w='min(320,iw)':h=-2"),
                         QStringLiteral("-q:v"), QStringLiteral("3"),
                         QStringLiteral("-update"), QStringLiteral("1"),
                         tempJpgPath });
    }
}
