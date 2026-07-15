/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "youtuberesolver.h"

#include <algorithm>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QThread>
#include <QUrl>
#include <QUrlQuery>

YoutubeResolver::YoutubeResolver(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(this);
    connect(m_process, &QProcess::finished, this, &YoutubeResolver::onFinished);
    connect(m_process, &QProcess::errorOccurred, this,
            &YoutubeResolver::onErrorOccurred);
    connect(m_process, &QProcess::readyReadStandardOutput, this,
            &YoutubeResolver::onReadyRead);
}

YoutubeResolver::~YoutubeResolver()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

void YoutubeResolver::setYtdlPath(const QString &path)
{
    if (path == m_ytdlPath)
        return;
    m_ytdlPath = path;
    emit ytdlPathChanged();
}

void YoutubeResolver::setPreferredHeight(int height)
{
    if (height == m_preferredHeight)
        return;
    m_preferredHeight = height;
    emit preferredHeightChanged();
}

void YoutubeResolver::setCookiesFile(const QString &path)
{
    if (path == m_cookiesFile)
        return;
    m_cookiesFile = path;
    emit cookiesFileChanged();
}

void YoutubeResolver::setFfmpegLocation(const QString &path)
{
    if (path == m_ffmpegLocation)
        return;
    m_ffmpegLocation = path;
    emit ffmpegLocationChanged();
}

void YoutubeResolver::setCacheDir(const QString &path)
{
    if (path == m_cacheDir)
        return;
    m_cacheDir = path;
    emit cacheDirChanged();
    updateCacheCount();
}

void YoutubeResolver::setCacheSize(int maxFiles)
{
    if (maxFiles == m_cacheSize)
        return;
    m_cacheSize = maxFiles;
    emit cacheSizeChanged();
    if (m_cacheSize > 0)
        enforceCacheLimit(); // a lower limit takes effect immediately
}

void YoutubeResolver::setThumbnailOffset(int seconds)
{
    if (seconds == m_thumbnailOffset)
        return;
    m_thumbnailOffset = seconds;
    emit thumbnailOffsetChanged();
}

void YoutubeResolver::setBusy(bool busy)
{
    if (busy == m_busy)
        return;
    m_busy = busy;
    emit busyChanged();
}

void YoutubeResolver::setDownloading(bool downloading)
{
    if (downloading == m_downloading)
        return;
    m_downloading = downloading;
    emit downloadingChanged();
}

void YoutubeResolver::setInstalling(bool installing)
{
    if (installing == m_installing)
        return;
    m_installing = installing;
    emit installingChanged();
}

bool YoutubeResolver::isSupportedUrl(const QString &url)
{
    return !videoId(url).isEmpty();
}

// Extract a YouTube video ID; empty when the URL is not a YouTube video page.
// Ported from SMPlayer's RetrieveYoutubeUrl::getVideoID. Only the video ID is
// taken — any `list`, `start_radio`, `index`, timestamp, etc. are discarded.
QString YoutubeResolver::videoId(const QString &url)
{
    QString fixed = url;
    fixed.replace(QLatin1String("m.youtube.com"), QLatin1String("www.youtube.com"));
    if (!fixed.contains(QLatin1String("://")))
        fixed.prepend(QLatin1String("https://"));

    const QUrl u(fixed);
    const QString host = u.host();
    const QString path = u.path();

    if (host == QLatin1String("youtu.be") || host == QLatin1String("y2u.be"))
        return path.mid(1).section(QLatin1Char('/'), 0, 0); // "/<id>[/…]"

    if (!host.contains(QLatin1String("youtube")))
        return {};

    if (path.contains(QLatin1String("watch"))) {
        const QUrlQuery q(u);
        if (q.hasQueryItem(QLatin1String("v")))
            return q.queryItemValue(QLatin1String("v"));
        if (q.hasQueryItem(QLatin1String("video_ids")))
            return q.queryItemValue(QLatin1String("video_ids"))
                    .section(QLatin1Char(','), 0, 0);
        return {};
    }
    // /shorts/<id>, /live/<id>, /embed/<id>
    static const char *const prefixes[] = { "/shorts/", "/live/", "/embed/" };
    for (const char *prefix : prefixes) {
        const QString p = QLatin1String(prefix);
        if (path.startsWith(p))
            return path.mid(p.length()).section(QLatin1Char('/'), 0, 0);
    }
    return {};
}

// Canonical watch URL for a page URL, dropping extra query params (esp. `list`).
static QString canonicalUrl(const QString &pageUrl)
{
    const QString id = YoutubeResolver::videoId(pageUrl);
    return id.isEmpty()
            ? pageUrl
            : QStringLiteral("https://www.youtube.com/watch?v=%1").arg(id);
}

void YoutubeResolver::resolve(const QString &pageUrl)
{
    cancel();

    const QString requestUrl = canonicalUrl(pageUrl);
    m_pageUrl = QUrl(requestUrl);
    m_op = Op::Resolve;
    m_process->setProcessChannelMode(QProcess::SeparateChannels);
    setBusy(true);

    // -j (dump single JSON), a MUXED format (see the header note), no playlist.
    QString format = QStringLiteral("best");
    if (m_preferredHeight > 0)
        format = QStringLiteral("best[height<=?%1]/best").arg(m_preferredHeight);

    const QStringList args = { QStringLiteral("-j"),
                               QStringLiteral("-f"),
                               format,
                               QStringLiteral("--no-playlist"),
                               QStringLiteral("--no-warnings"),
                               requestUrl };
    m_process->start(m_ytdlPath, args);
}

void YoutubeResolver::download(const QString &pageUrl)
{
    cancel();

    const QString requestUrl = canonicalUrl(pageUrl);
    m_downloadId = videoId(pageUrl);
    m_pageUrl = QUrl(requestUrl);

    const QString dir = cacheDir();
    if (dir.isEmpty() || !QDir().mkpath(dir)) {
        emit failed(tr("Could not create the download cache folder."));
        return;
    }

    // LRU cache hit: replay the already-downloaded file without re-fetching,
    // and mark it as most-recently-used.
    const QString cached = cachedFileForId(m_downloadId);
    if (!cached.isEmpty()) {
        touchFile(cached);
        emit downloaded(QUrl::fromLocalFile(cached),
                        QFileInfo(cached).completeBaseName());
        return;
    }
    if (m_downloadId.isEmpty()) {
        emit failed(tr("Could not determine the video id to download."));
        return;
    }

    // Prefer H.264 video + m4a audio: those merge into MP4 as a clean remux that
    // works with any ffmpeg (incl. old builds) and plays everywhere. Fall back to
    // the best video+audio (VP9/AV1, e.g. for >1080p) then best progressive. The
    // height cap limits the download size when a preferred quality is set.
    QString format = QStringLiteral(
            "bestvideo[vcodec^=avc1]+bestaudio[ext=m4a]/bestvideo+bestaudio/best");
    if (m_preferredHeight > 0)
        format = QStringLiteral(
                "bestvideo[height<=?%1][vcodec^=avc1]+bestaudio[ext=m4a]/"
                "bestvideo[height<=?%1]+bestaudio/best[height<=?%1]/best")
                .arg(m_preferredHeight);

    // Name the output by title so the window shows something friendly; find it
    // afterwards by the trailing "[<id>].mp4".
    const QString outTemplate =
            dir + QStringLiteral("/%(title).100B [%(id)s].%(ext)s");

    QStringList args = { QStringLiteral("-f"), format,
                         QStringLiteral("--merge-output-format"),
                         QStringLiteral("mp4"),
                         QStringLiteral("--no-playlist"),
                         QStringLiteral("--no-part"),
                         QStringLiteral("--no-mtime"),
                         QStringLiteral("--newline"), // progress on its own lines
                         // Save YouTube's poster as the cache thumbnail (usually
                         // the best image; a frame-grab is only a fallback).
                         QStringLiteral("--write-thumbnail"),
                         QStringLiteral("--convert-thumbnails"),
                         QStringLiteral("jpg"),
                         QStringLiteral("-o"), outTemplate };
    if (!m_cookiesFile.isEmpty())
        args << QStringLiteral("--cookies") << m_cookiesFile;
    if (!m_ffmpegLocation.isEmpty())
        args << QStringLiteral("--ffmpeg-location") << m_ffmpegLocation;
    args << requestUrl;

    m_op = Op::Download;
    m_process->setProcessChannelMode(QProcess::MergedChannels); // progress lines
    setBusy(true);
    setDownloading(true);
    m_process->start(m_ytdlPath, args);
}

void YoutubeResolver::cancel()
{
    const bool wasDownload = (m_op == Op::Download);
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
    m_op = Op::None;
    setBusy(false);
    setDownloading(false);
    // A cancelled download may have left partial / intermediate files behind.
    if (wasDownload)
        removeCacheFilesForId(m_downloadId);
}

void YoutubeResolver::onReadyRead()
{
    if (m_op != Op::Download)
        return; // resolve() consumes stdout (JSON) only at finish
    const QString text =
            QString::fromLocal8Bit(m_process->readAllStandardOutput());
    const QStringList lines = text.split(QRegularExpression(QStringLiteral("[\r\n]")),
                                         Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (!trimmed.isEmpty())
            emit progress(trimmed);
    }
}

void YoutubeResolver::onErrorOccurred(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart) {
        const Op op = m_op;
        m_op = Op::None;
        setBusy(false);
        setDownloading(false);
        Q_UNUSED(op);
        emit failed(tr("Could not run yt-dlp (\"%1\"). Check the path in "
                       "Preferences > Network.")
                            .arg(m_ytdlPath));
    }
    // Other errors surface through onFinished / a non-zero exit code.
}

void YoutubeResolver::onFinished(int exitCode, QProcess::ExitStatus status)
{
    const Op op = m_op;
    m_op = Op::None;
    setBusy(false);
    setDownloading(false);
    if (op == Op::Download)
        finishDownload(exitCode, status);
    else
        finishResolve(exitCode, status);
}

void YoutubeResolver::finishResolve(int exitCode, QProcess::ExitStatus status)
{
    if (status != QProcess::NormalExit) {
        emit failed(tr("yt-dlp did not finish normally."));
        return;
    }
    if (exitCode != 0) {
        QString err = QString::fromUtf8(m_process->readAllStandardError())
                              .trimmed();
        if (err.isEmpty())
            err = tr("yt-dlp exited with code %1.").arg(exitCode);
        emit failed(err);
        return;
    }

    // yt-dlp -j prints one JSON object (first line, if several).
    const QByteArray out =
            m_process->readAllStandardOutput().replace('\r', "").trimmed();
    const QByteArray firstLine = out.split('\n').value(0);
    const QJsonObject json = QJsonDocument::fromJson(firstLine).object();

    const QString mediaUrl = json.value(QStringLiteral("url")).toString();
    const QString title = json.value(QStringLiteral("title")).toString();

    if (mediaUrl.isEmpty()) {
        emit failed(tr("yt-dlp returned no playable stream."));
        return;
    }
    emit resolved(QUrl(mediaUrl), title, m_pageUrl);
}

void YoutubeResolver::finishDownload(int exitCode, QProcess::ExitStatus status)
{
    // Prefer a produced file over the exit code (a merge/postprocess hiccup can
    // still leave a playable file, and batch exit codes are unreliable).
    const QString file = cachedFileForId(m_downloadId);
    if (!file.isEmpty()) {
        touchFile(file);       // most-recently-used
        ensureThumbnail(file); // keep the poster, or fall back to a frame
        enforceCacheLimit();   // evict least-recently-used beyond the size cap
        emit downloaded(QUrl::fromLocalFile(file),
                        QFileInfo(file).completeBaseName());
        return;
    }
    // No final file: clean up any partial/intermediate pieces for this id.
    removeCacheFilesForId(m_downloadId);
    if (status != QProcess::NormalExit) {
        emit failed(tr("The download did not finish normally."));
        return;
    }
    emit failed(tr("yt-dlp produced no file (exit code %1). If this is an HD "
                   "video, check the ffmpeg location.").arg(exitCode));
}

// Matches the FINAL merged output "…[<id>].<ext>" — the id followed by exactly
// one extension. Deliberately excludes yt-dlp's intermediates
// ("…[<id>].temp.mp4", "…[<id>].f399.mp4", "…[<id>].f251.webm").
static QRegularExpression finalFileRe(const QString &id)
{
    return QRegularExpression(
            QStringLiteral("\\[%1\\]\\.[A-Za-z0-9]+$")
                    .arg(QRegularExpression::escape(id)));
}

QString YoutubeResolver::cachedFileForId(const QString &id) const
{
    QDir dir(m_cacheDir);
    if (id.isEmpty() || !dir.exists())
        return {};
    const QRegularExpression re = finalFileRe(id);
    QString best;
    QDateTime bestTime;
    for (const QFileInfo &fi : dir.entryInfoList(QDir::Files)) {
        if (!re.match(fi.fileName()).hasMatch())
            continue;
        if (!bestTime.isValid() || fi.lastModified() > bestTime) {
            bestTime = fi.lastModified();
            best = fi.absoluteFilePath();
        }
    }
    return best;
}

void YoutubeResolver::touchFile(const QString &path) const
{
    // Bump the modification time so it counts as most-recently-used. Best-effort
    // (opening the file just to set its time can fail if it's in use).
    QFile f(path);
    if (f.open(QIODevice::ReadWrite)) {
        f.setFileTime(QDateTime::currentDateTime(),
                      QFileDevice::FileModificationTime);
        f.close();
    }
}

// The cached videos (final "…[<id>].<mediaext>" files; excludes yt-dlp's
// intermediates and the .jpg thumbnails).
QList<QFileInfo> YoutubeResolver::finalCacheFiles() const
{
    QList<QFileInfo> files;
    QDir dir(m_cacheDir);
    if (!dir.exists())
        return files;
    static const QRegularExpression re(
            QStringLiteral("\\[[^\\]]+\\]\\.(?:mp4|mkv|webm|m4a|mov)$"),
            QRegularExpression::CaseInsensitiveOption);
    for (const QFileInfo &fi : dir.entryInfoList(QDir::Files))
        if (re.match(fi.fileName()).hasMatch())
            files.append(fi);
    return files;
}

void YoutubeResolver::deleteVideoAndThumbnail(const QString &videoPath) const
{
    // The video may be briefly locked if it was just released by the player
    // (setSource(QUrl()) frees the OS handle asynchronously), so retry a few
    // times. A file that isn't locked is removed on the first try (no delay).
    for (int i = 0; i < 10 && QFileInfo::exists(videoPath); ++i) {
        if (QFile::remove(videoPath))
            break;
        QThread::msleep(80);
    }
    // The sibling thumbnail shares the base name with a .jpg/.webp extension.
    const QFileInfo fi(videoPath);
    const QString base = fi.absolutePath() + QLatin1Char('/') + fi.completeBaseName();
    QFile::remove(base + QStringLiteral(".jpg"));
    QFile::remove(base + QStringLiteral(".webp"));
}

void YoutubeResolver::enforceCacheLimit()
{
    if (m_cacheSize > 0 && !m_cacheDir.isEmpty()) {
        QList<QFileInfo> files = finalCacheFiles();
        if (files.size() > m_cacheSize) {
            std::sort(files.begin(), files.end(),
                      [](const QFileInfo &a, const QFileInfo &b) {
                          return a.lastModified() < b.lastModified(); // oldest 1st
                      });
            for (int i = 0; i < files.size() - m_cacheSize; ++i)
                deleteVideoAndThumbnail(files.at(i).absoluteFilePath());
        }
    }
    updateCacheCount();
}

QString YoutubeResolver::ffmpegBinary() const
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

void YoutubeResolver::ensureThumbnail(const QString &videoPath) const
{
    const QFileInfo fi(videoPath);
    const QString jpg =
            fi.absolutePath() + QLatin1Char('/') + fi.completeBaseName()
            + QStringLiteral(".jpg");
    // yt-dlp's poster (--write-thumbnail) is normally the best image; keep it
    // unless it's missing or (near-)black. Then fall back to a video frame at
    // the configured offset.
    if (QFileInfo::exists(jpg) && !isImageMostlyBlack(jpg))
        return;
    frameGrabThumbnail(videoPath, jpg);
}

void YoutubeResolver::frameGrabThumbnail(const QString &videoPath,
                                         const QString &jpgPath) const
{
    // Grab one frame at the configured offset (default past typical black
    // intros). Detached: playback isn't delayed and the image is ready by the
    // time the cache browser is opened.
    QProcess::startDetached(ffmpegBinary(),
                            { QStringLiteral("-y"),
                              QStringLiteral("-ss"),
                              QString::number(qMax(0, m_thumbnailOffset)),
                              QStringLiteral("-i"), videoPath,
                              QStringLiteral("-frames:v"), QStringLiteral("1"),
                              QStringLiteral("-q:v"), QStringLiteral("3"),
                              QStringLiteral("-update"), QStringLiteral("1"),
                              jpgPath });
}

bool YoutubeResolver::isImageMostlyBlack(const QString &path)
{
    QImage img(path);
    if (img.isNull())
        return true; // unreadable poster -> treat as unusable, fall back
    img = img.scaled(32, 18, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    qint64 sum = 0;
    int n = 0;
    for (int y = 0; y < img.height(); ++y)
        for (int x = 0; x < img.width(); ++x) {
            sum += qGray(img.pixel(x, y));
            ++n;
        }
    // Near-black average (letterboxed posters keep a bright centre, so their
    // mean stays well above this).
    return n > 0 && (sum / n) < 16;
}

void YoutubeResolver::updateCacheCount()
{
    const int count = finalCacheFiles().size();
    if (count == m_cacheCount)
        return;
    m_cacheCount = count;
    emit cacheCountChanged();
}

QVariantList YoutubeResolver::cacheEntries() const
{
    QList<QFileInfo> files = finalCacheFiles();
    // Most-recently-played first.
    std::sort(files.begin(), files.end(),
              [](const QFileInfo &a, const QFileInfo &b) {
                  return a.lastModified() > b.lastModified();
              });
    // "<title> [<id>]" -> title, id.
    static const QRegularExpression nameRe(
            QStringLiteral("^(.*) \\[([^\\]]+)\\]$"));
    QVariantList out;
    for (const QFileInfo &fi : files) {
        const QString baseName = fi.completeBaseName();
        QString title = baseName;
        QString id;
        const QRegularExpressionMatch m = nameRe.match(baseName);
        if (m.hasMatch()) {
            title = m.captured(1);
            id = m.captured(2);
        }
        const QString thumbBase =
                fi.absolutePath() + QLatin1Char('/') + fi.completeBaseName();
        QString thumbUrl;
        for (const QString &ext : { QStringLiteral(".jpg"),
                                    QStringLiteral(".webp") }) {
            if (QFileInfo::exists(thumbBase + ext)) {
                thumbUrl = QUrl::fromLocalFile(thumbBase + ext).toString();
                break;
            }
        }
        const double mb = fi.size() / (1024.0 * 1024.0);
        out.append(QVariantMap{
            { QStringLiteral("title"), title },
            { QStringLiteral("id"), id },
            { QStringLiteral("fileUrl"), QUrl::fromLocalFile(fi.absoluteFilePath()).toString() },
            { QStringLiteral("thumbnailUrl"), thumbUrl },
            { QStringLiteral("sizeText"), QStringLiteral("%1 MB").arg(mb, 0, 'f', 1) },
            { QStringLiteral("path"), fi.absoluteFilePath() },
            // Epoch ms of last-played (for the browser's timestamp sort).
            { QStringLiteral("modified"),
              double(fi.lastModified().toMSecsSinceEpoch()) }
        });
    }
    return out;
}

void YoutubeResolver::removeCacheEntry(const QUrl &fileUrl)
{
    const QString path = fileUrl.toLocalFile();
    if (path.isEmpty() || QFileInfo(path).absolutePath() != QDir(m_cacheDir).absolutePath())
        return; // only delete inside the cache folder
    deleteVideoAndThumbnail(path);
    updateCacheCount();
}

void YoutubeResolver::removeCacheFilesForId(const QString &id) const
{
    QDir dir(m_cacheDir);
    if (id.isEmpty() || !dir.exists())
        return;
    const QString token = QStringLiteral("[%1]").arg(id);
    for (const QFileInfo &fi : dir.entryInfoList(QDir::Files))
        if (fi.fileName().contains(token))
            QFile::remove(fi.absoluteFilePath());
}

// The official yt-dlp release asset for this platform (GitHub "latest").
QString YoutubeResolver::platformDownloadUrl()
{
#if defined(Q_OS_WIN)
    return QStringLiteral("https://github.com/yt-dlp/yt-dlp/releases/latest/"
                          "download/yt-dlp.exe");
#elif defined(Q_OS_MACOS)
    return QStringLiteral("https://github.com/yt-dlp/yt-dlp/releases/latest/"
                          "download/yt-dlp_macos");
#elif defined(Q_OS_LINUX) && defined(__aarch64__)
    return QStringLiteral("https://github.com/yt-dlp/yt-dlp/releases/latest/"
                          "download/yt-dlp_linux_aarch64");
#else
    return QStringLiteral("https://github.com/yt-dlp/yt-dlp/releases/latest/"
                          "download/yt-dlp");
#endif
}

QString YoutubeResolver::defaultBinName()
{
#if defined(Q_OS_WIN)
    return QStringLiteral("yt-dlp.exe");
#else
    return QStringLiteral("yt-dlp");
#endif
}

QString YoutubeResolver::plannedInstallPath() const
{
    // Update an explicitly-configured binary in place; otherwise install into
    // the app data dir (user-writable, no admin) and adopt that path.
    const QFileInfo fi(m_ytdlPath);
    if (fi.isAbsolute())
        return QDir::toNativeSeparators(m_ytdlPath);
    const QString dir =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir::toNativeSeparators(dir + QLatin1Char('/') + defaultBinName());
}

void YoutubeResolver::installOrUpdate()
{
    if (m_installing)
        return;
    setInstalling(true);

    const QString target = QDir::fromNativeSeparators(plannedInstallPath());
    QDir().mkpath(QFileInfo(target).absolutePath());

    const QUrl downloadUrl(platformDownloadUrl());
    QNetworkRequest req(downloadUrl);
    // GitHub's CDN is friendlier to a browser-like UA; releases/latest 302s to
    // the versioned asset, so follow redirects.
    req.setRawHeader("User-Agent",
                     "Mozilla/5.0 (compatible; Vivace yt-dlp installer)");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_net.get(req);
    connect(reply, &QNetworkReply::downloadProgress, this,
            &YoutubeResolver::installProgress);
    connect(reply, &QNetworkReply::finished, this, [this, reply, target] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            setInstalling(false);
            emit installFailed(reply->errorString());
            return;
        }

        // Write atomically, then mark executable on Unix.
        QSaveFile file(target);
        if (!file.open(QIODevice::WriteOnly)) {
            setInstalling(false);
            emit installFailed(tr("Could not write %1.").arg(target));
            return;
        }
        file.write(reply->readAll());
        if (!file.commit()) {
            setInstalling(false);
            emit installFailed(tr("Could not save %1.").arg(target));
            return;
        }
#ifndef Q_OS_WIN
        QFile::setPermissions(target,
                              QFile::permissions(target) | QFileDevice::ExeOwner
                                      | QFileDevice::ExeGroup
                                      | QFileDevice::ExeOther);
#endif
        setInstalling(false);
        setYtdlPath(target); // adopt the freshly installed binary
        emit installFinished(QDir::toNativeSeparators(target));
    });
}
