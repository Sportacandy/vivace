/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Resolves a YouTube page URL to a directly-playable media URL by running an
    external yt-dlp process and reading its JSON (-j) output — the "url" field
    is fed to QMediaPlayer. Modelled on SMPlayer's youtube/retrieveyoutubeurl
    (GPL-2.0-or-later), but simplified for QtMultimedia: we ask yt-dlp for a
    single MUXED stream ("best[height<=?N]/best") because QMediaPlayer cannot
    merge the separate DASH video+audio streams SMPlayer hands to mpv — so the
    resolvable ceiling is whatever progressive format the site offers (≈720p on
    YouTube). Optional feature, off by default; yt-dlp is user-provided.
*/

#ifndef YOUTUBERESOLVER_H
#define YOUTUBERESOLVER_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QtQml/qqmlregistration.h>

class QFileInfo;

class YoutubeResolver : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    // Path to the yt-dlp executable (name only => resolved on PATH).
    Q_PROPERTY(QString ytdlPath READ ytdlPath WRITE setYtdlPath NOTIFY ytdlPathChanged)
    // Preferred maximum video height (e.g. 720); 0 = no cap.
    Q_PROPERTY(int preferredHeight READ preferredHeight WRITE setPreferredHeight
                       NOTIFY preferredHeightChanged)
    // Download mode: cookies.txt (unlocks HD/age-restricted) and the ffmpeg
    // location yt-dlp needs to merge separate HD video+audio (empty = PATH).
    Q_PROPERTY(QString cookiesFile READ cookiesFile WRITE setCookiesFile
                       NOTIFY cookiesFileChanged)
    Q_PROPERTY(QString ffmpegLocation READ ffmpegLocation WRITE setFfmpegLocation
                       NOTIFY ffmpegLocationChanged)
    // Download-mode LRU cache: folder that downloaded videos are kept in, and
    // the max number of files retained (least-recently-used evicted).
    Q_PROPERTY(QString cacheDir READ cacheDir WRITE setCacheDir
                       NOTIFY cacheDirChanged)
    Q_PROPERTY(int cacheSize READ cacheSize WRITE setCacheSize
                       NOTIFY cacheSizeChanged)
    // Seconds into the video to grab the cache thumbnail (past black intros).
    Q_PROPERTY(int thumbnailOffset READ thumbnailOffset WRITE setThumbnailOffset
                       NOTIFY thumbnailOffsetChanged)
    // Number of cached videos (for enabling the cache browser menu item).
    Q_PROPERTY(int cacheCount READ cacheCount NOTIFY cacheCountChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    // True only while a download() (mode 1) is in progress, for the busy overlay
    // (streaming resolve() also sets busy but is quick).
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)
    // True while yt-dlp itself is being downloaded/updated.
    Q_PROPERTY(bool installing READ installing NOTIFY installingChanged)

public:
    explicit YoutubeResolver(QObject *parent = nullptr);
    ~YoutubeResolver() override;

    QString ytdlPath() const { return m_ytdlPath; }
    void setYtdlPath(const QString &path);
    int preferredHeight() const { return m_preferredHeight; }
    void setPreferredHeight(int height);
    QString cookiesFile() const { return m_cookiesFile; }
    void setCookiesFile(const QString &path);
    QString ffmpegLocation() const { return m_ffmpegLocation; }
    void setFfmpegLocation(const QString &path);
    QString cacheDir() const { return m_cacheDir; }
    void setCacheDir(const QString &path);
    int cacheSize() const { return m_cacheSize; }
    void setCacheSize(int maxFiles);
    int thumbnailOffset() const { return m_thumbnailOffset; }
    void setThumbnailOffset(int seconds);
    int cacheCount() const { return m_cacheCount; }

    // The cached videos, most-recently-played first. Each entry is a map with
    // { title, id, fileUrl, thumbnailUrl, sizeText, path } for the cache browser.
    Q_INVOKABLE QVariantList cacheEntries() const;
    // Delete a cached video (and its thumbnail) by its file:// URL.
    Q_INVOKABLE void removeCacheEntry(const QUrl &fileUrl);

    bool busy() const { return m_busy; }
    bool downloading() const { return m_downloading; }
    bool installing() const { return m_installing; }

    // Where installOrUpdate() would place the yt-dlp binary (in-place if
    // ytdlPath is already an absolute file, else the app data dir).
    Q_INVOKABLE QString plannedInstallPath() const;

    // Download the latest official yt-dlp binary for this platform to
    // plannedInstallPath(), then set ytdlPath to it. Emits installProgress /
    // installFinished / installFailed.
    Q_INVOKABLE void installOrUpdate();

    // True for YouTube page URLs (youtube.com / youtu.be family). Mirrors
    // SMPlayer's isUrlSupported, which keys off a resolvable YouTube video ID.
    Q_INVOKABLE static bool isSupportedUrl(const QString &url);

    // The bare video ID for a supported URL (empty otherwise). resolve() uses
    // it to build a canonical watch URL that keeps only the video, dropping
    // extra query parameters — notably `list`, which would otherwise make
    // yt-dlp resolve a whole playlist/radio instead of the single video.
    Q_INVOKABLE static QString videoId(const QString &url);

    // Start resolving to a stream URL; emits resolved() or failed(). A running
    // operation is cancelled first.
    Q_INVOKABLE void resolve(const QString &pageUrl);

    // Download & play mode: run yt-dlp to download an HD file (merging separate
    // video+audio via ffmpeg, using cookies) into a temp folder; emits
    // downloaded() (the local file to play) or failed(), with progress().
    Q_INVOKABLE void download(const QString &pageUrl);
    Q_INVOKABLE void cancel();

signals:
    void ytdlPathChanged();
    void preferredHeightChanged();
    void cookiesFileChanged();
    void ffmpegLocationChanged();
    void cacheDirChanged();
    void cacheSizeChanged();
    void thumbnailOffsetChanged();
    void cacheCountChanged();
    void busyChanged();
    void downloadingChanged();
    // mediaUrl is directly playable; title is the video title; pageUrl echoes
    // the original request.
    void resolved(const QUrl &mediaUrl, const QString &title, const QUrl &pageUrl);
    // A downloaded local file (download mode) to play, with a title from its name.
    void downloaded(const QUrl &fileUrl, const QString &title);
    // A line of yt-dlp's output during a download, for the progress overlay.
    void progress(const QString &line);
    void failed(const QString &message);
    void installingChanged();
    void installProgress(qint64 received, qint64 total);
    void installFinished(const QString &path);
    void installFailed(const QString &message);

private:
    enum class Op { None, Resolve, Download };

    void setBusy(bool busy);
    void setDownloading(bool downloading);
    void setInstalling(bool installing);
    void onFinished(int exitCode, QProcess::ExitStatus status);
    void onErrorOccurred(QProcess::ProcessError error);
    void onReadyRead();
    void finishResolve(int exitCode, QProcess::ExitStatus status);
    void finishDownload(int exitCode, QProcess::ExitStatus status);
    // The cached file for a video id ("…[<id>].<ext>"), or empty if not cached.
    QString cachedFileForId(const QString &id) const;
    void touchFile(const QString &path) const;  // bump mtime (LRU last-used)
    void enforceCacheLimit();                    // evict least-recently-used
    void removeCacheFilesForId(const QString &id) const; // clean partials
    QList<QFileInfo> finalCacheFiles() const;    // the cached videos (not parts)
    void deleteVideoAndThumbnail(const QString &videoPath) const;
    void updateCacheCount();                     // recount + emit
    // Ensure a cache thumbnail exists: keep yt-dlp's downloaded poster if it is
    // usable, else grab a video frame at the offset as a fallback.
    void ensureThumbnail(const QString &videoPath) const;
    void frameGrabThumbnail(const QString &videoPath, const QString &jpgPath) const;
    static bool isImageMostlyBlack(const QString &path); // poster-quality check
    QString ffmpegBinary() const;                // ffmpeg path (location or PATH)
    static QString platformDownloadUrl();
    static QString defaultBinName();

    QProcess *m_process = nullptr;
    QNetworkAccessManager m_net;
    QString m_ytdlPath = QStringLiteral("yt-dlp");
    int m_preferredHeight = 720;
    QString m_cookiesFile;
    QString m_ffmpegLocation;
    QString m_cacheDir;
    int m_cacheSize = 100;
    int m_thumbnailOffset = 10;
    int m_cacheCount = 0;
    bool m_busy = false;
    bool m_downloading = false;
    bool m_installing = false;
    Op m_op = Op::None;              // which operation m_process is running
    QString m_downloadId;            // video id of the in-flight download
    QUrl m_pageUrl; // the request currently in flight
};

#endif // YOUTUBERESOLVER_H
