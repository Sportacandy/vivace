/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "playercontroller.h"

#include <algorithm>
#include <chrono>
#include <numeric>
#include <utility>

#include <QBuffer>
#include <QFile>

#include <QAudioDevice>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QImage>
#include <QLocale>
#include <QMediaMetaData>
#include <QPlaybackOptions>
#include <QRandomGenerator>

#include "httptssource.h"
#include "livestreamdevice.h"
#include <QRegularExpression>
#include <QSize>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QVideoFrame>
#include <QVideoSink>

#include "dvdifoparser.h"
#include "dvdtitledevice.h"
#include "filesettings.h"
#include "playlistparser.h"

namespace {

// Gated DVD-menu trace: set VIVACE_DVD_LOG=1 to append to %TEMP%/vivace_dvd.log.
// (A WIN32 GUI app discards stderr, so a file log is the reliable channel.)
void dvdLog(const QString &message)
{
    static const bool enabled = qEnvironmentVariableIsSet("VIVACE_DVD_LOG");
    if (!enabled)
        return;
    QFile file(QDir::temp().filePath(QStringLiteral("vivace_dvd.log")));
    if (file.open(QIODevice::Append | QIODevice::Text))
        file.write(message.toUtf8() + '\n');
}

// previous() restarts the current file beyond this position instead of
// jumping to the previous playlist entry (matches common player behavior).
constexpr qint64 restartThresholdMs = 3000;

// Resume rules (as in SMPlayer): don't bother for short files, don't save
// positions right at the start, treat near-the-end as watched.
constexpr qint64 resumeMinDurationMs = 2 * 60 * 1000;
constexpr qint64 resumeMinPositionMs = 10 * 1000;
constexpr qreal resumeWatchedFraction = 0.95;

QString formatDuration(qint64 ms)
{
    const qint64 totalSec = ms / 1000;
    return QStringLiteral("%1:%2:%3")
            .arg(totalSec / 3600, 2, 10, u'0')
            .arg((totalSec % 3600) / 60, 2, 10, u'0')
            .arg(totalSec % 60, 2, 10, u'0');
}

QStringList trackLabels(const QList<QMediaMetaData> &tracks)
{
    QStringList labels;
    labels.reserve(tracks.size());
    for (qsizetype i = 0; i < tracks.size(); ++i) {
        const QMediaMetaData &md = tracks.at(i);
        QString label = PlayerController::tr("Track %1").arg(i + 1);
        const QString lang = md.stringValue(QMediaMetaData::Language);
        const QString title = md.stringValue(QMediaMetaData::Title);
        if (!lang.isEmpty())
            label += QStringLiteral(" [%1]").arg(lang);
        if (!title.isEmpty())
            label += QStringLiteral(" ") + title;
        labels << label;
    }
    return labels;
}

// First track whose language matches one of the user's comma-separated
// preferences (codes like "ja" or names like "Japanese"), or -1.
int findTrackByLanguages(const QList<QMediaMetaData> &tracks,
                         const QString &languagesCsv)
{
    static const QRegularExpression separators(
            QStringLiteral("[,;\\s]+"));
    const QStringList tokens = languagesCsv.split(separators, Qt::SkipEmptyParts);

    for (const QString &token : tokens) {
        const QString wanted = token.toLower();

        QString wantedName;
        const QLocale locale(token);
        if (locale.language() != QLocale::C)
            wantedName = QLocale::languageToString(locale.language()).toLower();

        for (qsizetype i = 0; i < tracks.size(); ++i) {
            const QString lang =
                    tracks.at(i).stringValue(QMediaMetaData::Language).toLower();
            if (lang.isEmpty())
                continue;
            if (lang == wanted || lang == wantedName)
                return int(i);
        }
    }
    return -1;
}

// Media file extensions, matching the Open dialog's filter.
const QStringList &videoExtensions()
{
    static const QStringList extensions = {
        QStringLiteral("mp4"), QStringLiteral("mkv"), QStringLiteral("avi"),
        QStringLiteral("mov"), QStringLiteral("webm"), QStringLiteral("wmv"),
        QStringLiteral("ts"), QStringLiteral("m2ts"), QStringLiteral("flv"),
        QStringLiteral("ogv"),
    };
    return extensions;
}

const QStringList &audioExtensions()
{
    static const QStringList extensions = {
        QStringLiteral("mp3"), QStringLiteral("m4a"), QStringLiteral("flac"),
        QStringLiteral("ogg"), QStringLiteral("opus"), QStringLiteral("wav"),
        QStringLiteral("wma"),
    };
    return extensions;
}

const QStringList &mediaExtensions()
{
    static const QStringList extensions = videoExtensions() + audioExtensions();
    return extensions;
}

// Extensions the "Add files from folder" mode (mediaToAdd) accepts:
// 0 none, 1 video, 2 audio, 3 video+audio, 4 consecutive (all media).
QStringList extensionsForMode(int mode)
{
    switch (mode) {
    case 1: return videoExtensions();
    case 2: return audioExtensions();
    default: return mediaExtensions();
    }
}

// The "template" of a file name (extension dropped, digit runs removed), used
// to group consecutive files like ep01 / ep02 (SMPlayer's ConsecutiveFiles).
QString nameTemplate(const QString &fileName)
{
    const qsizetype dot = fileName.lastIndexOf(u'.');
    QString base = dot > 0 ? fileName.left(dot) : fileName;
    QString out;
    out.reserve(base.size());
    for (const QChar c : base)
        if (!c.isDigit())
            out.append(c.toLower());
    return out;
}

QString sessionPlaylistPath()
{
    return QStandardPaths::writableLocation(
                   QStandardPaths::AppDataLocation)
            + QStringLiteral("/session_playlist.m3u8");
}

// Playlist files become their contents; anything else is a media entry.
QList<PlaylistEntry> expandUrls(const QList<QUrl> &urls)
{
    QList<PlaylistEntry> entries;
    for (const QUrl &url : urls) {
        if (PlaylistParser::isPlaylistFile(url))
            entries += PlaylistParser::load(url);
        else
            entries.append({ url, QString() });
    }
    return entries;
}

} // namespace

PlayerController::PlayerController(QObject *parent)
    : QObject(parent),
      m_player(new QMediaPlayer(this)),
      m_audioOutput(new QAudioOutput(this)),
      m_playlist(new PlaylistModel(this)),
      m_recents(new RecentFiles(this)),
      m_fileSettings(new FileSettings(this)),
      m_mediaDevices(new QMediaDevices(this))
{
    // TV/radio channel lists live next to vivace_files.ini, stored as
    // m3u8 like SMPlayer's.
    const QString configDir =
            QFileInfo(QSettings(QSettings::IniFormat, QSettings::UserScope,
                                QStringLiteral("vivace-player"),
                                QStringLiteral("vivace_files"))
                              .fileName())
                    .absolutePath();
    m_tvChannels =
            new FavoritesModel(configDir + QStringLiteral("/tv.json"), this);
    m_radioChannels =
            new FavoritesModel(configDir + QStringLiteral("/radio.json"), this);
    m_favorites =
            new FavoritesModel(configDir + QStringLiteral("/favorites.json"), this);
    m_bookmarks = new Bookmarks(this);
    m_screenSaver = new ScreenSaver(this);

    m_player->setAudioOutput(m_audioOutput);

    // Keep the screensaver / display awake while a video is actually playing
    // (SMPlayer's disable_screensaver; audio-only playback is left alone).
    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this, &PlayerController::updateScreensaver);
    connect(m_player, &QMediaPlayer::hasVideoChanged,
            this, &PlayerController::updateScreensaver);

    // The unified chapter list changes with the DVD title/chapter state too.
    connect(this, &PlayerController::dvdPlaybackChanged,
            this, &PlayerController::chaptersChanged);

    // The active audio output device keys the per-device global A/V delay.
    connect(m_audioOutput, &QAudioOutput::deviceChanged,
            this, &PlayerController::currentAudioDeviceChanged);

    // Seek-preview: a hidden, silent second player (no audio output) grabs the
    // frame at a hovered position. It is primed (play -> pause on first frame)
    // so subsequent setPosition() calls deliver paused-seek frames.
    m_previewPlayer = new QMediaPlayer(this);
    m_previewSink = new QVideoSink(this);
    m_previewPlayer->setVideoSink(m_previewSink);
    connect(m_previewSink, &QVideoSink::videoFrameChanged,
            this, &PlayerController::onPreviewFrameChanged);
    connect(m_previewPlayer, &QMediaPlayer::mediaStatusChanged, this,
            [this](QMediaPlayer::MediaStatus status) {
                if (status == QMediaPlayer::LoadedMedia && !m_previewReady) {
                    m_previewPriming = true;
                    m_previewPlayer->play(); // decode to the first frame, then pause
                }
            });
    // Diagnostic: the seek-preview player opens the same file as the main one.
    // Its errors are not surfaced to the UI, but log them so we can tell whether
    // a reported playback error actually originates here.
    connect(m_previewPlayer, &QMediaPlayer::errorOccurred, this,
            [](QMediaPlayer::Error error, const QString &errorString) {
                qWarning() << "preview player error:" << int(error) << errorString;
            });
    connect(m_player, &QMediaPlayer::hasVideoChanged,
            this, &PlayerController::updateSeekPreviewSource);

    // A/V sync: present delayed video frames on time, and drop stale queued
    // frames on seek/source change.
    m_videoDelayTimer.setSingleShot(true);
    connect(&m_videoDelayTimer, &QTimer::timeout,
            this, &PlayerController::presentDueFrames);
    connect(this, &PlayerController::seeked,
            this, &PlayerController::flushVideoDelayQueue);
    connect(m_player, &QMediaPlayer::sourceChanged,
            this, &PlayerController::flushVideoDelayQueue);

    connect(m_mediaDevices, &QMediaDevices::audioOutputsChanged, this, [this]() {
        emit audioDevicesChanged();
        applyAudioDevice(); // the selected device may have (dis)appeared
    });

    applyStreamOptions(); // network timeout for streams (no probeSize inflation)

    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, &PlayerController::handleMediaStatus);
    connect(m_player, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error error, const QString &errorString) {
                qWarning() << "playback error:" << int(error) << errorString
                           << "| source=" << m_player->source().toString()
                           << "| status=" << int(m_player->mediaStatus())
                           << "| hasVideo=" << m_player->hasVideo()
                           << "| previewSrc=" << m_previewPlayer->source().toString();
                // "Ignore playback errors": skip a broken file and continue
                // the playlist instead of stopping (SMPlayer's ignore_errors).
                if (m_ignorePlaybackErrors && !m_dvdDevice) {
                    const int index = pickNextIndex();
                    if (index >= 0 && index != m_playlist->currentIndex()) {
                        playAt(index);
                        return;
                    }
                }
                emit errorMessage(errorString);
            });
    connect(m_player, &QMediaPlayer::tracksChanged,
            this, &PlayerController::trackLabelsChanged);
    connect(m_player, &QMediaPlayer::activeTracksChanged,
            this, &PlayerController::activeTracksChanged);

    // Learn the playing entry's duration for the playlist's duration column.
    connect(m_player, &QMediaPlayer::durationChanged, this, [this](qint64 d) {
        if (d > 0 && !m_player->sourceDevice())
            m_playlist->setDuration(m_playlist->currentIndex(), d);
    });

    // A-B repeat: loop back to A once playback reaches B — but only while
    // Repeat is on (the A-B section's Repeat toggle, = m_repeatAll), matching
    // SMPlayer where the A-B section loops only when loop/repeat is enabled.
    connect(m_player, &QMediaPlayer::positionChanged, this, [this](qint64 pos) {
        // Undo a spurious rewind-to-start on the first play-after-pause (a Qt
        // FFmpeg backend quirk seen on a cold first run). Checked on the first
        // tick after resuming; a large backward jump is restored to where the
        // user paused. Legitimate resumes (position unchanged) fall through.
        if (m_resumeGuardPos >= 0) {
            const qint64 guard = m_resumeGuardPos;
            m_resumeGuardPos = -1;
            if (pos < guard - 3000 && m_player->isSeekable()) {
                m_player->setPosition(guard);
                emit seeked(guard);
                // Leave m_smoothPosition at the paused value so the seek bar
                // never shows the blip; the setPosition above re-emits at guard.
                return;
            }
        }
        if (m_repeatAll && m_abMarkerA >= 0 && m_abMarkerB > m_abMarkerA
            && pos >= m_abMarkerB && m_player->isSeekable()) {
            m_player->setPosition(m_abMarkerA);
            emit seeked(m_abMarkerA);
        }
        if (m_smoothPosition != pos) {
            m_smoothPosition = pos;
            emit smoothPositionChanged();
        }
        updateSubtitle(pos); // keep the external-subtitle overlay in sync
    });

    // Arm the resume guard when resuming from pause (skip DVD and A-B looping,
    // which legitimately jump the position). Capture the position at the resume
    // transition, so a seek-while-paused is respected rather than undone.
    connect(m_player, &QMediaPlayer::playbackStateChanged, this,
            [this](QMediaPlayer::PlaybackState state) {
                if (state == QMediaPlayer::PlayingState
                    && m_lastPlaybackState == QMediaPlayer::PausedState
                    && !m_dvdDevice && m_abMarkerA < 0) {
                    const qint64 p = m_player->position();
                    if (p > 3000)
                        m_resumeGuardPos = p;
                }
                m_lastPlaybackState = state;
            });

    connect(m_player, &QMediaPlayer::sourceChanged,
            this, &PlayerController::mediaInfoChanged);
    // A-B markers and external subtitles are per-file; reset on media change.
    connect(m_player, &QMediaPlayer::sourceChanged, this, [this]() {
        if (m_abMarkerA >= 0 || m_abMarkerB >= 0) {
            m_abMarkerA = -1;
            m_abMarkerB = -1;
            emit abMarkersChanged();
        }
        if (!m_externalSubs.isEmpty() || m_subtitleDelayMs != 0) {
            m_externalSubs.clear();
            m_subtitleDelayMs = 0;
            updateSubtitle(0);
            emit externalSubtitlesChanged();
            emit subtitleDelayChanged();
        }
        resetVideoTransform(); // aspect/zoom/pan/rotate/flip/mirror per file
        updateSeekPreviewSource();

        // The friendly title only survives the source change that openStream()
        // primes it for; every other source reverts to a URL-derived title.
        const QString title = m_pendingStreamTitle;
        m_pendingStreamTitle.clear();
        if (title != m_mediaTitle) {
            m_mediaTitle = title;
            emit mediaTitleChanged();
        }

        // The per-file audio delay is reset each file; the global delay stays.
        if (m_fileAudioDelay != 0) {
            m_fileAudioDelay = 0;
            applyVideoDelayRouting();
            emit audioDelayChanged();
        }

        // Parse chapters for local file sources (skip DVD/device and streams).
        m_chapters.clear();
        if (!m_dvdDevice) {
            const QUrl url = m_player->source();
            if (url.isLocalFile())
                m_chapters = ChapterParse::chapters(url.toLocalFile());
        }
        emit chaptersChanged();
    });
    connect(m_player, &QMediaPlayer::metaDataChanged,
            this, &PlayerController::mediaInfoChanged);
    // DVD menu button rectangles live in the menu video's pixel space; track it
    // from the actual resolution so the QML overlay aligns (720x480 / 720x576).
    connect(m_player, &QMediaPlayer::metaDataChanged, this, [this]() {
        if (m_menuVts < 0)
            return;
        const QSize res = m_player->metaData()
                                  .value(QMediaMetaData::Resolution).toSize();
        if (res.width() > 0 && res.height() > 0
            && (res.width() != m_menuSpaceW || res.height() != m_menuSpaceH)) {
            m_menuSpaceW = res.width();
            m_menuSpaceH = res.height();
            emit dvdMenuChanged();
        }
    });
    connect(m_player, &QMediaPlayer::tracksChanged,
            this, &PlayerController::mediaInfoChanged);
    connect(m_player, &QMediaPlayer::durationChanged,
            this, &PlayerController::mediaInfoChanged);
    connect(m_player, &QMediaPlayer::hasVideoChanged,
            this, &PlayerController::mediaInfoChanged);
    connect(m_player, &QMediaPlayer::hasAudioChanged,
            this, &PlayerController::mediaInfoChanged);
}

PlayerController::~PlayerController()
{
    saveCurrentPosition();
    saveSessionPlaylist();
}

QObject *PlayerController::videoOutput() const
{
    return m_videoOutputItem;
}

void PlayerController::setVideoOutput(QObject *videoOutput)
{
    if (videoOutput == m_videoOutputItem)
        return;
    m_videoOutputItem = videoOutput;
    m_targetVideoSink = videoOutput
            ? videoOutput->property("videoSink").value<QVideoSink *>()
            : nullptr;
    applyVideoDelayRouting();
    emit videoOutputChanged();
}

void PlayerController::updateSeekPreviewSource()
{
    const QUrl src = m_player->source();
    const bool localFile = !m_dvdDevice && src.isLocalFile();
    if (localFile) {
        if (m_previewPlayer->source() != src) {
            m_previewReady = false;
            m_previewPriming = false;
            m_previewPendingMs = -1;
            m_previewPlayer->setSource(src);
        }
    } else if (!m_previewPlayer->source().isEmpty()) {
        m_previewReady = false;
        m_previewPlayer->setSource(QUrl());
    }
    const bool avail = localFile && m_player->hasVideo();
    if (avail != m_seekPreviewAvailable) {
        m_seekPreviewAvailable = avail;
        emit seekPreviewAvailableChanged();
    }
}

void PlayerController::onPreviewFrameChanged()
{
    if (m_previewPriming) {
        // First frame after priming: pause so later setPosition() calls act as
        // paused seeks, and honour any request that arrived while loading.
        m_previewPlayer->pause();
        m_previewPriming = false;
        m_previewReady = true;
        if (m_previewPendingMs >= 0) {
            m_previewPlayer->setPosition(m_previewPendingMs);
            m_previewPendingMs = -1;
        }
        return;
    }
    if (m_previewTargetSink)
        m_previewTargetSink->setVideoFrame(m_previewSink->videoFrame());
}

void PlayerController::setPreviewVideoOutput(QObject *item)
{
    m_previewTargetSink =
        item ? item->property("videoSink").value<QVideoSink *>() : nullptr;
}

void PlayerController::requestSeekPreview(qint64 ms)
{
    if (!m_seekPreviewAvailable)
        return;
    if (!m_previewReady) {
        m_previewPendingMs = ms; // applied once priming finishes
        return;
    }
    m_previewPlayer->setPosition(ms);
}

// Effective delay = global (device) + per-file. Only the video-delay direction
// (audio earlier) is implemented, so the sum is clamped to <= 0.
int PlayerController::effectiveAudioDelay() const
{
    return qBound(-10000, m_globalAudioDelay + m_fileAudioDelay, 0);
}

void PlayerController::setGlobalAudioDelay(int ms)
{
    ms = qBound(-10000, ms, 0);
    if (ms == m_globalAudioDelay)
        return;
    m_globalAudioDelay = ms;
    applyVideoDelayRouting();
    emit audioDelayChanged();
}

void PlayerController::setFileAudioDelay(int ms)
{
    // The per-file value may be positive (to offset a negative global); the
    // effective sum is what gets clamped.
    ms = qBound(-10000, ms, 10000);
    if (ms == m_fileAudioDelay)
        return;
    m_fileAudioDelay = ms;
    applyVideoDelayRouting();
    emit audioDelayChanged();
}

void PlayerController::adjustFileAudioDelay(int deltaMs)
{
    // The +/- steps show an OSD (the effective delay); the per-file reset on
    // load does not.
    setFileAudioDelay(m_fileAudioDelay + deltaMs);
    emit osdMessage(tr("A/V delay: %1 ms").arg(effectiveAudioDelay()));
}

// Route the player's video either straight to the VideoOutput (no delay) or
// through our intercept sink, from which frames are presented |audioDelay| ms
// late. Qt has no A/V-offset API, so this is the mechanism (see the A/V-sync
// research note in CLAUDE.md).
void PlayerController::applyVideoDelayRouting()
{
    const bool delayVideo = effectiveAudioDelay() < 0 && m_targetVideoSink;
    if (delayVideo) {
        if (!m_videoDelaySink) {
            m_videoDelaySink = new QVideoSink(this);
            connect(m_videoDelaySink, &QVideoSink::videoFrameChanged, this,
                    &PlayerController::enqueueDelayedFrame);
            m_videoClock.start();
        }
        // Feed frames into the intercept sink instead of the display.
        if (m_player->videoSink() != m_videoDelaySink) {
            flushVideoDelayQueue();
            m_player->setVideoSink(m_videoDelaySink);
        }
    } else {
        // Direct: the player renders straight to the VideoOutput.
        flushVideoDelayQueue();
        if (m_videoOutputItem)
            m_player->setVideoOutput(m_videoOutputItem);
    }
}

void PlayerController::enqueueDelayedFrame(const QVideoFrame &frame)
{
    const int delay = effectiveAudioDelay();
    if (delay >= 0 || !m_targetVideoSink)
        return;
    m_videoQueue.append({ m_videoClock.elapsed() - delay, frame });
    if (!m_videoDelayTimer.isActive())
        presentDueFrames();
}

void PlayerController::presentDueFrames()
{
    const qint64 now = m_videoClock.elapsed();
    while (!m_videoQueue.isEmpty() && m_videoQueue.first().dueMs <= now) {
        if (m_targetVideoSink)
            m_targetVideoSink->setVideoFrame(m_videoQueue.first().frame);
        m_videoQueue.removeFirst();
    }
    if (!m_videoQueue.isEmpty()) {
        const qint64 wait = m_videoQueue.first().dueMs - m_videoClock.elapsed();
        m_videoDelayTimer.start(int(qMax(qint64(0), wait)));
    }
}

void PlayerController::flushVideoDelayQueue()
{
    m_videoQueue.clear();
    m_videoDelayTimer.stop();
}

QStringList PlayerController::videoTrackLabels() const
{
    return trackLabels(m_player->videoTracks());
}

QStringList PlayerController::audioTrackLabels() const
{
    return trackLabels(m_player->audioTracks());
}

QStringList PlayerController::subtitleTrackLabels() const
{
    return trackLabels(m_player->subtitleTracks());
}

void PlayerController::open(const QList<QUrl> &urls)
{
    if (urls.isEmpty())
        return;

    // A dropped/opened folder is either a DVD (VIDEO_TS) or a media folder.
    if (urls.size() == 1 && urls.first().isLocalFile()
        && QFileInfo(urls.first().toLocalFile()).isDir()) {
        if (!openDvd(urls.first()))
            openDirectory(urls.first(), false);
        return;
    }

    // "Automatically add files from the same folder": opening one local
    // media file queues its siblings (filtered by the "Add files from
    // folder" mode) and starts at the chosen one. Mode 0 (None) skips this.
    if (m_autoAddFolderFiles && m_mediaToAdd != 0 && urls.size() == 1
        && urls.first().isLocalFile()
        && !PlaylistParser::isPlaylistFile(urls.first())) {
        const QFileInfo fileInfo(urls.first().toLocalFile());
        if (fileInfo.isFile()
            && mediaExtensions().contains(fileInfo.suffix().toLower())) {
            const QDir dir = fileInfo.dir();
            const QStringList exts = extensionsForMode(m_mediaToAdd);
            QStringList nameFilters;
            for (const QString &ext : exts)
                nameFilters << QStringLiteral("*.") + ext;
            QStringList names = dir.entryList(
                    nameFilters, QDir::Files, QDir::Name | QDir::LocaleAware);

            // Consecutive files: keep only those sharing the opened file's
            // name template (e.g. ep01/ep02), plus the opened file itself.
            if (m_mediaToAdd == 4) {
                const QString wanted = nameTemplate(fileInfo.fileName());
                names.removeIf([&wanted](const QString &n) {
                    return nameTemplate(n) != wanted;
                });
            }

            if (names.size() > 1) {
                QList<PlaylistEntry> entries;
                for (const QString &name : names)
                    entries.append({ QUrl::fromLocalFile(
                            dir.absoluteFilePath(name)), QString() });
                m_player->stop();
                m_playlist->clear();
                m_playlist->add(entries);
                // add() may reorder (auto sort), so locate the opened file.
                const QUrl openedUrl =
                        QUrl::fromLocalFile(fileInfo.absoluteFilePath());
                int startIndex = 0;
                for (int i = 0; i < m_playlist->count(); ++i) {
                    if (m_playlist->urlAt(i) == openedUrl) {
                        startIndex = i;
                        break;
                    }
                }
                playIndex(startIndex, /*resume=*/true);
                return;
            }
        }
    }

    const bool openedPlaylist = urls.size() == 1
            && PlaylistParser::isPlaylistFile(urls.first());
    const QList<PlaylistEntry> entries = expandUrls(urls);
    if (entries.isEmpty())
        return;

    m_player->stop();
    m_playlist->clear();
    m_playlist->add(entries);
    // "Start playback after loading a playlist" governs playlist files only;
    // opening plain media always plays.
    if (!openedPlaylist || m_playOnLoadPlaylist)
        playIndex(m_playlist->currentIndex() >= 0 ? m_playlist->currentIndex() : 0,
                  /*resume=*/true);
}

void PlayerController::openStream(const QUrl &mediaUrl, const QString &title)
{
    if (mediaUrl.isEmpty())
        return;

    // Adopted by the sourceChanged handler so the window/playlist show the
    // video title instead of the raw stream URL.
    m_pendingStreamTitle = title;

    m_player->stop();
    m_playlist->clear();
    m_playlist->add({ { mediaUrl, title } });
    playAt(0);
}

void PlayerController::enqueue(const QList<QUrl> &urls)
{
    if (urls.isEmpty())
        return;

    const QList<PlaylistEntry> entries = expandUrls(urls);
    if (entries.isEmpty())
        return;

    const int firstAdded = m_playlist->count();
    m_playlist->add(entries);
    if (m_playlist->currentIndex() == -1)
        playAt(firstAdded);
}

void PlayerController::playAt(int index)
{
    // Choosing an item in the playlist plays it from the beginning; resuming
    // where a file was left off is reserved for "opening" it (see open()).
    playIndex(index, /*resume=*/false);
}

void PlayerController::playIndex(int index, bool resume)
{
    const QUrl url = m_playlist->urlAt(index);
    if (url.isEmpty())
        return;

    m_resumeOnLoad = resume; // consulted by handleMediaStatus on LoadedMedia
    saveCurrentPosition();
    m_playlist->setCurrentIndex(index);

    // Keep the shuffle cursor aligned when the user jumps to an item directly.
    if (m_shuffle && m_playlist->count() > 1) {
        if (m_shuffleOrder.size() != m_playlist->count())
            fillShuffleOrder();
        const int pos = m_shuffleOrder.indexOf(index);
        if (pos >= 0)
            m_shufflePos = pos;
    }

    // Detach any DVD device before switching (setSource/setSourceDevice below
    // replaces the source anyway, but clean up our device object + state).
    if (m_dvdDevice) {
        m_dvdDevice->deleteLater();
        m_dvdDevice = nullptr;
        m_dvdCurrentTitle = -1;
        m_player->setLoops(QMediaPlayer::Once);
        if (m_menuVts >= 0) {
            m_menuVts = -1;
            m_menuButtons = {};
            m_menuSelected = 0;
            emit dvdMenuChanged();
        }
        m_menus = {};
        emit dvdPlaybackChanged();
    }

    const QString scheme = url.scheme().toLower();
    if (scheme == QLatin1String("http") || scheme == QLatin1String("https")) {
        // Route HTTP(S) through the buffered TS source; it plays raw MPEG-TS
        // (TV tuners) via our device and hands everything else back for direct
        // playback. See HttpTsSource.
        startHttpTsStream(url);
        return;
    }

    teardownHttpTsSource(); // leaving a stream for a local/other source
    m_player->setSource(url);
    m_pendingAutoPlay = true;
    m_player->play();
}

bool PlayerController::openDvd(const QUrl &folder)
{
    if (!folder.isLocalFile())
        return false;

    QDir dir(folder.toLocalFile());
    if (dir.dirName().compare(QStringLiteral("VIDEO_TS"), Qt::CaseInsensitive) != 0
        && dir.exists(QStringLiteral("VIDEO_TS"))) {
        dir.cd(QStringLiteral("VIDEO_TS"));
    }

    // Parse the whole disc structure up front; playback follows the
    // longest title. Discs without parseable IFOs fall back to the raw
    // VOB chain of the largest title set.
    const QString videoTs = dir.absolutePath();
    const QList<DvdIfo::Title> titles = DvdIfo::titles(videoTs);

    m_dvdDir = videoTs;
    m_dvdTitles = titles;
    m_menus = DvdMenu::parse(videoTs);
    m_vm.reset();

    for (const DvdIfo::Title &title : titles) {
        qInfo().nospace() << "dvd: title " << title.titleNumber << " vts "
                          << title.vtsNumber << " duration "
                          << title.durationMs << "ms cells "
                          << title.cells.size() << " chapters "
                          << title.chapterStartsMs.size() << " tmap "
                          << title.timeMapSectors.size() << "x"
                          << title.timeMapUnitSec << "s";
        for (const DvdIfo::Cell &cell : title.cells) {
            qInfo().nospace() << "dvd:   cell sectors " << cell.firstSector
                              << ".." << cell.lastSector;
        }
    }

    if (!titles.isEmpty()) {
        // Debug aids: VIVACE_DVD_TITLE forces a title number,
        // VIVACE_DVD_CHAPTER additionally starts at a chapter (1-based),
        // VIVACE_DVD_SEEK seeks to a title-global time (ms) after load.
        const qint64 debugSeek = qgetenv("VIVACE_DVD_SEEK").toLongLong();
        if (debugSeek > 0) {
            QTimer::singleShot(2500, this,
                               [this, debugSeek]() { seekDvd(debugSeek); });
        }
        const QByteArray forced = qgetenv("VIVACE_DVD_TITLE");
        if (!forced.isEmpty()) {
            for (const DvdIfo::Title &title : titles) {
                if (title.titleNumber != forced.toInt())
                    continue;
                const int chapter = qgetenv("VIVACE_DVD_CHAPTER").toInt() - 1;
                if (chapter >= 0 && chapter < title.chapterCellIndexes.size()) {
                    return applyDvdTitle(title,
                                         title.chapterCellIndexes.at(chapter),
                                         title.chapterStartsMs.at(chapter));
                }
                return applyDvdTitle(title);
            }
        }

        // Experimental DVD menus: if the disc has a navigable menu, start
        // there (as SMPlayer/mpv do via libdvdnav) instead of jumping straight
        // into the longest title. VIVACE_DVD_NOMENU forces the old behaviour.
        if (m_menus.hasMenus() && m_dvdMenusEnabled
            && qgetenv("VIVACE_DVD_NOMENU").isEmpty()) {
            // First-Play (optional): run the disc's on-insert sequence (logos /
            // warnings / intro that may jump to a menu or auto-play a title).
            if (m_dvdUseFirstPlay && m_menus.hasFirstPlay() && runFirstPlay())
                return true;
            if (enterDefaultMenu())
                return true;
            qWarning() << "dvd: menu entry failed; falling back to longest title";
        }

        const auto main = std::max_element(
                titles.constBegin(), titles.constEnd(),
                [](const DvdIfo::Title &a, const DvdIfo::Title &b) {
                    return a.durationMs < b.durationMs;
                });
        if (applyDvdTitle(*main))
            return true;
    }

    // Fallback: no usable IFO structure.
    DvdTitleDevice *device = DvdTitleDevice::create(videoTs, this);
    if (!device)
        return false;
    if (!device->open(QIODevice::ReadOnly)) {
        delete device;
        return false;
    }

    saveCurrentPosition();
    m_player->stop();
    m_playlist->clear();

    if (m_dvdDevice)
        m_dvdDevice->deleteLater();
    m_dvdDevice = device;
    m_dvdCurrentTitle = -1;
    m_dvdRunEndCell = -1;
    emit dvdPlaybackChanged();

    m_pendingStreamTitle = dvdDiscName();
    m_player->setSourceDevice(device, QUrl::fromLocalFile(videoTs));
    m_pendingAutoPlay = true;
    m_player->play();
    return true;
}

bool PlayerController::applyDvdTitle(const DvdIfo::Title &title,
                                     int fromCellIndex,
                                     qint64 positionOffsetMs,
                                     qint64 startSector)
{
    // The stream may only span one timestamp timeline: cells flagged as
    // starting a new one (STC discontinuity / VOB unit change) stall the
    // decoder clock if concatenated. Ends of runs auto-advance instead.
    int runEnd = int(title.cells.size());
    for (int i = fromCellIndex + 1; i < title.cellNewTimeline.size(); ++i) {
        if (title.cellNewTimeline.at(i)) {
            runEnd = i;
            break;
        }
    }

    QList<DvdIfo::Cell> cells =
            title.cells.mid(fromCellIndex, runEnd - fromCellIndex);
    if (!cells.isEmpty() && startSector > cells.first().firstSector
        && startSector <= cells.first().lastSector) {
        cells.first().firstSector = startSector; // mid-cell (time map) seek
    }
    DvdTitleDevice *device = DvdTitleDevice::createFromCells(
            m_dvdDir, title.vtsNumber, cells, this);
    if (!device) {
        qWarning() << "dvd: no segments for title" << title.titleNumber;
        emit errorMessage(tr("DVD title %1 could not be assembled")
                                  .arg(title.titleNumber));
        return false;
    }
    if (!device->open(QIODevice::ReadOnly)) {
        qWarning() << "dvd: cannot open VOBs for title" << title.titleNumber;
        emit errorMessage(tr("DVD title %1 could not be opened")
                                  .arg(title.titleNumber));
        delete device;
        return false;
    }
    qInfo() << "dvd: playing title" << title.titleNumber << "cells"
            << fromCellIndex << "to" << runEnd - 1 << "sector" << startSector
            << "stream size" << device->size();

    m_dvdRunEndCell = runEnd;
    m_player->setLoops(QMediaPlayer::Once); // titles play once (menus loop)
    saveCurrentPosition();
    m_player->stop();
    m_playlist->clear();

    if (m_dvdDevice)
        m_dvdDevice->deleteLater();
    m_dvdDevice = device;
    m_dvdCurrentTitle = title.titleNumber;
    m_dvdPositionOffsetMs = positionOffsetMs;
    // Bookmarks are per-title; the key stays stable across in-title seeks.
    m_bookmarks->setCurrentKey(QStringLiteral("dvd:%1#title=%2")
                                       .arg(m_dvdDir)
                                       .arg(title.titleNumber));
    emit dvdPlaybackChanged();

    // Unique per-source hint URL: QMediaPlayer treats an unchanged source
    // URL as "same media", which broke switching between titles that all
    // pointed at the same VIDEO_TS folder. Also make sure no stale
    // per-file position exists for it (DVDs always start at 0).
    QUrl hint = QUrl::fromLocalFile(m_dvdDir);
    hint.setQuery(QStringLiteral("title=%1&cell=%2&sector=%3")
                          .arg(title.titleNumber).arg(fromCellIndex)
                          .arg(qMax<qint64>(0, startSector)));
    m_fileSettings->remove(hint);

    // Show the disc/folder name in the window title, not the raw hint URL.
    m_pendingStreamTitle = dvdDiscName();
    m_player->setSourceDevice(device, hint);
    m_pendingAutoPlay = true;
    m_player->play();
    return true;
}

const DvdIfo::Title *PlayerController::currentDvdTitle() const
{
    for (const DvdIfo::Title &title : m_dvdTitles) {
        if (title.titleNumber == m_dvdCurrentTitle)
            return &title;
    }
    return nullptr;
}

QVariantList PlayerController::dvdTitles() const
{
    QVariantList rows;
    if (!m_dvdDevice)
        return rows;
    for (const DvdIfo::Title &title : m_dvdTitles) {
        rows << QVariantMap {
            { QStringLiteral("number"), title.titleNumber },
            { QStringLiteral("label"),
              tr("Title %1 (%2)").arg(title.titleNumber)
                      .arg(formatDuration(title.durationMs)) }
        };
    }
    return rows;
}

QVariantList PlayerController::dvdChapters() const
{
    QVariantList rows;
    const DvdIfo::Title *title = m_dvdDevice ? currentDvdTitle() : nullptr;
    if (!title)
        return rows;
    for (qsizetype i = 0; i < title->chapterStartsMs.size(); ++i) {
        rows << QVariantMap {
            { QStringLiteral("label"),
              tr("Chapter %1 (%2)").arg(i + 1)
                      .arg(formatDuration(title->chapterStartsMs.at(i))) },
            { QStringLiteral("startMs"), title->chapterStartsMs.at(i) }
        };
    }
    return rows;
}

qint64 PlayerController::dvdTitleDurationMs() const
{
    const DvdIfo::Title *title = m_dvdDevice ? currentDvdTitle() : nullptr;
    return title ? title->durationMs : 0;
}

void PlayerController::playDvdTitle(int titleNumber)
{
    for (const DvdIfo::Title &title : m_dvdTitles) {
        if (title.titleNumber == titleNumber) {
            applyDvdTitle(title);
            return;
        }
    }
}

void PlayerController::playDvdChapter(int chapterIndex)
{
    // Chapter jumps rebuild the stream at the chapter's entry cell:
    // time-based seeking is unreliable on the concatenated VOB stream
    // (FFmpeg's duration estimate clamps out-of-range targets).
    const DvdIfo::Title *title = currentDvdTitle();
    if (!title || chapterIndex < 0
        || chapterIndex >= title->chapterCellIndexes.size()) {
        return;
    }
    const qint64 startMs = title->chapterStartsMs.value(chapterIndex);
    if (applyDvdTitle(*title, title->chapterCellIndexes.at(chapterIndex),
                      startMs)) {
        emit seeked(startMs);
    }
}

// ---- Experimental DVD menu ("menu-lite") ------------------------------------

const DvdMenu::Domain *PlayerController::menuDomain(int vts) const
{
    if (vts == 0)
        return m_menus.vmgm.isValid() ? &m_menus.vmgm : nullptr;
    auto it = m_menus.vtsm.constFind(vts);
    return it != m_menus.vtsm.constEnd() ? &it.value() : nullptr;
}

int PlayerController::firstVtsWithTitle() const
{
    if (!m_dvdTitles.isEmpty())
        return m_dvdTitles.first().vtsNumber;
    if (!m_menus.vtsm.isEmpty())
        return m_menus.vtsm.constBegin().key();
    return 1;
}

bool PlayerController::menuPgcHasButtons(const DvdMenu::Domain *dom,
                                         int pgcNumber) const
{
    if (!dom || pgcNumber < 1 || pgcNumber > dom->pgcs.size())
        return false;
    const DvdMenu::Pgc &pgc = dom->pgcs.at(pgcNumber - 1);
    if (pgc.cells.isEmpty())
        return false;
    return DvdMenu::parseButtons(dom->vobPath, pgc.cells.first().firstSector,
                                 pgc.cells.last().lastSector).isValid();
}

bool PlayerController::runFirstPlay()
{
    if (!m_menus.hasFirstPlay())
        return false;
    m_vm.reset();
    const DvdVm::Action a = m_vm.run(m_menus.firstPlay.preCommands);
    if (a.kind != DvdVm::Action::None && a.kind != DvdVm::Action::Nop)
        return performNavAction(a, 0, 0); // First-Play lives in the VMGM domain
    // A commands-only First-Play that didn't branch, or a video-logo First-Play
    // (its own cells) — the latter is not played in menu-lite; fall back to the
    // menu so the disc is still usable.
    return false;
}

bool PlayerController::enterDefaultMenu()
{
    // Prefer the menu that actually carries selectable buttons — the disc's
    // real interactive menu. Discs vary in where that lives (a movie disc's
    // main menu is usually the VMGM title/root menu; an episode disc often
    // uses the VTSM root menu, with the VMGM title menu being a buttonless
    // intro). Probing the PCI for buttons picks the right one regardless.
    const int fv = firstVtsWithTitle();
    struct Cand { int vts; int menuId; };
    QList<Cand> cands { { 0, 2 }, { 0, 3 }, { fv, 3 }, { fv, 2 } };
    for (auto it = m_menus.vtsm.constBegin(); it != m_menus.vtsm.constEnd(); ++it) {
        cands << Cand { it.key(), 3 } << Cand { it.key(), 2 };
    }
    for (const Cand &c : cands) {
        const DvdMenu::Domain *dom = menuDomain(c.vts);
        if (!dom)
            continue;
        const int p = dom->entryPgc(c.menuId);
        if (p && menuPgcHasButtons(dom, p))
            return playMenuPgc(c.vts, p, true, 0);
    }

    // No entry menu had buttons directly — fall back to the conventional entry
    // (VMGM title/root, then VTSM), letting pre-command redirects run; and as a
    // last resort the first PGC of any menu domain.
    if (m_menus.vmgm.isValid()) {
        if (int p = m_menus.vmgm.entryPgc(2)) return playMenuPgc(0, p, true, 0);
        if (int p = m_menus.vmgm.entryPgc(3)) return playMenuPgc(0, p, true, 0);
    }
    if (const DvdMenu::Domain *dom = menuDomain(fv)) {
        if (int p = dom->entryPgc(3)) return playMenuPgc(fv, p, true, 0);
        if (int p = dom->entryPgc(2)) return playMenuPgc(fv, p, true, 0);
    }
    if (m_menus.vmgm.isValid())
        return playMenuPgc(0, 1, true, 0);
    if (!m_menus.vtsm.isEmpty())
        return playMenuPgc(m_menus.vtsm.constBegin().key(), 1, true, 0);
    return false;
}

bool PlayerController::enterMenu(int vts, int menuId, int depth)
{
    const DvdMenu::Domain *dom = menuDomain(vts);
    if (!dom) { // some discs keep the root menu in the VMGM domain
        dom = menuDomain(0);
        vts = 0;
    }
    if (!dom)
        return false;
    int p = dom->entryPgc(menuId);
    if (!p) p = dom->entryPgc(3); // root
    if (!p) p = dom->entryPgc(2); // title
    if (!p) p = 1;
    return playMenuPgc(vts, p, true, depth);
}

bool PlayerController::playMenuPgc(int vts, int pgcNumber, bool runPre, int depth)
{
    if (depth > 12) {
        qWarning() << "dvd menu: redirect loop, aborting";
        return false;
    }
    const DvdMenu::Domain *dom = menuDomain(vts);
    if (!dom || pgcNumber < 1 || pgcNumber > dom->pgcs.size())
        return false;
    const DvdMenu::Pgc &pgc = dom->pgcs.at(pgcNumber - 1);

    // Pre-commands may redirect (e.g. an intro menu that jumps to the root).
    if (runPre && !pgc.preCommands.isEmpty()) {
        const DvdVm::Action a = m_vm.run(pgc.preCommands);
        if (a.kind != DvdVm::Action::None && a.kind != DvdVm::Action::Nop)
            return performNavAction(a, vts, depth + 1);
    }
    if (pgc.cells.isEmpty()) {
        if (!pgc.postCommands.isEmpty()) {
            const DvdVm::Action a = m_vm.run(pgc.postCommands);
            if (a.kind != DvdVm::Action::None && a.kind != DvdVm::Action::Nop)
                return performNavAction(a, vts, depth + 1);
        }
        return false;
    }

    DvdTitleDevice *device =
            DvdTitleDevice::createFromMenuCells(dom->vobPath, pgc.cells, this);
    if (!device || !device->open(QIODevice::ReadOnly)) {
        delete device;
        qWarning() << "dvd menu: cannot open menu VOB" << dom->vobPath;
        return false;
    }

    saveCurrentPosition();
    m_player->stop();
    m_playlist->clear();
    if (m_dvdDevice)
        m_dvdDevice->deleteLater();
    m_dvdDevice = device;
    m_dvdCurrentTitle = -1;
    m_dvdRunEndCell = -1;
    m_dvdPositionOffsetMs = 0;
    m_menuVts = vts;
    m_menuPgc = pgcNumber;

    // Clickable buttons from the first NAV pack in the PGC's cells.
    m_menuButtons = DvdMenu::parseButtons(dom->vobPath,
                                          pgc.cells.first().firstSector,
                                          pgc.cells.last().lastSector);

    // The disc's own subpicture highlight (button outlines) + palette.
    m_menuSpu = DvdMenu::decodeSubpicture(dom->vobPath,
                                          pgc.cells.first().firstSector,
                                          pgc.cells.last().lastSector);
    memcpy(m_menuPalette, pgc.palette, sizeof(m_menuPalette));
    m_menuHighlightSel = -2; // invalidate the rendered-highlight cache
    m_menuHasButtons = !m_menuButtons.buttons.isEmpty();
    int sel = m_vm.sprm[8] >> 10; // SPRM8 = highlighted button * 1024
    if (sel < 1 || sel > m_menuButtons.buttons.size())
        sel = m_menuButtons.startButton;
    if (sel < 1 || sel > m_menuButtons.buttons.size())
        sel = m_menuButtons.buttons.isEmpty() ? 0 : 1;
    m_menuSelected = sel;

    qInfo().nospace() << "dvd menu: showing vts " << vts << " pgc " << pgcNumber
                      << " (" << m_menuButtons.buttons.size() << " buttons)";
    dvdLog(QStringLiteral("playMenuPgc vts=%1 pgc=%2 buttons=%3 hasButtons=%4 sel=%5")
                   .arg(vts).arg(pgcNumber).arg(m_menuButtons.buttons.size())
                   .arg(m_menuHasButtons).arg(m_menuSelected));

    emit dvdPlaybackChanged();
    emit dvdMenuChanged();

    QUrl hint = QUrl::fromLocalFile(m_dvdDir);
    hint.setQuery(QStringLiteral("menu=%1:%2").arg(vts).arg(pgcNumber));
    m_fileSettings->remove(hint);
    m_pendingStreamTitle = dvdDiscName();
    // Interactive menus (with buttons) loop until the user chooses; a buttonless
    // menu (an intro / transition) plays once, then its post-commands run.
    m_player->setLoops(m_menuHasButtons ? QMediaPlayer::Infinite
                                        : QMediaPlayer::Once);
    m_player->setSourceDevice(device, hint);
    m_pendingAutoPlay = true;
    m_player->play();
    return true;
}

bool PlayerController::performNavAction(const DvdVm::Action &a, int currentVts,
                                        int depth)
{
    using K = DvdVm::Action::Kind;
    switch (a.kind) {
    case K::LinkPgcn:
        return playMenuPgc(currentVts, a.data1, true, depth + 1);
    case K::JumpTt:
        return playGlobalTitle(a.data1);
    case K::JumpVtsTt:
        return playVtsTitle(currentVts > 0 ? currentVts : firstVtsWithTitle(),
                            a.data1, 0);
    case K::JumpVtsPtt:
        return playVtsTitle(currentVts > 0 ? currentVts : firstVtsWithTitle(),
                            a.data1, a.data2);
    case K::JumpSsVtsm:
        return enterMenu(a.data1, a.data3, depth + 1);
    case K::JumpSsVmgmMenu:
        return enterMenu(0, a.data1, depth + 1);
    case K::JumpSsVmgmPgc:
        return playMenuPgc(0, a.data1, true, depth + 1);
    case K::JumpSsFp:
        return enterDefaultMenu();
    case K::CallSsVtsm:
        return enterMenu(currentVts, a.data1, depth + 1);
    case K::CallSsVmgmMenu:
        return enterMenu(0, a.data1, depth + 1);
    case K::CallSsVmgmPgc:
        return playMenuPgc(0, a.data1, true, depth + 1);
    case K::Exit:
        leaveMenu();
        m_player->stop();
        emit playbackFinished();
        return true;
    default:
        // Relative links (top/next/prev PGC/cell/pg) — replay the current menu
        // as a best-effort for menu-lite.
        if (m_menuVts >= 0)
            return playMenuPgc(m_menuVts, m_menuPgc, false, depth + 1);
        return false;
    }
}

bool PlayerController::playGlobalTitle(int titleNumber)
{
    for (const DvdIfo::Title &t : m_dvdTitles) {
        if (t.titleNumber == titleNumber) {
            leaveMenu();
            return applyDvdTitle(t);
        }
    }
    return false;
}

bool PlayerController::playVtsTitle(int vts, int vtsTitleNumber, int part)
{
    dvdLog(QStringLiteral("playVtsTitle vts=%1 vtsTtn=%2 part=%3")
                   .arg(vts).arg(vtsTitleNumber).arg(part));
    for (const DvdIfo::Title &t : m_dvdTitles) {
        if (t.vtsNumber == vts && t.vtsTitleNumber == vtsTitleNumber) {
            leaveMenu();
            if (part > 1 && part <= t.chapterCellIndexes.size()) {
                return applyDvdTitle(t, t.chapterCellIndexes.at(part - 1),
                                     t.chapterStartsMs.at(part - 1));
            }
            return applyDvdTitle(t);
        }
    }
    return playGlobalTitle(vtsTitleNumber); // single-VTS discs: local == global
}

void PlayerController::leaveMenu()
{
    if (m_menuVts < 0)
        return;
    m_menuVts = -1;
    m_menuPgc = 0;
    m_menuButtons = {};
    m_menuSpu = {};
    m_menuHighlightSel = -2;
    m_menuSelected = 0;
    emit dvdMenuChanged();
}

QVariantList PlayerController::dvdMenuButtons() const
{
    QVariantList rows;
    for (int i = 0; i < m_menuButtons.buttons.size(); ++i) {
        const DvdMenu::Button &b = m_menuButtons.buttons.at(i);
        if (b.rect.isEmpty())
            continue;
        rows << QVariantMap {
            { QStringLiteral("index"), i + 1 },
            { QStringLiteral("x"), b.rect.x() },
            { QStringLiteral("y"), b.rect.y() },
            { QStringLiteral("w"), b.rect.width() },
            { QStringLiteral("h"), b.rect.height() },
            { QStringLiteral("autoAction"), b.autoAction }
        };
    }
    return rows;
}

QString PlayerController::dvdMenuHighlightUrl() const
{
    if (m_menuVts < 0 || !m_menuSpu.isValid())
        return {};
    if (m_menuHighlightSel == m_menuSelected)
        return m_menuHighlightCache; // unchanged since last render

    QRect selRect;
    quint32 selColor = 0;
    if (m_menuSelected >= 1 && m_menuSelected <= m_menuButtons.buttons.size()) {
        selRect = m_menuButtons.buttons.at(m_menuSelected - 1).rect;
        selColor = m_menuButtons.selectFor(m_menuSelected);
    }
    const QImage img = DvdMenu::renderHighlight(m_menuSpu, m_menuPalette,
                                                selRect, selColor);
    QString url;
    if (!img.isNull()) {
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "PNG");
        url = QStringLiteral("data:image/png;base64,")
                + QString::fromLatin1(png.toBase64());
    }
    m_menuHighlightSel = m_menuSelected;
    m_menuHighlightCache = url;
    return url;
}

void PlayerController::dvdMenuActivate(int buttonNumber)
{
    if (m_menuVts < 0 || buttonNumber < 1
        || buttonNumber > m_menuButtons.buttons.size())
        return;
    m_menuSelected = buttonNumber;
    m_vm.sprm[8] = quint16(buttonNumber << 10);
    const DvdMenu::Button &button = m_menuButtons.buttons.at(buttonNumber - 1);
    const int vts = m_menuVts;
    const DvdVm::Action a = m_vm.run({ button.command });
    dvdLog(QStringLiteral("activate btn=%1 auto=%2 cmd=%3 -> actionKind=%4 data1=%5")
                   .arg(buttonNumber).arg(button.autoAction)
                   .arg(QString::fromLatin1(button.command.toHex()))
                   .arg(int(a.kind)).arg(a.data1));

    // Run the button's command exactly as the disc authored it (as VLC /
    // MPC-HC do): an episode thumbnail that LinkPGCNs to a preview/highlight
    // state menu goes there first; playing the episode is a second activation.
    // (An earlier one-click "play-through" that resolved straight to the title
    // skipped the disc's preview state and diverged from other players.)
    if (a.kind == DvdVm::Action::None || a.kind == DvdVm::Action::Nop) {
        emit dvdMenuChanged(); // highlight only
        return;
    }
    performNavAction(a, vts, 0);
}

void PlayerController::dvdMenuActivateSelected()
{
    dvdMenuActivate(m_menuSelected);
}

void PlayerController::dvdMenuMove(const QString &direction)
{
    if (m_menuVts < 0 || m_menuSelected < 1
        || m_menuSelected > m_menuButtons.buttons.size())
        return;
    const DvdMenu::Button &b = m_menuButtons.buttons.at(m_menuSelected - 1);
    int next = 0;
    if (direction == QLatin1String("up")) next = b.up;
    else if (direction == QLatin1String("down")) next = b.down;
    else if (direction == QLatin1String("left")) next = b.left;
    else if (direction == QLatin1String("right")) next = b.right;
    if (next >= 1 && next <= m_menuButtons.buttons.size()
        && next != m_menuSelected) {
        m_menuSelected = next;
        m_vm.sprm[8] = quint16(next << 10);
        emit dvdMenuChanged();
    }
}

void PlayerController::dvdMenuHover(int buttonNumber)
{
    if (m_menuVts < 0 || buttonNumber < 1
        || buttonNumber > m_menuButtons.buttons.size()
        || buttonNumber == m_menuSelected)
        return;
    m_menuSelected = buttonNumber;
    m_vm.sprm[8] = quint16(buttonNumber << 10);
    emit dvdMenuChanged();
}

void PlayerController::showDvdMenu()
{
    if (m_dvdDir.isEmpty() || !m_menus.hasMenus())
        return;
    enterDefaultMenu();
}

bool PlayerController::dvdPlayMainTitle()
{
    if (m_dvdTitles.isEmpty())
        return false;
    const auto main = std::max_element(
            m_dvdTitles.constBegin(), m_dvdTitles.constEnd(),
            [](const DvdIfo::Title &a, const DvdIfo::Title &b) {
                return a.durationMs < b.durationMs;
            });
    leaveMenu();
    return applyDvdTitle(*main);
}

QString PlayerController::dvdDiscName() const
{
    if (m_dvdDir.isEmpty())
        return {};
    QDir dir(m_dvdDir);
    dir.cdUp(); // VIDEO_TS -> the disc/root folder
    const QString name = dir.dirName();
    return name.isEmpty() ? QStringLiteral("DVD") : name;
}

void PlayerController::setShuffle(bool shuffle)
{
    if (shuffle == m_shuffle)
        return;
    m_shuffle = shuffle;
    emit shuffleChanged();
}

void PlayerController::setRepeatAll(bool repeatAll)
{
    if (repeatAll == m_repeatAll)
        return;
    m_repeatAll = repeatAll;
    emit repeatAllChanged();
}

void PlayerController::setResumeEnabled(bool enabled)
{
    if (enabled == m_resumeEnabled)
        return;
    m_resumeEnabled = enabled;
    emit resumeEnabledChanged();
}

void PlayerController::setPreferredAudioLanguages(const QString &languages)
{
    if (languages == m_preferredAudioLanguages)
        return;
    m_preferredAudioLanguages = languages;
    emit preferredAudioLanguagesChanged();
}

void PlayerController::setPreferredSubtitleLanguages(const QString &languages)
{
    if (languages == m_preferredSubtitleLanguages)
        return;
    m_preferredSubtitleLanguages = languages;
    emit preferredSubtitleLanguagesChanged();
}

void PlayerController::setSubtitlesByDefault(bool enabled)
{
    if (enabled == m_subtitlesByDefault)
        return;
    m_subtitlesByDefault = enabled;
    emit subtitlesByDefaultChanged();
}

void PlayerController::setSessionPlaylistEnabled(bool enabled)
{
    if (enabled == m_sessionPlaylistEnabled)
        return;
    m_sessionPlaylistEnabled = enabled;
    emit sessionPlaylistEnabledChanged();
}

void PlayerController::setAutosavePlaylistOnExit(bool autosave)
{
    if (autosave == m_autosavePlaylistOnExit)
        return;
    m_autosavePlaylistOnExit = autosave;
    emit autosavePlaylistOnExitChanged();
}

void PlayerController::setAutoAddFolderFiles(bool autoAdd)
{
    if (autoAdd == m_autoAddFolderFiles)
        return;
    m_autoAddFolderFiles = autoAdd;
    emit autoAddFolderFilesChanged();
}

void PlayerController::setRememberTrackSelections(bool remember)
{
    if (remember == m_rememberTrackSelections)
        return;
    m_rememberTrackSelections = remember;
    emit rememberTrackSelectionsChanged();
}

void PlayerController::setAutoPlayNext(bool autoPlay)
{
    if (autoPlay == m_autoPlayNext)
        return;
    m_autoPlayNext = autoPlay;
    emit autoPlayNextChanged();
}

void PlayerController::setDisableScreensaver(bool disable)
{
    if (disable == m_disableScreensaver)
        return;
    m_disableScreensaver = disable;
    updateScreensaver();
    emit disableScreensaverChanged();
}

void PlayerController::updateScreensaver()
{
    // Inhibit only while a video is actively playing; audio-only playback and
    // pause/stop let the screensaver run as usual.
    const bool inhibit = m_disableScreensaver
            && m_player->playbackState() == QMediaPlayer::PlayingState
            && m_player->hasVideo();
    m_screenSaver->setInhibited(inhibit);
}

void PlayerController::setPlayOnLoadPlaylist(bool play)
{
    if (play == m_playOnLoadPlaylist)
        return;
    m_playOnLoadPlaylist = play;
    emit playOnLoadPlaylistChanged();
}

void PlayerController::setIgnorePlaybackErrors(bool ignore)
{
    if (ignore == m_ignorePlaybackErrors)
        return;
    m_ignorePlaybackErrors = ignore;
    emit ignorePlaybackErrorsChanged();
}

void PlayerController::setMediaToAdd(int mode)
{
    if (mode == m_mediaToAdd)
        return;
    m_mediaToAdd = mode;
    emit mediaToAddChanged();
}

void PlayerController::setNetworkTimeout(int seconds)
{
    if (seconds == m_networkTimeout)
        return;
    m_networkTimeout = seconds;
    applyStreamOptions();
    emit networkTimeoutChanged();
}

void PlayerController::teardownHttpTsSource()
{
    if (!m_httpTsSource)
        return;
    m_httpTsSource->device()->abort(); // wake any blocked backend read
    m_httpTsSource->deleteLater();
    m_httpTsSource = nullptr;
}

void PlayerController::startHttpTsStream(const QUrl &url)
{
    teardownHttpTsSource();
    m_httpTsSource = new HttpTsSource(this);
    HttpTsSource *source = m_httpTsSource;
    connect(source, &HttpTsSource::tsConfirmed, this, [this, source]() {
        // Raw MPEG-TS (a TV tuner): feed the backend clean TS via our device.
        m_player->setSourceDevice(source->device(),
                                  QUrl(QStringLiteral("livestream.ts")));
        m_pendingAutoPlay = true;
        m_player->play();
    });
    connect(source, &HttpTsSource::notTsStream, this, [this, url]() {
        // Not TS (HLS, mp4, …): let QMediaPlayer play the URL directly.
        m_player->setSource(url);
        m_pendingAutoPlay = true;
        m_player->play();
    });
    connect(source, &HttpTsSource::failed, this, [this](const QString &message) {
        emit errorMessage(message.isEmpty() ? tr("Could not open the stream")
                                            : message);
    });
    source->start(url);
}

void PlayerController::applyStreamOptions()
{
    // Raise the FFmpeg backend's socket-I/O timeout for network streams (a
    // user-tunable knob for slow connections). FFmpeg backend only.
    // NOTE: we deliberately do NOT inflate probeSize — a large probe made the
    // backend read/analyse megabytes before starting, stalling ordinary local
    // files for seconds. Live TV tuners are handled by HttpTsSource feeding
    // clean TS from byte 0, so the default probe size is sufficient there.
    QPlaybackOptions options = m_player->playbackOptions();
    if (m_networkTimeout > 0)
        options.setNetworkTimeout(std::chrono::seconds(m_networkTimeout));
    else
        options.resetNetworkTimeout();
    options.resetProbeSize();
    m_player->setPlaybackOptions(options);
}

void PlayerController::setPlayFilesFromStart(bool fromStart)
{
    if (fromStart == m_playFilesFromStart)
        return;
    m_playFilesFromStart = fromStart;
    emit playFilesFromStartChanged();
}

void PlayerController::clearFileSettings()
{
    m_fileSettings->clearAll();
}

int PlayerController::currentFileFormat() const
{
    const QVariant value = m_player->metaData().value(QMediaMetaData::FileFormat);
    return value.isValid() ? value.toInt() : -1;
}

int PlayerController::currentVideoCodec() const
{
    const QVariant value = m_player->metaData().value(QMediaMetaData::VideoCodec);
    return value.isValid() ? value.toInt() : -1;
}

int PlayerController::currentAudioCodec() const
{
    const QVariant value = m_player->metaData().value(QMediaMetaData::AudioCodec);
    return value.isValid() ? value.toInt() : -1;
}

QVariantList PlayerController::audioDevices() const
{
    QVariantList rows;
    rows << QVariantMap { { QStringLiteral("id"), QString() },
                          { QStringLiteral("description"),
                            tr("System default") } };
    const QList<QAudioDevice> devices = QMediaDevices::audioOutputs();
    for (const QAudioDevice &device : devices) {
        rows << QVariantMap {
            { QStringLiteral("id"), QString::fromUtf8(device.id()) },
            { QStringLiteral("description"), device.description() }
        };
    }
    return rows;
}

QString PlayerController::currentAudioDeviceId() const
{
    return QString::fromUtf8(m_audioOutput->device().id());
}

QString PlayerController::currentAudioDeviceDescription() const
{
    return m_audioOutput->device().description();
}

void PlayerController::setAudioDeviceId(const QString &deviceId)
{
    if (deviceId == m_audioDeviceId)
        return;
    m_audioDeviceId = deviceId;
    applyAudioDevice();
    emit audioDeviceIdChanged();
}

void PlayerController::applyAudioDevice()
{
    if (m_audioDeviceId.isEmpty()) {
        m_audioOutput->setDevice(QMediaDevices::defaultAudioOutput());
        return;
    }
    const QList<QAudioDevice> devices = QMediaDevices::audioOutputs();
    for (const QAudioDevice &device : devices) {
        if (QString::fromUtf8(device.id()) == m_audioDeviceId) {
            m_audioOutput->setDevice(device);
            return;
        }
    }
    m_audioOutput->setDevice(QMediaDevices::defaultAudioOutput());
}

void PlayerController::openDirectory(const QUrl &directory, bool recursive)
{
    if (!directory.isLocalFile())
        return;

    QStringList nameFilters;
    for (const QString &ext : mediaExtensions())
        nameFilters << QStringLiteral("*.") + ext;

    QStringList files;
    QDirIterator it(directory.toLocalFile(), nameFilters, QDir::Files,
                    recursive ? QDirIterator::Subdirectories
                              : QDirIterator::NoIteratorFlags);
    while (it.hasNext())
        files << it.next();
    std::sort(files.begin(), files.end(), [](const QString &a, const QString &b) {
        return QString::localeAwareCompare(a, b) < 0;
    });
    if (files.isEmpty())
        return;

    QList<PlaylistEntry> entries;
    entries.reserve(files.size());
    for (const QString &file : files)
        entries.append({ QUrl::fromLocalFile(file), QString() });

    m_player->stop();
    m_playlist->clear();
    m_playlist->add(entries);
    playIndex(0, /*resume=*/true); // opening a folder resumes its first file
}

QString PlayerController::takeScreenshot(const QString &folder,
                                         const QString &format)
{
    QVideoSink *sink = m_player->videoSink();
    if (!sink)
        return {};
    const QImage image = sink->videoFrame().toImage();
    if (image.isNull())
        return {};

    QDir dir(folder);
    if (folder.isEmpty() || !dir.mkpath(QStringLiteral(".")))
        return {};

    const QString suffix = format.toLower() == QLatin1String("jpg")
                                   ? QStringLiteral("jpg")
                                   : QStringLiteral("png");
    const QString name = QStringLiteral("vivace_%1.%2")
            .arg(QDateTime::currentDateTime()
                         .toString(QStringLiteral("yyyy-MM-dd_hh-mm-ss-zzz")),
                 suffix);
    const QString path = dir.filePath(name);

    if (!image.save(path))
        return {};
    return QDir::toNativeSeparators(path);
}

void PlayerController::addCurrentTo(FavoritesModel *list)
{
    if (!list)
        return;

    // DVD playback: favorite the disc folder (open() replays it as a DVD).
    if (m_dvdDevice && !m_dvdDir.isEmpty()) {
        QDir parent(m_dvdDir);
        parent.cdUp(); // the disc folder containing VIDEO_TS
        list->addUrl(parent.dirName(),
                     QUrl::fromLocalFile(parent.absolutePath()).toString());
        return;
    }

    const QUrl url = m_player->source();
    if (url.isEmpty())
        return;
    QString title = m_player->metaData().stringValue(QMediaMetaData::Title);
    if (title.isEmpty())
        title = url.fileName();
    if (title.isEmpty())
        title = url.toDisplayString();
    list->addUrl(title, url.toString());
}

void PlayerController::addBookmark(const QString &name)
{
    if (!m_bookmarks->hasKey())
        return;
    // DVDs bookmark title-global time; regular media use the raw position.
    const qint64 time = m_dvdDevice
            ? m_player->position() + m_dvdPositionOffsetMs
            : m_player->position();
    m_bookmarks->add(time, name);
}

void PlayerController::goToBookmark(qint64 timeMs)
{
    if (timeMs < 0)
        return;
    if (m_dvdDevice) {
        seekDvd(timeMs);
        return;
    }
    if (!m_player->isSeekable())
        return;
    const qint64 target = qBound<qint64>(0, timeMs, m_player->duration());
    m_player->setPosition(target);
    emit seeked(target);
}

void PlayerController::setAMarker()
{
    m_abMarkerA = m_player->position();
    // B must stay after A; drop it if the new A is at or past it.
    if (m_abMarkerB >= 0 && m_abMarkerB <= m_abMarkerA)
        m_abMarkerB = -1;
    emit abMarkersChanged();
    emit osdMessage(tr("A marker set to %1").arg(formatDuration(m_abMarkerA)));
}

void PlayerController::setBMarker()
{
    m_abMarkerB = m_player->position();
    if (m_abMarkerA >= 0 && m_abMarkerA >= m_abMarkerB) {
        // B before A is meaningless; keep B but drop the stale A.
        m_abMarkerA = -1;
    }
    emit abMarkersChanged();
    emit osdMessage(tr("B marker set to %1").arg(formatDuration(m_abMarkerB)));
}

void PlayerController::clearABMarkers()
{
    if (m_abMarkerA < 0 && m_abMarkerB < 0)
        return;
    m_abMarkerA = -1;
    m_abMarkerB = -1;
    emit abMarkersChanged();
    emit osdMessage(tr("A-B markers cleared"));
}

// ---- view transforms (aspect / zoom-pan / rotate / flip / mirror) ----------

void PlayerController::setVideoAspect(qreal aspect)
{
    if (qFuzzyCompare(m_videoAspect, aspect))
        return;
    m_videoAspect = aspect;
    emit videoTransformChanged();
}

void PlayerController::setVideoRotation(int degrees)
{
    degrees = ((degrees % 360) + 360) % 360; // normalise to 0/90/180/270
    if (degrees == m_videoRotation)
        return;
    m_videoRotation = degrees;
    emit videoTransformChanged();
}

void PlayerController::setVideoFlip(bool flip)
{
    if (flip == m_videoFlip)
        return;
    m_videoFlip = flip;
    emit videoTransformChanged();
}

void PlayerController::setVideoMirror(bool mirror)
{
    if (mirror == m_videoMirror)
        return;
    m_videoMirror = mirror;
    emit videoTransformChanged();
}

void PlayerController::zoomIn()
{
    m_videoZoom = qMin(m_videoZoom + 0.1, 10.0);
    emit videoTransformChanged();
}

void PlayerController::zoomOut()
{
    m_videoZoom = qMax(m_videoZoom - 0.1, 0.1);
    emit videoTransformChanged();
}

void PlayerController::panBy(int dx, int dy)
{
    m_videoPanX += dx;
    m_videoPanY += dy;
    emit videoTransformChanged();
}

void PlayerController::resetZoomAndPan()
{
    if (qFuzzyCompare(m_videoZoom, 1.0) && m_videoPanX == 0 && m_videoPanY == 0)
        return;
    m_videoZoom = 1.0;
    m_videoPanX = 0;
    m_videoPanY = 0;
    emit videoTransformChanged();
}

void PlayerController::resetVideoTransform()
{
    if (m_videoAspect == 0.0 && qFuzzyCompare(m_videoZoom, 1.0)
        && m_videoPanX == 0 && m_videoPanY == 0 && m_videoRotation == 0
        && !m_videoFlip && !m_videoMirror)
        return;
    m_videoAspect = 0.0;
    m_videoZoom = 1.0;
    m_videoPanX = 0;
    m_videoPanY = 0;
    m_videoRotation = 0;
    m_videoFlip = false;
    m_videoMirror = false;
    emit videoTransformChanged();
}

// ---- external subtitles ----------------------------------------------------

void PlayerController::updateSubtitle(qint64 positionMs)
{
    if (m_externalSubs.isEmpty()) {
        if (!m_currentSubtitleText.isEmpty()) {
            m_currentSubtitleText.clear();
            emit currentSubtitleTextChanged();
        }
        return;
    }
    const qint64 t = positionMs - m_subtitleDelayMs;
    QString text;
    for (const SubtitleCue &cue : m_externalSubs) {
        if (cue.startMs > t)
            break; // sorted by start; no later cue can already be showing
        if (t <= cue.endMs) {
            text = cue.text;
            break;
        }
    }
    if (text != m_currentSubtitleText) {
        m_currentSubtitleText = text;
        emit currentSubtitleTextChanged();
    }
}

bool PlayerController::loadSubtitles(const QUrl &url)
{
    QList<SubtitleCue> cues = SubtitleParser::load(url);
    if (cues.isEmpty()) {
        emit errorMessage(tr("Could not load subtitles from %1")
                                  .arg(url.fileName()));
        return false;
    }
    m_externalSubs = std::move(cues);
    m_subtitleDelayMs = 0;
    // Avoid double subtitles: turn off any embedded subtitle track.
    if (m_player->activeSubtitleTrack() >= 0)
        m_player->setActiveSubtitleTrack(-1);
    emit externalSubtitlesChanged();
    emit subtitleDelayChanged();
    updateSubtitle(m_player->position());
    emit osdMessage(tr("Subtitles loaded: %1").arg(url.fileName()));
    return true;
}

void PlayerController::unloadSubtitles()
{
    if (m_externalSubs.isEmpty())
        return;
    m_externalSubs.clear();
    updateSubtitle(0); // clears the on-screen text
    emit externalSubtitlesChanged();
    emit osdMessage(tr("Subtitles unloaded"));
}

void PlayerController::setSubtitleDelay(int ms)
{
    if (ms == m_subtitleDelayMs)
        return;
    m_subtitleDelayMs = ms;
    emit subtitleDelayChanged();
    updateSubtitle(m_player->position());
    emit osdMessage(tr("Subtitle delay: %1 ms").arg(m_subtitleDelayMs));
}

void PlayerController::adjustSubtitleDelay(int deltaMs)
{
    setSubtitleDelay(m_subtitleDelayMs + deltaMs);
}

void PlayerController::setAutoloadSubtitles(bool autoload)
{
    if (autoload == m_autoloadSubtitles)
        return;
    m_autoloadSubtitles = autoload;
    emit autoloadSubtitlesChanged();
}

void PlayerController::autoloadSubtitlesFor(const QUrl &mediaUrl)
{
    if (!m_autoloadSubtitles || !mediaUrl.isLocalFile())
        return;
    const QFileInfo info(mediaUrl.toLocalFile());
    const QString stem = info.absolutePath() + QLatin1Char('/')
                         + info.completeBaseName();
    for (const QString &ext : SubtitleParser::supportedExtensions()) {
        const QString candidate = stem + QLatin1Char('.') + ext;
        if (QFile::exists(candidate)) {
            loadSubtitles(QUrl::fromLocalFile(candidate));
            return;
        }
    }
}

void PlayerController::restoreSessionPlaylist()
{
    const QList<PlaylistEntry> entries =
            PlaylistParser::load(QUrl::fromLocalFile(sessionPlaylistPath()));
    if (!entries.isEmpty())
        m_playlist->add(entries); // queued, not auto-played
}

void PlayerController::saveSessionPlaylist()
{
    // Saved for restore-on-startup (session playlist) and/or as an on-exit
    // copy (autosave); restore only happens when the session option is on.
    if (!m_sessionPlaylistEnabled && !m_autosavePlaylistOnExit)
        return;
    const QString path = sessionPlaylistPath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    PlaylistParser::save(QUrl::fromLocalFile(path), m_playlist->entries());
}

/*  HTML info page in the format of SMPlayer's InfoFile::getInfo:
    title with type icon, General, Clip info, Initial Video Stream,
    per-kind stream tables — same section names, same alternating
    row colors.
*/
QString PlayerController::mediaInfoHtml() const
{
    const QUrl url = m_player->source();
    if (url.isEmpty())
        return {};

    QString s;
    int row = 0;

    const auto openPar = [&row](const QString &text) {
        row = 0;
        return QStringLiteral("<h2>%1</h2><table width=\"100%\">").arg(text);
    };
    const auto closePar = []() { return QStringLiteral("</table>"); };
    const auto openItem = [&row]() {
        return QStringLiteral("<tr bgcolor=\"%1\">")
                .arg(row++ % 2 ? QStringLiteral("lavender")
                               : QStringLiteral("powderblue"));
    };
    const auto addItem = [&](const QString &tag, const QString &value) {
        if (value.isEmpty())
            return QString();
        return openItem() + QStringLiteral("<td><b>%1</b></td><td>%2</td></tr>")
                .arg(tag, value.toHtmlEscaped());
    };
    const auto addColumns = [&](const QStringList &columns) {
        row = 0;
        QString out = openItem();
        for (const QString &column : columns)
            out += QStringLiteral("<td><b>%1</b></td>").arg(column);
        return out + QStringLiteral("</tr>");
    };
    const auto addTrackRows = [&](const QList<QMediaMetaData> &tracks) {
        QString out;
        for (qsizetype n = 0; n < tracks.size(); ++n) {
            const QMediaMetaData &md = tracks.at(n);
            out += openItem()
                   + QStringLiteral("<td>%1</td><td>%2</td><td>%3</td></tr>")
                             .arg(n)
                             .arg(md.stringValue(QMediaMetaData::Language)
                                          .toHtmlEscaped(),
                                  md.stringValue(QMediaMetaData::Title)
                                          .toHtmlEscaped());
        }
        return out;
    };

    const QMediaMetaData md = m_player->metaData();

    // Title with the media-type icon, as in SMPlayer.
    QString icon = QStringLiteral("type_unknown");
    if (!url.isLocalFile())
        icon = QStringLiteral("type_url");
    else if (m_player->hasVideo())
        icon = QStringLiteral("type_video");
    else if (m_player->hasAudio())
        icon = QStringLiteral("type_audio");
    const QString displayName =
            url.fileName().isEmpty() ? url.toDisplayString() : url.fileName();
    s += QStringLiteral("<h1><img src=\"qrc:/qt/qml/Vivace/icons/Default/%1.png\"> %2</h1>")
                 .arg(icon, displayName.toHtmlEscaped());

    // General
    s += openPar(tr("General"));
    if (url.isLocalFile()) {
        const QFileInfo fi(url.toLocalFile());
        s += addItem(tr("File"),
                     QDir::toNativeSeparators(fi.absoluteFilePath()));
        if (fi.exists())
            s += addItem(tr("Size"), tr("%1 KB (%2 MB)")
                                 .arg(fi.size() / 1024)
                                 .arg(fi.size() / 1048576));
    } else {
        s += addItem(tr("URL"), url.toDisplayString());
    }
    if (m_player->duration() > 0)
        s += addItem(tr("Length"), formatDuration(m_player->duration()));
    s += addItem(tr("Demuxer"), md.stringValue(QMediaMetaData::FileFormat));
    s += closePar();

    // Clip info (all present descriptive metadata, as SMPlayer lists it)
    QString clip;
    clip += addItem(tr("Name"), md.stringValue(QMediaMetaData::Title));
    clip += addItem(tr("Artist"),
                    md.stringValue(QMediaMetaData::ContributingArtist));
    clip += addItem(tr("Album artist"),
                    md.stringValue(QMediaMetaData::AlbumArtist));
    clip += addItem(tr("Author"), md.stringValue(QMediaMetaData::Author));
    clip += addItem(tr("Composer"), md.stringValue(QMediaMetaData::Composer));
    clip += addItem(tr("Album"), md.stringValue(QMediaMetaData::AlbumTitle));
    clip += addItem(tr("Genre"), md.stringValue(QMediaMetaData::Genre));
    clip += addItem(tr("Date"), md.stringValue(QMediaMetaData::Date));
    clip += addItem(tr("Track"), md.stringValue(QMediaMetaData::TrackNumber));
    clip += addItem(tr("Copyright"), md.stringValue(QMediaMetaData::Copyright));
    clip += addItem(tr("Comment"), md.stringValue(QMediaMetaData::Comment));
    clip += addItem(tr("Description"),
                    md.stringValue(QMediaMetaData::Description));
    clip += addItem(tr("Publisher"), md.stringValue(QMediaMetaData::Publisher));
    clip += addItem(tr("Language"), md.stringValue(QMediaMetaData::Language));
    clip += addItem(tr("URL"), md.stringValue(QMediaMetaData::Url));
    if (!clip.isEmpty())
        s += openPar(tr("Clip info")) + clip + closePar();

    // Initial video stream
    if (m_player->hasVideo()) {
        s += openPar(tr("Initial Video Stream"));
        const QSize res = md.value(QMediaMetaData::Resolution).toSize();
        if (res.isValid()) {
            s += addItem(tr("Resolution"), QStringLiteral("%1 x %2")
                                 .arg(res.width()).arg(res.height()));
            if (res.height() > 0)
                s += addItem(tr("Aspect ratio"),
                             QString::number(qreal(res.width()) / res.height(),
                                             'f', 2));
        }
        s += addItem(tr("Format"), md.stringValue(QMediaMetaData::VideoCodec));
        const int vbr = md.value(QMediaMetaData::VideoBitRate).toInt();
        if (vbr > 0)
            s += addItem(tr("Bitrate"), tr("%1 kbps").arg(vbr / 1000));
        const qreal fps = md.value(QMediaMetaData::VideoFrameRate).toReal();
        if (fps > 0)
            s += addItem(tr("Frames per second"),
                         QString::number(fps, 'f', 3));
        if (md.value(QMediaMetaData::HasHdrContent).toBool())
            s += addItem(tr("HDR"), tr("yes"));
        s += closePar();
    }

    // Video streams table
    const QList<QMediaMetaData> videoTracks = m_player->videoTracks();
    if (!videoTracks.isEmpty()) {
        s += openPar(tr("Video Streams"));
        s += addColumns({ QStringLiteral("#"), tr("Language"), tr("Name") });
        s += addTrackRows(videoTracks);
        s += closePar();
    }

    // Initial audio stream
    if (m_player->hasAudio()) {
        s += openPar(tr("Initial Audio Stream"));
        s += addItem(tr("Format"), md.stringValue(QMediaMetaData::AudioCodec));
        const int abr = md.value(QMediaMetaData::AudioBitRate).toInt();
        if (abr > 0)
            s += addItem(tr("Bitrate"), tr("%1 kbps").arg(abr / 1000));
        s += closePar();
    }

    // Audio streams table
    const QList<QMediaMetaData> audioTracks = m_player->audioTracks();
    if (!audioTracks.isEmpty()) {
        s += openPar(tr("Audio Streams"));
        s += addColumns({ QStringLiteral("#"), tr("Language"), tr("Name") });
        s += addTrackRows(audioTracks);
        s += closePar();
    }

    // Subtitles table
    const QList<QMediaMetaData> subtitleTracks = m_player->subtitleTracks();
    if (!subtitleTracks.isEmpty()) {
        s += openPar(tr("Subtitles"));
        s += addColumns({ QStringLiteral("#"), tr("Language"), tr("Name") });
        s += addTrackRows(subtitleTracks);
        s += closePar();
    }

    return QStringLiteral("<html><body>%1</body></html>").arg(s);
}

bool PlayerController::savePlaylist(const QUrl &file)
{
    return PlaylistParser::save(file, m_playlist->entries());
}

void PlayerController::fillShuffleOrder()
{
    const int count = m_playlist->count();
    m_shuffleOrder.resize(count);
    std::iota(m_shuffleOrder.begin(), m_shuffleOrder.end(), 0);
    std::shuffle(m_shuffleOrder.begin(), m_shuffleOrder.end(),
                 *QRandomGenerator::global());
}

int PlayerController::pickNextIndex()
{
    const int count = m_playlist->count();
    if (count == 0)
        return -1;

    if (m_shuffle && count > 1) {
        // Exhaustive shuffle: play every item once per cycle, in a random
        // order, before any repeats.
        if (m_shuffleOrder.size() != count) {
            fillShuffleOrder();
            m_shufflePos = m_shuffleOrder.indexOf(m_playlist->currentIndex());
        }
        if (m_shufflePos + 1 < m_shuffleOrder.size())
            return m_shuffleOrder.at(++m_shufflePos);
        // Cycle finished.
        if (!m_repeatAll)
            return -1;
        const int last = m_shuffleOrder.isEmpty() ? -1 : m_shuffleOrder.last();
        fillShuffleOrder();
        // Avoid replaying the just-finished track first in the new cycle.
        if (m_shuffleOrder.size() > 1 && m_shuffleOrder.first() == last)
            std::swap(m_shuffleOrder[0], m_shuffleOrder[1]);
        m_shufflePos = 0;
        return m_shuffleOrder.at(0);
    }

    const int index = m_playlist->nextIndex();
    if (index < 0 && m_repeatAll)
        return 0;
    return index;
}

void PlayerController::next()
{
    const int index = pickNextIndex();
    if (index >= 0)
        playAt(index);
}

void PlayerController::previous()
{
    if (m_player->position() > restartThresholdMs && m_player->isSeekable()) {
        m_player->setPosition(0);
        return;
    }

    const int index = m_playlist->previousIndex();
    if (index >= 0)
        playAt(index);
    else if (m_player->isSeekable())
        m_player->setPosition(0);
}

void PlayerController::togglePlayPause()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->pause();
        return;
    }

    if (m_player->source().isEmpty()) {
        // Pressing play with nothing loaded resumes the current item.
        const int current = m_playlist->currentIndex();
        playIndex(current >= 0 ? current : 0, /*resume=*/true);
        return;
    }

    m_player->play();
}

void PlayerController::stop()
{
    saveCurrentPosition();
    m_player->stop();
}

QUrl PlayerController::closeSource()
{
    const QUrl was = m_player->source();
    saveCurrentPosition();
    m_player->stop();
    m_player->setSource(QUrl()); // release the file handle
    return was;
}

void PlayerController::seekRelative(qint64 deltaMs)
{
    if (!m_player->isSeekable())
        return;

    if (m_dvdDevice) {
        // Title-global seek; the backend's duration estimate is no bound.
        seekDvd(m_player->position() + m_dvdPositionOffsetMs + deltaMs);
        return;
    }

    const qint64 target = qBound<qint64>(0, m_player->position() + deltaMs,
                                         m_player->duration());
    m_player->setPosition(target);
    emit seeked(target);
}

int PlayerController::currentDvdChapterIndex() const
{
    const DvdIfo::Title *title = currentDvdTitle();
    if (!title || title->chapterStartsMs.isEmpty())
        return -1;
    const qint64 pos = m_player->position() + m_dvdPositionOffsetMs;
    int index = 0;
    for (qsizetype i = 0; i < title->chapterStartsMs.size(); ++i) {
        if (title->chapterStartsMs.at(i) <= pos)
            index = int(i);
        else
            break;
    }
    return index;
}

void PlayerController::previousDvdChapter()
{
    const int current = currentDvdChapterIndex();
    if (current >= 0)
        playDvdChapter(qMax(0, current - 1));
}

void PlayerController::nextDvdChapter()
{
    const DvdIfo::Title *title = currentDvdTitle();
    const int current = currentDvdChapterIndex();
    if (title && current >= 0
        && current + 1 < title->chapterStartsMs.size()) {
        playDvdChapter(current + 1);
    }
}

// ---- unified chapters (DVD or parsed file) ---------------------------------

QVariantList PlayerController::chapters() const
{
    if (m_dvdDevice)
        return dvdChapters();
    QVariantList rows;
    for (int i = 0; i < m_chapters.size(); ++i) {
        const ChapterParse::Chapter &c = m_chapters.at(i);
        rows << QVariantMap {
            { QStringLiteral("label"),
              c.title.isEmpty() ? tr("Chapter %1").arg(i + 1) : c.title },
            { QStringLiteral("startMs"), c.startMs }
        };
    }
    return rows;
}

int PlayerController::fileChapterIndexAt(qint64 ms) const
{
    int index = -1;
    for (int i = 0; i < m_chapters.size(); ++i) {
        if (m_chapters.at(i).startMs <= ms)
            index = i;
        else
            break;
    }
    return index;
}

void PlayerController::playChapter(int index)
{
    if (m_dvdDevice) {
        playDvdChapter(index);
        return;
    }
    if (index < 0 || index >= m_chapters.size())
        return;
    const qint64 target = m_chapters.at(index).startMs;
    m_player->setPosition(target);
    emit seeked(target);
}

void PlayerController::nextChapter()
{
    if (m_dvdDevice) {
        nextDvdChapter();
        return;
    }
    const int current = fileChapterIndexAt(m_player->position());
    if (current + 1 < m_chapters.size())
        playChapter(current + 1);
}

void PlayerController::previousChapter()
{
    if (m_dvdDevice) {
        previousDvdChapter();
        return;
    }
    const qint64 pos = m_player->position();
    const int current = fileChapterIndexAt(pos);
    if (current < 0) {
        if (!m_chapters.isEmpty())
            playChapter(0);
        return;
    }
    // More than 3 s into the chapter restarts it; otherwise step back one.
    if (pos - m_chapters.at(current).startMs > 3000)
        playChapter(current);
    else
        playChapter(qMax(0, current - 1));
}

void PlayerController::seekDvd(qint64 titleMs)
{
    // Purely byte-based: time-based seeking (setPosition) is unreliable
    // on multi-episode discs whose PTS timelines restart mid-stream.
    const DvdIfo::Title *title = currentDvdTitle();
    if (!title || title->cellStartsMs.isEmpty())
        return;
    titleMs = qBound<qint64>(0, titleMs, title->durationMs);

    // Preferred: the disc's own time map (second-accurate VOBU sectors).
    if (title->timeMapUnitSec > 0 && !title->timeMapSectors.isEmpty()) {
        const qint64 unitMs = qint64(title->timeMapUnitSec) * 1000;
        const qint64 index =
                qMin(titleMs / unitMs - 1,
                     qint64(title->timeMapSectors.size()) - 1);
        if (index >= 0) {
            const qint64 sector =
                    title->timeMapSectors.at(index) & 0x7FFFFFFF;
            for (qsizetype i = 0; i < title->cells.size(); ++i) {
                const DvdIfo::Cell &cell = title->cells.at(i);
                if (sector >= cell.firstSector && sector <= cell.lastSector) {
                    if (applyDvdTitle(*title, int(i), (index + 1) * unitMs,
                                      sector)) {
                        emit seeked((index + 1) * unitMs);
                    }
                    return;
                }
            }
            // Sector outside the included cells (e.g. skipped angle
            // block): fall through to cell-granular seeking.
        }
    }

    // Fallback: the start of the containing cell.
    int cellIndex = 0;
    for (qsizetype i = 0; i < title->cellStartsMs.size(); ++i) {
        if (title->cellStartsMs.at(i) <= titleMs)
            cellIndex = int(i);
        else
            break;
    }
    const qint64 cellStart = title->cellStartsMs.at(cellIndex);
    if (applyDvdTitle(*title, cellIndex, cellStart))
        emit seeked(cellStart);
}

void PlayerController::frameStep(int frames)
{
    // No frame-step API in QMediaPlayer; approximate by seeking one frame
    // duration while paused (spike-era accuracy, revisit if it proves rough).
    if (!m_player->isSeekable() || frames == 0)
        return;

    qreal fps = m_player->metaData().value(QMediaMetaData::VideoFrameRate).toReal();
    if (fps <= 0)
        fps = 25.0;

    m_player->pause();
    const qint64 delta = qRound64(1000.0 / fps * frames);
    seekRelative(delta == 0 ? (frames > 0 ? 1 : -1) : delta);
}

void PlayerController::handleMediaStatus(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::LoadedMedia) {
        // A play() issued right after setSource()/setSourceDevice() can be
        // dropped by the backend if it races the async pipeline setup,
        // leaving playback paused after opening a file (seen intermittently
        // in Release/RelWithDebInfo builds, not in Debug — a timing-
        // dependent race, not a logic difference). Re-assert it now that the
        // media has actually finished loading.
        if (m_pendingAutoPlay) {
            m_pendingAutoPlay = false;
            if (m_player->playbackState() != QMediaPlayer::PlayingState)
                m_player->play();
        }

        // Device-based sources (DVD titles) get track preferences only:
        // their hint URL is no good as a recents entry or resume key.
        if (m_player->sourceDevice()) {
            selectPreferredTracks();
            return;
        }

        const QUrl url = m_player->source();
        m_bookmarks->setCurrentKey(url.toString());
        if (m_externalSubs.isEmpty())
            autoloadSubtitlesFor(url);

        QString title = m_player->metaData()
                                .stringValue(QMediaMetaData::Title);
        if (title.isEmpty())
            title = url.fileName();
        m_recents->add(url, title);

        selectPreferredTracks();
        restoreTrackSelections(); // stored per-file choices win over defaults

        // Resume where this file was left off last time — only when the file
        // was "opened" (menu/recents/drag/CLI). Choosing an item in the
        // playlist plays from the beginning (m_resumeOnLoad = false). "Play
        // files from start" additionally suppresses resume for playlist runs.
        const bool skipResume = !m_resumeEnabled || !m_resumeOnLoad
                || (m_playFilesFromStart && m_playlist->count() > 1);
        const qint64 resumePos = skipResume ? -1 : m_fileSettings->position(url);
        if (resumePos > 0 && m_player->isSeekable()
            && resumePos < m_player->duration()) {
            m_player->setPosition(resumePos);
            emit seeked(resumePos);
        }
        return;
    }

    if (status == QMediaPlayer::EndOfMedia) {
        // DVD menu end-of-cell.
        if (m_menuVts >= 0) {
            if (m_menuHasButtons) {
                // Interactive menu: loop the clip (belt-and-suspenders — setLoops
                // Infinite normally handles it) while waiting for a choice.
                m_player->setPosition(0);
                m_player->play();
                return;
            }
            // Buttonless intro / transition menu: run its post-commands to move
            // on (e.g. the title-menu intro that JumpTT's into the movie).
            const DvdMenu::Domain *dom = menuDomain(m_menuVts);
            if (dom && m_menuPgc >= 1 && m_menuPgc <= dom->pgcs.size()) {
                const DvdVm::Action a =
                        m_vm.run(dom->pgcs.at(m_menuPgc - 1).postCommands);
                if (a.kind != DvdVm::Action::None
                    && a.kind != DvdVm::Action::Nop) {
                    performNavAction(a, m_menuVts, 0);
                    return;
                }
            }
            // Nothing to chain to — don't dead-end: play the main title.
            dvdPlayMainTitle();
            return;
        }
        // DVD: continue with the next timeline run of the title.
        if (m_dvdDevice) {
            const DvdIfo::Title *title = currentDvdTitle();
            if (title && m_dvdRunEndCell >= 0
                && m_dvdRunEndCell < title->cells.size()) {
                applyDvdTitle(*title, m_dvdRunEndCell,
                              title->cellStartsMs.at(m_dvdRunEndCell));
            } else {
                emit playbackFinished();
            }
            return;
        }

        m_fileSettings->remove(m_player->source()); // watched to the end
        const int index = m_autoPlayNext ? pickNextIndex() : -1;
        if (index >= 0)
            playAt(index);
        else
            emit playbackFinished();
    }
}

void PlayerController::restoreTrackSelections()
{
    if (!m_rememberTrackSelections)
        return;

    const QUrl url = m_player->source();
    const int audio = m_fileSettings->audioTrack(url);
    if (audio >= 0 && audio < m_player->audioTracks().size())
        m_player->setActiveAudioTrack(audio);

    const int subtitle = m_fileSettings->subtitleTrack(url);
    if (subtitle != FileSettings::notStored
        && subtitle >= -1 && subtitle < m_player->subtitleTracks().size()) {
        m_player->setActiveSubtitleTrack(subtitle);
    }
}

void PlayerController::selectPreferredTracks()
{
    if (!m_preferredAudioLanguages.trimmed().isEmpty()) {
        const int index = findTrackByLanguages(m_player->audioTracks(),
                                               m_preferredAudioLanguages);
        if (index >= 0)
            m_player->setActiveAudioTrack(index);
    }

    if (!m_subtitlesByDefault) {
        m_player->setActiveSubtitleTrack(-1);
    } else if (!m_preferredSubtitleLanguages.trimmed().isEmpty()) {
        const int index = findTrackByLanguages(m_player->subtitleTracks(),
                                               m_preferredSubtitleLanguages);
        if (index >= 0)
            m_player->setActiveSubtitleTrack(index);
    }
}

void PlayerController::saveCurrentPosition()
{
    if (m_player->sourceDevice())
        return; // DVD titles have no stable per-file key yet

    const QUrl url = m_player->source();
    if (url.isEmpty() || !m_player->isSeekable())
        return;

    // Per-file track selections (SMPlayer's "remember settings for files").
    if (m_rememberTrackSelections
        && (!m_player->audioTracks().isEmpty()
            || !m_player->subtitleTracks().isEmpty())) {
        m_fileSettings->setTracks(url, m_player->activeAudioTrack(),
                                  m_player->activeSubtitleTrack());
    }

    const qint64 duration = m_player->duration();
    const qint64 position = m_player->position();
    if (duration < resumeMinDurationMs)
        return;

    if (position < resumeMinPositionMs
        || position > qint64(duration * resumeWatchedFraction)) {
        m_fileSettings->remove(url);
    } else {
        m_fileSettings->setPosition(url, position);
    }
}
