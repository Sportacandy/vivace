/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "playercontroller.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
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
#include <QPointer>
#include <QRandomGenerator>

#include "httptssource.h"
#include "livestreamdevice.h"
#include <QRegularExpression>
#include <QGuiApplication>
#include <QSize>
#include <QSettings>
#include <QStandardPaths>
#include <QStyleHints>
#include <QThreadPool>
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

// Soften a decoded DVD subpicture bitmap's visibly coarse antialiasing
// steps (2026-08-16 user report: "very jaggy glyph"). A DVD subtitle is a
// paletted bitmap authored at native SD resolution (no vector/text data
// survives in the stream at all, so there's no higher-quality source to
// fall back to) that Vivace then scales up to whatever size the video is
// displayed at -- the antialiasing baked in at authoring time only has a
// couple of native pixels to blend an edge over, which reads as "blocky"
// once magnified. This is the exact same complaint mpv addresses with its
// own `--sub-gauss` option for image (DVD/PGS) subtitles, via a Gaussian
// blur on the decoded bitmap -- not dithering (dithering fixes color-
// banding in gradients, not edge aliasing) and not a "vector font" (there
// is no vector glyph data to recover from a paletted bitmap). A two-pass
// (horizontal then vertical) TRIANGULAR (tent) blur approximates a
// Gaussian; done on premultiplied alpha so the blur doesn't smear black
// into the surrounding transparent area.
//
// NOT a uniform box average (the first version of this fix, briefly):
// a box blur weights every tap equally, so even the CENTER pixel gets
// diluted by 1/(2*radius+1) -- fine for a true wide-open edge, but a DVD
// glyph's strokes at native resolution are often only a few pixels wide,
// so a uniform box blur eroded brightness across the whole stroke, not
// just its edges (2026-08-16 user report: "subtitle text after blur seems
// less bright"). A triangular kernel (weight = radius+1-|d|, i.e. [1,2,1]
// for radius 1) gives the center tap much more weight than its neighbours,
// so a fully-opaque interior pixel stays close to fully opaque while the
// actual transition pixels at an edge still get meaningfully softened.
QImage softenSubtitleEdges(const QImage &src, int radius)
{
    if (radius <= 0 || src.isNull())
        return src;
    const QImage premult = src.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const int w = premult.width();
    const int h = premult.height();
    QImage pass1(w, h, QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < h; ++y) {
        const QRgb *srcLine = reinterpret_cast<const QRgb *>(premult.constScanLine(y));
        QRgb *dstLine = reinterpret_cast<QRgb *>(pass1.scanLine(y));
        for (int x = 0; x < w; ++x) {
            int sa = 0, sr = 0, sg = 0, sb = 0, weightSum = 0;
            for (int d = -radius; d <= radius; ++d) {
                const int sx = x + d;
                if (sx < 0 || sx >= w)
                    continue;
                const int weight = radius + 1 - std::abs(d);
                const QRgb p = srcLine[sx];
                sa += qAlpha(p) * weight; sr += qRed(p) * weight;
                sg += qGreen(p) * weight; sb += qBlue(p) * weight;
                weightSum += weight;
            }
            dstLine[x] = qRgba(sr / weightSum, sg / weightSum, sb / weightSum, sa / weightSum);
        }
    }
    QImage pass2(w, h, QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < h; ++y) {
        QRgb *dstLine = reinterpret_cast<QRgb *>(pass2.scanLine(y));
        for (int x = 0; x < w; ++x) {
            int sa = 0, sr = 0, sg = 0, sb = 0, weightSum = 0;
            for (int d = -radius; d <= radius; ++d) {
                const int sy = y + d;
                if (sy < 0 || sy >= h)
                    continue;
                const int weight = radius + 1 - std::abs(d);
                const QRgb p = reinterpret_cast<const QRgb *>(pass1.constScanLine(sy))[x];
                sa += qAlpha(p) * weight; sr += qRed(p) * weight;
                sg += qGreen(p) * weight; sb += qBlue(p) * weight;
                weightSum += weight;
            }
            dstLine[x] = qRgba(sr / weightSum, sg / weightSum, sb / weightSum, sa / weightSum);
        }
    }
    return pass2.convertToFormat(QImage::Format_ARGB32);
}

// Same on-disk file? Used to skip adding a playlist entry that's already
// there (e.g. saving a YouTube-cache video to a folder it was already saved
// to before). canonicalFilePath() (not a plain URL/string compare) so a
// relative or differently-cased path resolves to the same real file;
// falls back to absoluteFilePath() for a dangling entry (canonicalFilePath()
// returns empty when the file doesn't exist), matching the same fallback
// PlaylistParser::save()'s entryLine() uses.
bool sameLocalFile(const QUrl &a, const QUrl &b)
{
    if (!a.isLocalFile() || !b.isLocalFile())
        return a == b;
    const QFileInfo fa(a.toLocalFile());
    const QFileInfo fb(b.toLocalFile());
    QString pa = fa.canonicalFilePath();
    if (pa.isEmpty())
        pa = fa.absoluteFilePath();
    QString pb = fb.canonicalFilePath();
    if (pb.isEmpty())
        pb = fb.absoluteFilePath();
    return pa == pb;
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

// A DVD/Blu-ray-declared ISO 639-2 language code (e.g. "jpn") as a display
// name (e.g. "日本語"), falling back to the raw code uppercased when
// QLocale doesn't recognize it, or empty for an empty/absent code -- shared
// by the Audio/Subtitles > Track menu labels (Blu-ray's own; DVD's
// dvdAudioTrackLabels()/dvdSubtitleTrackLabels() still inline the same
// logic separately, predating this helper) and the Information dialog's
// Streams tables (both BD and DVD), so all of them show the exact same
// name for the same stream.
QString discLanguageDisplayName(const QString &code)
{
    if (code.isEmpty())
        return QString();
    const QString native = QLocale(code).nativeLanguageName();
    return native.isEmpty() ? code.toUpper() : native;
}

// A QLocale::Language enum value (what QMediaMetaData::Language actually
// stores -- see QMediaMetaData::stringValue()'s own Language case, which
// converts it to plain English via QLocale::languageToString()) as a native
// display name (e.g. "日本語"), matching discLanguageDisplayName()'s own
// format -- used so the Information dialog's Language column is consistent
// (native/translated name) for EVERY source, not just Blu-ray/DVD: an
// ordinary file with an embedded language tag previously showed the plain
// English name via stringValue(); this makes it show the same kind of
// native name the Track menus already show for DVD/Blu-ray. Empty when no
// language is set at all (QLocale::AnyLanguage).
QString nativeLanguageDisplayName(QLocale::Language lang)
{
    if (lang == QLocale::AnyLanguage || lang == QLocale::C)
        return QString();
    const QString native = QLocale(lang).nativeLanguageName();
    return native.isEmpty() ? QLocale::languageToString(lang) : native;
}

// libbluray's own declared audio coding_type (bd_stream_type_e) as a plain
// codec name -- Qt's QMediaMetaData::AudioCodec enum has no entry for most
// BD-native audio codecs (the whole DTS family, TrueHD), so it reports
// "Unspecified" for them (confirmed against a real disc: even the Media
// Info dialog's own "Audio codec" tab, which lists every codec Qt Multimedia
// can enumerate, has no DTS entry at all). Empty for a coding_type this
// mapping doesn't recognize (falls back to the Qt-reported value).
QString blurayAudioCodecName(quint8 codingType)
{
    switch (codingType) {
    case 0x03: return PlayerController::tr("MPEG-1 Audio");
    case 0x04: return PlayerController::tr("MPEG-2 Audio");
    case 0x80: return PlayerController::tr("LPCM");
    case 0x81: return PlayerController::tr("AC-3 (Dolby Digital)");
    case 0x82: return QStringLiteral("DTS");
    case 0x83: return PlayerController::tr("Dolby TrueHD");
    case 0x84: return PlayerController::tr("AC-3 Plus (Dolby Digital Plus)");
    case 0x85: return PlayerController::tr("DTS-HD High Resolution Audio");
    case 0x86: return PlayerController::tr("DTS-HD Master Audio");
    case 0xa1: return PlayerController::tr("AC-3 Plus (secondary)");
    case 0xa2: return PlayerController::tr("DTS-HD (secondary)");
    default: return QString();
    }
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

// Same matching as findTrackByLanguages() above, against the IFO-declared
// DVD subtitle stream table instead of FFmpeg's (for DVD subpicture tracks,
// unreliable -- see dvdsubtitletrack.h) QMediaMetaData track list.
int findDvdSubtitleTrackByLanguages(const QList<DvdIfo::SubtitleStream> &streams,
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

        for (int i = 0; i < streams.size(); ++i) {
            const QString lang = streams.at(i).language.toLower();
            if (lang.isEmpty())
                continue;
            if (lang == wanted || lang == wantedName)
                return i;
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
    connect(m_player, &QMediaPlayer::sourceChanged, this, [this](const QUrl &source) {
        flushVideoDelayQueue();
        // With a negative A/V delay, the player renders into m_videoDelaySink
        // (our intercept), not m_targetVideoSink (the VideoOutput actually on
        // screen) -- see applyVideoDelayRouting(). So when playback genuinely
        // ends with nothing queued next (source cleared to empty), Qt's own
        // "clear the video output" behavior on stop never reaches
        // m_targetVideoSink, since it was never the player's direct sink to
        // begin with. Without this, the last frame presented via
        // presentDueFrames() stays on screen forever, and the "drop a file
        // here" placeholder (bound to source being empty, see Main.qml) never
        // visually takes over the video area. flushVideoDelayQueue() above
        // only discards not-yet-due queued frames -- it doesn't touch what's
        // already been presented to the screen -- so an explicit invalid
        // frame is the actual fix. Not needed with delay==0/direct routing:
        // there source-clearing already reaches the VideoOutput sink
        // directly through Qt's own normal handling.
        if (source.isEmpty() && m_targetVideoSink)
            m_targetVideoSink->setVideoFrame(QVideoFrame());
    });

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
        updateDvdSubtitleImage(); // keep the DVD subtitle overlay in sync

        // A transition cell's own (irrelevant) buttons are hidden until
        // playback reaches the real, frozen menu cell -- see
        // m_menuButtonsRevealMs's own doc comment. Re-query
        // dvdMenuButtons()/dvdMenuHighlightUrl() (plain getters, only
        // re-evaluated by QML when dvdMenuChanged fires) the one time
        // this is crossed.
        if (m_menuVts >= 0 && !m_menuButtonsRevealed
            && pos >= m_menuButtonsRevealMs) {
            m_menuButtonsRevealed = true;
            dvdLog(QStringLiteral("dvd menu: buttons revealed at pos=%1 "
                                  "(threshold=%2)")
                           .arg(pos).arg(m_menuButtonsRevealMs));
            emit dvdMenuChanged();
        }
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
                // A pause the backend produced on its own, not through our
                // pause() gateway (the only place that legitimately sets this
                // flag), is the same class of asynchronous-pipeline race
                // documented above for m_resumeGuardPos -- observed shortly
                // after playback starts, more often in installed/deployed
                // builds than a locally-run one. Recover instead of leaving
                // the user staring at a video they never asked to pause.
                if (state == QMediaPlayer::PausedState && !m_intentionalPause
                    && m_lastPlaybackState == QMediaPlayer::PlayingState
                    && !m_dvdDevice && m_menuVts < 0) {
                    m_player->play();
                }
                m_intentionalPause = false;
                m_lastPlaybackState = state;
            });

    connect(m_player, &QMediaPlayer::sourceChanged,
            this, &PlayerController::mediaInfoChanged);
    // A-B markers and external subtitles are per-file; reset on media change.
    connect(m_player, &QMediaPlayer::sourceChanged, this, [this]() {
        m_loadedMediaSetupDone = false;
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

        // Parse chapters for local file sources (skip DVD/Blu-ray devices and
        // streams -- both keep their own chapter lists, see blurayChapters()/
        // dvdChapters(), populated independently of this handler).
        m_chapters.clear();
        if (!m_dvdDevice && !m_blurayDevice) {
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
    delete m_blurayDisc; // not a QObject, unlike m_blurayDevice (parented to `this`)
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
    // A Blu-ray title's "source" is a hint URL built from the disc's own
    // folder path (via QUrl::fromLocalFile()), which isLocalFile() sees as
    // a real local file even though it isn't one -- without this guard the
    // preview player tried (and failed) to open the disc's root folder as
    // a plain video file on every seek-bar hover (found via vivace.log
    // during real-disc testing: "preview player error: ... Could not open
    // file"), same reason DVD is already excluded here.
    const bool localFile = !m_dvdDevice && !m_blurayDevice && src.isLocalFile();
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

QStringList PlayerController::dvdAudioTrackLabels() const
{
    QStringList labels;
    // While just the disc's own menu is showing (no title loaded yet,
    // m_dvdCurrentTitle <= 0), m_player->audioTracks() reflects whatever
    // the MENU's own device happens to carry (often none), not the
    // eventual title's -- showing IFO-declared labels directly here is
    // the only way Audio > Track has anything to show before the user
    // ever presses the disc's own "play" button (found 2026-08-16, "Bug
    // 7": the menu was empty the whole time the disc's own menu was up).
    if (m_dvdCurrentTitle <= 0) {
        for (const DvdIfo::AudioStream &s : m_dvdAudioStreams) {
            QString label = tr("Track %1").arg(s.id + 1);
            if (!s.language.isEmpty()) {
                const QString native = QLocale(s.language).nativeLanguageName();
                label = native.isEmpty() ? s.language.toUpper() : native;
            }
            labels << label;
        }
        return labels;
    }
    // Once a real title is loaded, cap to what FFmpeg actually finds
    // multiplexed (m_dvdAudioStreams' own doc comment: a disc can declare
    // more audio streams than it actually records), so the menu doesn't
    // offer entries that silently do nothing.
    const QList<QMediaMetaData> tracks = m_player->audioTracks();
    labels.reserve(tracks.size());
    for (int i = 0; i < tracks.size(); ++i) {
        QString label = tr("Track %1").arg(i + 1);
        if (i < m_dvdAudioStreams.size()
            && !m_dvdAudioStreams.at(i).language.isEmpty()) {
            const QString lang = m_dvdAudioStreams.at(i).language;
            const QString native = QLocale(lang).nativeLanguageName();
            label = native.isEmpty() ? lang.toUpper() : native;
        }
        labels << label;
    }
    return labels;
}

QStringList PlayerController::subtitleTrackLabels() const
{
    return trackLabels(m_player->subtitleTracks());
}

void PlayerController::open(const QList<QUrl> &urls)
{
    if (urls.isEmpty())
        return;

    // Clear any stale DVD menu overlay state up front, unconditionally, no
    // matter what kind of new open this turns out to be (another DVD, a
    // plain file, a directory, ...). Found necessary 2026-08-16: dropping a
    // SECOND DVD while a FIRST disc's menu was showing left its buttons/
    // highlight active on top of the newly-opened disc's video, since
    // openDvd() itself never reset this state. Rather than track down and
    // patch every individual code path that can end up playing something
    // new (openDvd(), playIndex(), ...) and risk missing one -- exactly the
    // class of mistake that caused this bug in the first place -- clear it
    // once, here, at the single top-level entry point every kind of "open"
    // funnels through. leaveMenu() is a safe no-op when no menu is active.
    leaveMenu();

    // A dropped/opened folder is a DVD (VIDEO_TS), a Blu-ray disc (BDMV), or
    // a plain media folder.
    if (urls.size() == 1 && urls.first().isLocalFile()
        && QFileInfo(urls.first().toLocalFile()).isDir()) {
        if (!openDvd(urls.first()) && !openBluray(urls.first()))
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
                detachCurrentPlaylistFile();
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

    // The in-memory playlist is "backed by" a file only right after loading
    // that file; opening plain media detaches it (see currentPlaylistFile's
    // doc comment in the header).
    const QUrl newCurrentPlaylistFile = openedPlaylist ? urls.first() : QUrl();
    if (newCurrentPlaylistFile != m_currentPlaylistFile) {
        m_currentPlaylistFile = newCurrentPlaylistFile;
        emit currentPlaylistFileChanged();
    }

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
    detachCurrentPlaylistFile();
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
        leaveMenu(); // was a partial, hand-duplicated reset here; see its own doc comment
        m_menus = {};
        emit dvdPlaybackChanged();
    }
    // Same for a Blu-ray disc: fully detach so a plain file/playlist item
    // played next doesn't leave stale BD title/chapter state behind.
    if (m_blurayDisc) {
        if (m_blurayDevice) {
            m_blurayDevice->deleteLater();
            m_blurayDevice = nullptr;
        }
        delete m_blurayDisc;
        m_blurayDisc = nullptr;
        m_blurayCurrentTitleIndex = -1;
        emit blurayPlaybackChanged();
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
    // TEMP DIAGNOSTIC (2026-08-16, CARS menu investigation): dump every
    // parsed menu PGC's raw structure so the disc's real navigation graph
    // can be inspected directly, without reimplementing IFO parsing
    // externally. Gated so it never runs in normal use.
    if (qEnvironmentVariableIsSet("VIVACE_DVD_DUMP_MENUS")) {
        auto dumpDomain = [](const QString &label, const DvdMenu::Domain &dom) {
            if (!dom.isValid()) {
                dvdLog(QStringLiteral("dumpMenus: %1 INVALID").arg(label));
                return;
            }
            for (int i = 0; i < dom.pgcs.size(); ++i) {
                const DvdMenu::Pgc &p = dom.pgcs.at(i);
                dvdLog(QStringLiteral("dumpMenus: %1 pgc=%2 menuId=%3 entry=%4 "
                                      "cells=%5 pre=%6 post=%7 cellCmds=%8 "
                                      "stillTime(last)=%9")
                               .arg(label).arg(i + 1).arg(p.menuId).arg(p.entry)
                               .arg(p.cells.size()).arg(p.preCommands.size())
                               .arg(p.postCommands.size()).arg(p.cellCommands.size())
                               .arg(p.cells.isEmpty() ? -1 : p.cells.last().stillTime));
            }
        };
        dumpDomain(QStringLiteral("vmgm"), m_menus.vmgm);
        for (auto it = m_menus.vtsm.constBegin(); it != m_menus.vtsm.constEnd(); ++it)
            dumpDomain(QStringLiteral("vtsm%1").arg(it.key()), it.value());
    }
    m_vm.reset();
    m_dvdAudioChosenByUser = false;
    m_dvdSubtitleChosenByUser = false;
    // Audio/Subtitle > Track menus otherwise show nothing at all until a
    // title is actually playing (m_dvdAudioStreams/m_dvdSubtitleStreams are
    // normally only populated by applyDvdTitle()) -- but a DVD's declared
    // streams are a per-VTS IFO fact, known as soon as the disc is parsed,
    // well before the user ever leaves the root menu (found 2026-08-16: the
    // menus were genuinely empty while just the disc menu was showing).
    // Pre-populate from the same "main" (longest) title dvdPlayMainTitle()
    // would eventually pick, so the track labels/languages are already
    // there to look at -- and, for the (usual) single-VTS-disc case,
    // exactly right regardless of which title actually ends up playing.
    if (!titles.isEmpty()) {
        const auto main = std::max_element(
                titles.constBegin(), titles.constEnd(),
                [](const DvdIfo::Title &a, const DvdIfo::Title &b) {
                    return a.durationMs < b.durationMs;
                });
        m_dvdAudioStreams = main->audioStreams;
        m_dvdSubtitleStreams = main->subtitleStreams;
        emit trackLabelsChanged();
    }

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

    dvdLog(QStringLiteral("openDvd: titles=%1 hasMenus=%2 useFirstPlay=%3 "
                          "hasFirstPlay=%4 menusEnabled=%5")
                   .arg(titles.size()).arg(m_menus.hasMenus())
                   .arg(m_dvdUseFirstPlay).arg(m_menus.hasFirstPlay())
                   .arg(m_dvdMenusEnabled));

    if (!titles.isEmpty()) {
        // Debug aids: VIVACE_DVD_TITLE forces a title number,
        // VIVACE_DVD_CHAPTER additionally starts at a chapter (1-based),
        // VIVACE_DVD_SEEK seeks to a title-global time (ms) after load.
        // VIVACE_DVD_AUTOSELECT=N auto-activates menu button N (or the disc's
        // own default selection if N<=0) a few seconds after any interactive
        // menu appears, to script past disc menus headlessly (see
        // playMenuPgc()). VIVACE_DVD_FORCE_SUB_LANG overrides which language
        // the preferred-subtitle match looks for (see applyDvdTitle()),
        // without needing to change Settings.
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
    detachCurrentPlaylistFile();

    if (m_dvdDevice)
        m_dvdDevice->deleteLater();
    m_dvdDevice = device;
    m_dvdCurrentTitle = -1;
    m_dvdRunStartCell = -1;
    m_dvdRunEndCell = -1;
    emit dvdPlaybackChanged();

    m_pendingStreamTitle = dvdDiscName();
    m_player->setSourceDevice(device, QUrl::fromLocalFile(videoTs));
    m_pendingAutoPlay = true;
    m_player->play();
    return true;
}

bool PlayerController::openBluray(const QUrl &folder)
{
    if (!folder.isLocalFile())
        return false;

    QDir dir(folder.toLocalFile());
    // Accept either the disc root (containing BDMV/) or BDMV itself.
    if (dir.dirName().compare(QStringLiteral("BDMV"), Qt::CaseInsensitive) == 0)
        dir.cdUp();
    const QString discRoot = dir.absolutePath();
    if (!BlurayDisc::isBlurayFolder(discRoot))
        return false;

    auto *disc = new BlurayDisc;
    if (!disc->open(discRoot)) {
        delete disc;
        emit errorMessage(tr("Could not open the Blu-ray disc (it may be "
                             "encrypted, or use an unsupported format)."));
        return false;
    }
    const int mainIndex = disc->mainTitleListIndex();
    if (mainIndex < 0) {
        delete disc;
        emit errorMessage(tr("No playable titles were found on this "
                             "Blu-ray disc."));
        return false;
    }

    // Detach any DVD state first (a folder can't be both, but keep the
    // invariant that only one disc type is "active" at a time).
    if (m_dvdDevice) {
        m_dvdDevice->deleteLater();
        m_dvdDevice = nullptr;
        m_dvdCurrentTitle = -1;
        leaveMenu();
        m_menus = {};
        emit dvdPlaybackChanged();
    }
    if (m_blurayDevice) {
        m_blurayDevice->deleteLater();
        m_blurayDevice = nullptr;
    }
    delete m_blurayDisc;
    m_blurayDisc = disc;
    m_blurayDir = discRoot;
    emit blurayPlaybackChanged();

    playBlurayTitle(mainIndex);
    return true;
}

void PlayerController::playBlurayTitle(int titleListIndex)
{
    if (!m_blurayDisc)
        return;
    if (!m_blurayDisc->selectTitle(titleListIndex)) {
        emit errorMessage(tr("This Blu-ray title could not be selected."));
        return;
    }
    QIODevice *device = m_blurayDisc->createDevice(this);
    if (!device) {
        emit errorMessage(tr("This Blu-ray title could not be opened."));
        return;
    }
    if (!device->open(QIODevice::ReadOnly)) {
        emit errorMessage(tr("This Blu-ray title could not be opened."));
        delete device;
        return;
    }

    saveCurrentPosition();
    m_player->stop();
    m_playlist->clear();
    detachCurrentPlaylistFile();

    if (m_blurayDevice)
        m_blurayDevice->deleteLater();
    m_blurayDevice = device;
    m_blurayCurrentTitleIndex = titleListIndex;
    emit blurayPlaybackChanged();

    // A unique hint URL per title -- QMediaPlayer ignores a setSourceDevice()
    // call whose hint URL is unchanged from the previous one (same reason DVD
    // titles/cells each get their own hint URL).
    QUrl hint = QUrl::fromLocalFile(m_blurayDir);
    hint.setQuery(QStringLiteral("bdTitle=%1").arg(titleListIndex));
    m_pendingStreamTitle = QDir(m_blurayDir).dirName();
    m_player->setSourceDevice(device, hint);
    m_pendingAutoPlay = true;
    m_player->play();
}

QVariantList PlayerController::blurayTitles() const
{
    QVariantList rows;
    if (!m_blurayDisc)
        return rows;
    const auto &titles = m_blurayDisc->titles();
    for (int i = 0; i < titles.size(); ++i) {
        rows << QVariantMap {
            { QStringLiteral("index"), i },
            { QStringLiteral("label"),
              tr("Title %1 (%2)").arg(i + 1)
                      .arg(formatDuration(titles.at(i).durationMs)) }
        };
    }
    return rows;
}

QVariantList PlayerController::blurayChapters() const
{
    QVariantList rows;
    if (!m_blurayDisc || m_blurayCurrentTitleIndex < 0)
        return rows;
    const auto &titles = m_blurayDisc->titles();
    if (m_blurayCurrentTitleIndex >= titles.size())
        return rows;
    const auto &chapters = titles.at(m_blurayCurrentTitleIndex).chapters;
    for (const BlurayDisc::Chapter &c : chapters) {
        rows << QVariantMap {
            { QStringLiteral("label"), c.label },
            { QStringLiteral("startMs"), c.startMs }
        };
    }
    return rows;
}

int PlayerController::blurayChapterIndexAt(qint64 ms) const
{
    if (!m_blurayDisc || m_blurayCurrentTitleIndex < 0)
        return -1;
    const auto &titles = m_blurayDisc->titles();
    if (m_blurayCurrentTitleIndex >= titles.size())
        return -1;
    const auto &chapters = titles.at(m_blurayCurrentTitleIndex).chapters;
    int index = -1;
    for (int i = 0; i < chapters.size(); ++i) {
        if (chapters.at(i).startMs <= ms)
            index = i;
        else
            break;
    }
    return index;
}

int PlayerController::blurayVisibleAudioTrackCount() const
{
    if (!m_blurayDisc || m_blurayCurrentTitleIndex < 0)
        return -1;
    const auto &titles = m_blurayDisc->titles();
    if (m_blurayCurrentTitleIndex >= titles.size())
        return -1;
    // Cap to the SMALLER of what the disc's own CLPI stream table declares
    // and what FFmpeg actually finds multiplexed -- NOT just the FFmpeg
    // count. Investigated 2026-08-17 (user report: a disc with a single
    // declared Japanese audio track showed a spurious "Track 2"): the disc
    // genuinely declares only 1 audio stream (confirmed directly against
    // libbluray's own BLURAY_CLIP_INFO::audio_stream_count for this title's
    // one and only clip), but Qt's FFmpeg backend finds a 2nd, UNDECLARED
    // elementary stream physically present in the raw M2TS mux (metadata
    // dump: codec "MP3", no duration ever established -- consistent with a
    // leftover/scratch authoring-tool artifact, not a real user-selectable
    // track). A real BD player never offers this: it strictly follows the
    // disc's own STN table (which is what BlurayDisc::audioLanguages comes
    // from), never raw TS content-sniffing. So `declared` is authoritative
    // for how MANY tracks should be user-selectable at all here, not just a
    // source of NAMES for whatever FFmpeg happens to find -- the opposite
    // capping direction from dvdAudioTrackLabels() (a DVD can declare MORE
    // streams than end up switchable; here the disc can declare FEWER than
    // FFmpeg's raw demux finds). Whichever list is shorter wins.
    return qMin(m_player->audioTracks().size(),
                titles.at(m_blurayCurrentTitleIndex).audioLanguages.size());
}

int PlayerController::blurayVisibleSubtitleTrackCount() const
{
    if (!m_blurayDisc || m_blurayCurrentTitleIndex < 0)
        return -1;
    const auto &titles = m_blurayDisc->titles();
    if (m_blurayCurrentTitleIndex >= titles.size())
        return -1;
    // Same reasoning as blurayVisibleAudioTrackCount() above.
    return qMin(m_player->subtitleTracks().size(),
                titles.at(m_blurayCurrentTitleIndex).subtitleLanguages.size());
}

QStringList PlayerController::blurayAudioTrackLabels() const
{
    QStringList labels;
    const int count = blurayVisibleAudioTrackCount();
    if (count < 0)
        return labels;
    const QList<QString> &declared =
            m_blurayDisc->titles().at(m_blurayCurrentTitleIndex).audioLanguages;
    labels.reserve(count);
    for (int i = 0; i < count; ++i) {
        const QString display = discLanguageDisplayName(declared.at(i));
        labels << (display.isEmpty() ? tr("Track %1").arg(i + 1) : display);
    }
    return labels;
}

QStringList PlayerController::bluraySubtitleTrackLabels() const
{
    QStringList labels;
    const int count = blurayVisibleSubtitleTrackCount();
    if (count < 0)
        return labels;
    const QList<QString> &declared =
            m_blurayDisc->titles().at(m_blurayCurrentTitleIndex).subtitleLanguages;
    labels.reserve(count);
    for (int i = 0; i < count; ++i) {
        const QString display = discLanguageDisplayName(declared.at(i));
        labels << (display.isEmpty() ? tr("Track %1").arg(i + 1) : display);
    }
    return labels;
}

bool PlayerController::applyDvdTitle(const DvdIfo::Title &title,
                                     int fromCellIndex,
                                     qint64 positionOffsetMs,
                                     qint64 startSector)
{
    // Captured before anything below changes: whether this rebuild is for
    // the SAME title (a seek, run-boundary auto-advance, or chapter jump
    // within it -- all of which call this function again) rather than a
    // genuinely new one, and the audio track actually active a moment ago
    // -- see m_dvdPendingAudioTrackRestore's own doc comment for why this
    // needs to survive past this function returning.
    const bool sameTitle = m_dvdCurrentTitle == title.titleNumber;
    const int previousAudioTrack = m_player->activeAudioTrack();
    const int previousSubtitleTrack = m_dvdActiveSubtitleTrack;

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
    dvdLog(QStringLiteral("applyDvdTitle: title=%1 vts=%2 cells=%3..%4")
                   .arg(title.titleNumber).arg(title.vtsNumber)
                   .arg(fromCellIndex).arg(runEnd - 1));

    m_dvdRunStartCell = fromCellIndex;
    m_dvdRunEndCell = runEnd;
    m_player->setLoops(QMediaPlayer::Once); // titles play once (menus loop)
    saveCurrentPosition();
    m_player->stop();
    m_playlist->clear();
    detachCurrentPlaylistFile();

    if (m_dvdDevice)
        m_dvdDevice->deleteLater();
    m_dvdDevice = device;
    m_dvdCurrentTitle = title.titleNumber;
    m_dvdPositionOffsetMs = positionOffsetMs;
    m_dvdAudioStreams = title.audioStreams;
    m_dvdSubtitleStreams = title.subtitleStreams;
    // A same-title rebuild (seek / run-boundary advance / chapter jump
    // within this title) must PRESERVE whatever subtitle track is already
    // active -- including an explicit Subtitles > Track pick that has
    // nothing to do with either the language-preference default or the
    // disc's own on-screen subtitle menu -- rather than resetting to off
    // and recomputing from scratch every time (found 2026-08-16: dragging
    // the seek bar was silently turning a chosen subtitle back off). Still
    // needs a fresh build though: the subtitle event index is scoped to
    // the CURRENT RUN's own cells (see startDvdSubtitleTrackBuild()'s own
    // doc comment), which just changed.
    if (sameTitle && previousSubtitleTrack >= 0
        && previousSubtitleTrack < m_dvdSubtitleStreams.size()) {
        m_dvdSubtitleTrack = {};
        m_dvdSubtitleImageUrl.clear();
        emit dvdSubtitleImageChanged();
        startDvdSubtitleTrackBuild(previousSubtitleTrack);
    } else {
        setActiveDvdSubtitleTrack(-1); // off unless a preferred-language match is found below
        // VIVACE_DVD_FORCE_SUB_LANG overrides which language to look for (not
        // whether subtitles-by-default is on at all) -- lets a test run exercise
        // the matching logic without touching Settings; see the debug-aids note
        // above openDvd().
        QString prefSubLangs = m_preferredSubtitleLanguages;
        if (qEnvironmentVariableIsSet("VIVACE_DVD_FORCE_SUB_LANG"))
            prefSubLangs = QString::fromLocal8Bit(qgetenv("VIVACE_DVD_FORCE_SUB_LANG"));
        if (m_subtitlesByDefault && !prefSubLangs.trimmed().isEmpty()) {
            const int index = findDvdSubtitleTrackByLanguages(
                    m_dvdSubtitleStreams, prefSubLangs);
            dvdLog(QStringLiteral("dvd subtitle: preferred='%1' matchedIndex=%2 "
                                  "declared=%3")
                           .arg(prefSubLangs).arg(index)
                           .arg(m_dvdSubtitleStreams.size()));
            if (index >= 0)
                setActiveDvdSubtitleTrack(index);
        }
        // The disc's OWN subtitle menu (Subtitles ▸ ... reachable from the
        // DVD's root menu, e.g. "字幕") works by having its buttons set
        // SPRM2 (the VM's subpicture-stream register) via a System-Set
        // command, then link back into the title -- confirmed by decoding
        // a real disc's menu-PGC command bytes with a from-scratch VM
        // simulation (2026-08-16): the button DOES correctly update
        // m_vm.sprm[2], but nothing previously read it back out and
        // applied it to actual playback, so choosing a subtitle from the
        // disc's own menu silently did nothing.
        // SPRM2's real DVD-Video encoding (confirmed empirically against
        // this disc's own button commands, NOT a plain 0-based index as
        // first assumed): bit 6 (0x40) is a "subtitle stream selected"
        // flag, bits 0-5 are the 0-based stream index -- a real "日本語"/
        // "英語" button wrote 0x40/0x41 (ON, stream 0/1), while "字幕なし"
        // (no subtitle) wrote a clean 0 (bit 6 clear, no index).
        // Gated on m_dvdSubtitleChosenByUser (see its own doc comment):
        // this same disc has a commands-only, never-shown "VTS menu
        // entry" PGC that ALSO unconditionally zeros sprm[2] (matching
        // the disc-authored "explicit off" pattern byte-for-byte) before
        // ever linking into the real, visible root menu -- so a bit/value
        // check on sprm[2] alone cannot tell "the disc's own built-in
        // default" apart from "the user really chose this in the
        // subtitle menu". Without this gate, EVERY playback of this disc
        // would have silently overridden the language-preference default
        // above with the disc's built-in "no subtitle", even when the
        // user never opened the subtitle menu at all (caught during this
        // fix's own verification, not reported by the user -- see the
        // 2026-08-16 fix notes). Once the user HAS made a real choice, it
        // overrides the language-preference default (last write wins,
        // matching real DVD player behavior). Deliberately INSIDE this
        // "fresh title" branch, not applied unconditionally on every
        // rebuild: sprm[2] doesn't change on its own between rebuilds, so
        // re-applying it on a same-title seek could resurrect a stale
        // disc-menu choice over a since-changed explicit Subtitles > Track
        // pick (which the branch above already restores correctly without
        // needing this at all).
        if (m_dvdSubtitleChosenByUser) {
            if (m_vm.sprm[2] & 0x40) {
                const int idx = int(m_vm.sprm[2] & 0x3F);
                if (idx < title.subtitleStreams.size())
                    setActiveDvdSubtitleTrack(idx);
            } else if (m_vm.sprm[2] == 0) {
                setActiveDvdSubtitleTrack(-1);
            }
        }
    }
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
    // Read by handleMediaStatus()'s LoadedMedia handler once this new
    // source finishes loading -- see m_dvdPendingAudioTrackRestore's own
    // doc comment. Only meaningful for a same-title rebuild; a fresh title
    // still gets the ordinary preferred-audio-language default there.
    m_dvdPendingAudioTrackRestore =
            (sameTitle && previousAudioTrack >= 0) ? previousAudioTrack : -1;
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
    dvdLog(QStringLiteral("dvdTitles: device=%1 m_dvdTitles.size=%2 currentTitle=%3")
                   .arg(m_dvdDevice != nullptr).arg(m_dvdTitles.size())
                   .arg(m_dvdCurrentTitle));
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

std::optional<bool> PlayerController::probeFollowAction(const DvdVm::Action &a,
                                                         int vts,
                                                         DvdVm::Machine &vm,
                                                         int depth) const
{
    using K = DvdVm::Action::Kind;
    switch (a.kind) {
    case K::LinkPgcn:
        return probeMenuLeadsToButtonsRec(vts, a.data1, vm, depth + 1);
    case K::JumpSsVtsm: {
        const DvdMenu::Domain *dom = menuDomain(a.data1);
        const int p = dom ? dom->entryPgc(a.data3) : 0;
        if (!p)
            return false;
        return probeMenuLeadsToButtonsRec(a.data1, p, vm, depth + 1);
    }
    case K::JumpSsVmgmMenu: {
        const int p = m_menus.vmgm.entryPgc(a.data1);
        if (!p)
            return false;
        return probeMenuLeadsToButtonsRec(0, p, vm, depth + 1);
    }
    case K::JumpSsVmgmPgc:
        return probeMenuLeadsToButtonsRec(0, a.data1, vm, depth + 1);
    case K::CallSsVtsm: {
        const DvdMenu::Domain *dom = menuDomain(vts);
        const int p = dom ? dom->entryPgc(a.data1) : 0;
        if (!p)
            return false;
        return probeMenuLeadsToButtonsRec(vts, p, vm, depth + 1);
    }
    case K::CallSsVmgmMenu: {
        const int p = m_menus.vmgm.entryPgc(a.data1);
        if (!p)
            return false;
        return probeMenuLeadsToButtonsRec(0, p, vm, depth + 1);
    }
    case K::JumpTt:
    case K::JumpVtsTt:
    case K::JumpVtsPtt:
    case K::Exit:
    case K::JumpSsFp:
        // A jump straight to real title content (or an exit/first-play
        // loop-back) -- definitively NOT an interactive menu reached via
        // this candidate, regardless of how many buttonless redirector
        // PGCs led here.
        return false;
    case K::None:
    case K::Nop:
    default:
        // The command block completed without linking anywhere -- the
        // caller falls through to checking THIS pgc's own cells.
        return std::nullopt;
    }
}

bool PlayerController::probeMenuLeadsToButtonsRec(int vts, int pgcNumber,
                                                  DvdVm::Machine &vm,
                                                  int depth) const
{
    if (depth > 12)
        return false;
    const DvdMenu::Domain *dom = menuDomain(vts);
    if (!dom || pgcNumber < 1 || pgcNumber > dom->pgcs.size())
        return false;
    const DvdMenu::Pgc &pgc = dom->pgcs.at(pgcNumber - 1);
    const bool trace = qEnvironmentVariableIsSet("VIVACE_DVD_DUMP_MENUS");

    if (!pgc.preCommands.isEmpty()) {
        const DvdVm::Action a = vm.run(pgc.preCommands);
        if (trace) {
            dvdLog(QStringLiteral("probeTrace: depth=%1 vts=%2 pgc=%3 PRE "
                                  "actionKind=%4 data1=%5 data2=%6 data3=%7")
                           .arg(depth).arg(vts).arg(pgcNumber).arg(int(a.kind))
                           .arg(a.data1).arg(a.data2).arg(a.data3));
        }
        if (auto resolved = probeFollowAction(a, vts, vm, depth))
            return *resolved;
        // std::nullopt: fell through -- check this pgc's own cells below.
    }
    if (menuPgcHasButtons(dom, pgcNumber))
        return true;
    // No buttons of its own. If this is a non-interactive PGC that plays
    // through and auto-advances (its last cell's still_time is 0, i.e. it
    // does NOT freeze waiting for input) and it has post-commands, follow
    // those too -- exactly the "intro plays, then post-commands redirect"
    // chain a real disc uses, which real playback would eventually reach
    // on its own; simulating it here lets a single buttonless intro that
    // chains into the real menu still count as "this candidate works".
    const bool waitsForInput = !pgc.cells.isEmpty() && pgc.cells.last().stillTime != 0;
    if (!waitsForInput && !pgc.postCommands.isEmpty()) {
        const DvdVm::Action a = vm.run(pgc.postCommands);
        if (trace) {
            dvdLog(QStringLiteral("probeTrace: depth=%1 vts=%2 pgc=%3 POST "
                                  "actionKind=%4 data1=%5 data2=%6 data3=%7")
                           .arg(depth).arg(vts).arg(pgcNumber).arg(int(a.kind))
                           .arg(a.data1).arg(a.data2).arg(a.data3));
        }
        if (auto resolved = probeFollowAction(a, vts, vm, depth))
            return *resolved;
    }
    if (trace) {
        dvdLog(QStringLiteral("probeTrace: depth=%1 vts=%2 pgc=%3 DEAD END "
                              "(no buttons, no further redirect)")
                       .arg(depth).arg(vts).arg(pgcNumber));
    }
    return false;
}

bool PlayerController::probeMenuLeadsToButtons(int vts, int pgcNumber) const
{
    DvdVm::Machine scratchVm; // independent of the real m_vm -- no side effects
    return probeMenuLeadsToButtonsRec(vts, pgcNumber, scratchVm, 0);
}

bool PlayerController::runFirstPlay()
{
    if (!m_menus.hasFirstPlay())
        return false;
    m_vm.reset();
    const DvdVm::Action a = m_vm.run(m_menus.firstPlay.preCommands);
    dvdLog(QStringLiteral("runFirstPlay: actionKind=%1 data1=%2")
                   .arg(int(a.kind)).arg(a.data1));
    if (a.kind != DvdVm::Action::None && a.kind != DvdVm::Action::Nop)
        return performNavAction(a, 0, 0, 0); // First-Play lives in the VMGM domain, no current PGC
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
    // intro). Probing (via probeMenuLeadsToButtons(), which simulates the
    // entry PGC's own pre/post-command redirect chain through a scratch VM
    // -- NOT just checking whether the entry PGC's own cells directly
    // contain a NAV pack with buttons) picks the right one regardless of
    // how many buttonless decision-tree/redirector PGCs sit in between.
    // Needed 2026-08-16 for a disc (Pixar's Cars) whose real interactive
    // menu pages are reached only after a ~20-instruction, buttonless
    // decision-tree PGC that the shallow check could never see past.
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
        const bool hasBtns = p && probeMenuLeadsToButtons(c.vts, p);
        dvdLog(QStringLiteral("enterDefaultMenu: cand vts=%1 menuId=%2 pgc=%3 "
                              "hasButtons=%4")
                       .arg(c.vts).arg(c.menuId).arg(p).arg(hasBtns));
        if (hasBtns)
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

bool PlayerController::playMenuPgc(int vts, int pgcNumber, bool runPre, int depth,
                                   int startCell)
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
    // Only run them from the PGC's own natural start -- a LinkPgn jump into
    // the middle of a multi-page PGC must not re-run them.
    if (runPre && startCell == 0 && !pgc.preCommands.isEmpty()) {
        const DvdVm::Action a = m_vm.run(pgc.preCommands);
        if (a.kind != DvdVm::Action::None && a.kind != DvdVm::Action::Nop)
            return performNavAction(a, vts, pgcNumber, depth + 1);
    }
    if (pgc.cells.isEmpty()) {
        if (!pgc.postCommands.isEmpty()) {
            const DvdVm::Action a = m_vm.run(pgc.postCommands);
            if (a.kind != DvdVm::Action::None && a.kind != DvdVm::Action::Nop)
                return performNavAction(a, vts, pgcNumber, depth + 1);
        }
        return false;
    }

    // A multi-cell PGC can have SEVERAL cells each independently marked
    // still_time != 0, not just its true last cell -- found 2026-08-15 on a
    // 4-cell chapter-index PGC where every cell is its own "page" (a
    // LinkPgn(N) button jumps straight to page N's own entry cell, which is
    // ITSELF a still cell). So freeze at the FIRST still cell reached from
    // `startCell`, not unconditionally at the PGC's true last cell -- else
    // every page would visibly play through to the end before freezing,
    // and jumping into page 2 would incorrectly re-play page 1's cells too.
    const int start = qBound(0, startCell, pgc.cells.size() - 1);
    int freezeIdx = -1;
    for (int i = start; i < pgc.cells.size(); ++i) {
        if (pgc.cells.at(i).stillTime != 0) {
            freezeIdx = i;
            break;
        }
    }
    const int endIdx = freezeIdx >= 0 ? freezeIdx : pgc.cells.size() - 1;
    const QList<DvdIfo::Cell> playCells = pgc.cells.mid(start, endIdx - start + 1);
    const bool freezesAtEnd = freezeIdx >= 0;

    DvdTitleDevice *device =
            DvdTitleDevice::createFromMenuCells(dom->vobPath, playCells, this);
    if (!device || !device->open(QIODevice::ReadOnly)) {
        delete device;
        qWarning() << "dvd menu: cannot open menu VOB" << dom->vobPath;
        return false;
    }

    saveCurrentPosition();
    m_player->stop();
    m_playlist->clear();
    detachCurrentPlaylistFile();
    if (m_dvdDevice)
        m_dvdDevice->deleteLater();
    m_dvdDevice = device;
    m_dvdCurrentTitle = -1;
    m_dvdRunStartCell = -1;
    m_dvdRunEndCell = -1;
    m_dvdPositionOffsetMs = 0;
    m_menuVts = vts;
    m_menuPgc = pgcNumber;

    // A multi-cell PGC can (and, on a real disc, does) have a completely
    // DIFFERENT button/highlight layout per cell -- e.g. a brief
    // transition/logo cell with one dummy button, then the real static menu
    // cell with several real ones. parseButtons()/decodeSubpicture() return
    // the FIRST valid NAV pack they find scanning forward, so scanning the
    // WHOLE run would silently return the transition cell's (wrong) layout
    // whenever it has ANY valid NAV pack of its own -- confirmed against a
    // real disc (2026-08-15): scanning the whole PGC found the transition
    // cell's single dummy button (empty command) while the frozen cell
    // actually has 4 real, correctly-positioned buttons with real commands.
    // When this run freezes on its last cell (freezesAtEnd, computed
    // above), that's the cell the user is actually looking at and can
    // click, so parse buttons/highlight from JUST that cell's own sector
    // range.
    const DvdIfo::Cell &buttonScanFirst =
            freezesAtEnd ? playCells.last() : playCells.first();
    const DvdIfo::Cell &buttonScanLast = playCells.last();

    // Clickable buttons from the first NAV pack in the relevant cell range.
    m_menuButtons = DvdMenu::parseButtons(dom->vobPath,
                                          buttonScanFirst.firstSector,
                                          buttonScanLast.lastSector);

    // The disc's own subpicture highlight (button outlines) + palette.
    m_menuSpu = DvdMenu::decodeSubpicture(dom->vobPath,
                                          buttonScanFirst.firstSector,
                                          buttonScanLast.lastSector);
    memcpy(m_menuPalette, pgc.palette, sizeof(m_menuPalette));
    m_menuHighlightSel = -2; // invalidate the rendered-highlight cache
    m_menuHasButtons = !m_menuButtons.buttons.isEmpty();
    // Per-cell still_time on the frozen cell (see DvdIfo::Cell::stillTime's
    // own doc comment) is the disc's authoritative "freeze and wait here"
    // signal -- verified against real discs: a one-shot logo/intro WITH a
    // dummy button and real post-commands, and genuine multi-choice menus
    // (including ones where EVERY cell is independently still, one per
    // page), all had PGC-level still_time == 0, but each still-required
    // cell carries still_time == 0xFF on the per-cell field. Fall back to
    // "has a button and nowhere else to go" only if there's no explicit
    // still-cell at all, for discs that rely purely on repeated playback
    // instead.
    m_menuFreezeAtEnd = freezesAtEnd;
    m_menuWaitsForInput = m_menuFreezeAtEnd
            || (m_menuHasButtons && pgc.postCommands.isEmpty());
    int sel = m_vm.sprm[8] >> 10; // SPRM8 = highlighted button * 1024
    if (sel < 1 || sel > m_menuButtons.buttons.size())
        sel = m_menuButtons.startButton;
    if (sel < 1 || sel > m_menuButtons.buttons.size())
        sel = m_menuButtons.buttons.isEmpty() ? 0 : 1;
    m_menuSelected = sel;
    // Delay showing buttons/highlight until playback actually reaches the
    // CELL whose own NAV pack they were read from -- not just "the frozen
    // last cell" (freezesAtEnd's own case: since buttonScanFirst there is
    // ALREADY playCells.last(), the found sector necessarily falls in that
    // same last cell, so this reduces to the previous behaviour exactly)
    // or unconditionally "from the very start" otherwise. Needed
    // 2026-08-16 (Cars, "特典メニュー"/Bonus-menu submenu): a real disc's
    // multi-cell menu PGC can have NEITHER a per-cell freeze marker NOR
    // buttons in its very first cell -- only a LATER cell carries the
    // real, static button-bearing NAV pack, while the earlier cells play
    // a buttonless-LOOKING (but NAV-pack-bearing, hence previously found
    // immediately) entry animation. Showing the buttons/highlight from
    // cell 0 in that case rendered empty highlight rectangles over the
    // animation from the very first frame, well before the disc's own
    // intended reveal point.
    qint64 revealMs = 0;
    const qint64 foundSector = m_menuButtons.foundSector >= 0
            ? m_menuButtons.foundSector : m_menuSpu.foundSector;
    if (foundSector >= 0) {
        for (const DvdIfo::Cell &c : playCells) {
            if (foundSector >= c.firstSector && foundSector <= c.lastSector)
                break;
            revealMs += c.durationMs;
        }
    }
    m_menuButtonsRevealMs = revealMs;
    m_menuButtonsRevealed = m_menuButtonsRevealMs <= 0; // 0 = already revealed

    qInfo().nospace() << "dvd menu: showing vts " << vts << " pgc " << pgcNumber
                      << " (" << m_menuButtons.buttons.size() << " buttons)";
    dvdLog(QStringLiteral("playMenuPgc vts=%1 pgc=%2 startCell=%3 playCells=%4 "
                          "buttons=%5 hasButtons=%6 freezeCellStillTime=%7 "
                          "freezeAtEnd=%8 waitsForInput=%9 revealMs=%10 sel=%11")
                   .arg(vts).arg(pgcNumber).arg(start).arg(playCells.size())
                   .arg(m_menuButtons.buttons.size()).arg(m_menuHasButtons)
                   .arg(buttonScanFirst.stillTime)
                   .arg(m_menuFreezeAtEnd).arg(m_menuWaitsForInput)
                   .arg(m_menuButtonsRevealMs).arg(m_menuSelected));

    emit dvdPlaybackChanged();
    emit dvdMenuChanged();

    QUrl hint = QUrl::fromLocalFile(m_dvdDir);
    hint.setQuery(QStringLiteral("menu=%1:%2").arg(vts).arg(pgcNumber));
    m_fileSettings->remove(hint);
    m_pendingStreamTitle = dvdDiscName();
    // Native looping is only appropriate for the "no explicit still-cell,
    // just keep replaying while waiting" fallback -- when the disc marks
    // an explicit freeze (m_menuFreezeAtEnd), we want QMediaPlayer to
    // reach a genuine, one-time EndOfMedia so the EndOfMedia handler can
    // leave it frozen there, not silently restart the clip from position 0.
    m_player->setLoops((m_menuWaitsForInput && !m_menuFreezeAtEnd)
                               ? QMediaPlayer::Infinite
                               : QMediaPlayer::Once);
    m_player->setSourceDevice(device, hint);
    m_pendingAutoPlay = true;
    m_player->play();

    // Debug aid (see the VIVACE_DVD_TITLE family above openDvd()):
    // VIVACE_DVD_AUTOSELECT=N auto-activates button N (or the default
    // selection if N<=0) a few seconds after any interactive menu appears,
    // to script past disc menus headlessly.
    if (m_menuHasButtons && m_menuWaitsForInput
        && qEnvironmentVariableIsSet("VIVACE_DVD_AUTOSELECT")) {
        int btn = qgetenv("VIVACE_DVD_AUTOSELECT").toInt();
        if (btn <= 0)
            btn = m_menuSelected;
        QTimer::singleShot(4000, this, [this, btn]() { dvdMenuActivate(btn); });
    }
    return true;
}

bool PlayerController::runPgcPostCommands(int vts, int pgcNumber, int depth)
{
    const DvdMenu::Domain *dom = menuDomain(vts);
    if (dom && pgcNumber >= 1 && pgcNumber <= dom->pgcs.size()) {
        const DvdVm::Action a =
                m_vm.run(dom->pgcs.at(pgcNumber - 1).postCommands);
        if (a.kind != DvdVm::Action::None && a.kind != DvdVm::Action::Nop)
            return performNavAction(a, vts, pgcNumber, depth + 1);
    }
    // Nothing to chain to — don't dead-end: play the main title.
    return dvdPlayMainTitle();
}

bool PlayerController::runTitlePostCommands(const DvdIfo::Title &title)
{
    const DvdVm::Action a = m_vm.run(title.postCommands);
    dvdLog(QStringLiteral("runTitlePostCommands: title=%1 actionKind=%2 "
                          "data1=%3 data2=%4 data3=%5")
                   .arg(title.titleNumber).arg(int(a.kind)).arg(a.data1)
                   .arg(a.data2).arg(a.data3));
    if (a.kind == DvdVm::Action::None || a.kind == DvdVm::Action::Nop)
        return false; // nothing to chain to -- caller keeps its own "stop" path
    return performNavAction(a, title.vtsNumber, 0, 0);
}

bool PlayerController::performNavAction(const DvdVm::Action &a, int currentVts,
                                        int currentPgc, int depth)
{
    using K = DvdVm::Action::Kind;
    switch (a.kind) {
    case K::LinkPgcn:
        return playMenuPgc(currentVts, a.data1, true, depth + 1);
    case K::LinkPgn: {
        // "Link to program N (current PGC)" -- jumps straight to program
        // N's own entry cell within the PGC whose command block actually
        // produced this action (currentPgc -- NOT m_menuPgc, which can be
        // stale: see currentPgc's own doc comment in the header for the
        // real disc bug this distinction fixes). Found on a real disc's
        // chapter-index submenu, where each "page" of chapter buttons is
        // its own program/cell and a range button like "7-12" is
        // LinkPgn(2).
        int startCellIdx = 0;
        const DvdMenu::Domain *dom = menuDomain(currentVts);
        if (dom && currentPgc >= 1 && currentPgc <= dom->pgcs.size()) {
            const DvdMenu::Pgc &curPgc = dom->pgcs.at(currentPgc - 1);
            if (a.data1 >= 1 && a.data1 <= curPgc.programEntryCells.size())
                startCellIdx = curPgc.programEntryCells.at(a.data1 - 1);
        }
        return playMenuPgc(currentVts, currentPgc, false, depth + 1, startCellIdx);
    }
    case K::LinkTailPgc:
        // "Link to this PGC's own tail" -- per real disc behavior (a
        // button named like "Play Feature" on a menu whose only other
        // content is a skippable intro clip), this means run the PGC
        // whose command block produced this action (currentPgc, same
        // staleness reasoning as LinkPgn above) own post-commands right
        // now, exactly as if its cells had just finished playing
        // naturally. NOT "replay this menu from the start" (the old
        // default-case fallback, which just re-triggered the intro
        // animation the button was meant to skip past).
        return runPgcPostCommands(currentVts, currentPgc, depth);
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
    default: {
        // Relative links (top/next/prev PGC/cell/pg) — replay the current menu
        // as a best-effort for menu-lite.
        const int pgc = currentPgc >= 1 ? currentPgc : m_menuPgc;
        if (m_menuVts >= 0)
            return playMenuPgc(m_menuVts, pgc, false, depth + 1);
        return false;
    }
    }
}

bool PlayerController::playGlobalTitle(int titleNumber)
{
    dvdLog(QStringLiteral("playGlobalTitle: titleNumber=%1").arg(titleNumber));
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
    // Don't show a cell's buttons before playback has actually reached
    // that cell -- see m_menuButtonsRevealMs's own doc comment (a
    // transition/logo cell playing before the real, frozen menu cell has
    // its own, irrelevant button layout).
    if (m_player->position() < m_menuButtonsRevealMs)
        return {};
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
    // See dvdMenuButtons()'s identical guard: don't show the highlight
    // overlay before playback reaches the cell it actually belongs to.
    if (m_player->position() < m_menuButtonsRevealMs)
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

// ---- Real DVD movie subtitles (see dvdsubtitletrack.h) ----------------------

QStringList PlayerController::dvdSubtitleTrackLabels() const
{
    QStringList labels;
    for (const DvdIfo::SubtitleStream &s : m_dvdSubtitleStreams) {
        QString label = tr("Track %1").arg(s.id + 1);
        if (!s.language.isEmpty()) {
            const QString native = QLocale(s.language).nativeLanguageName();
            label = native.isEmpty() ? s.language.toUpper() : native;
        }
        labels << label;
    }
    return labels;
}

void PlayerController::setActiveDvdSubtitleTrack(int index)
{
    if (index == m_dvdActiveSubtitleTrack)
        return;
    m_dvdActiveSubtitleTrack = index;
    emit activeDvdSubtitleTrackChanged();
    m_dvdSubtitleTrack = {};
    m_dvdSubtitleImageUrl.clear();
    emit dvdSubtitleImageChanged();
    if (index >= 0 && index < m_dvdSubtitleStreams.size())
        startDvdSubtitleTrackBuild(index);
}

void PlayerController::startDvdSubtitleTrackBuild(int streamIndex)
{
    if (!m_dvdDevice || m_dvdCurrentTitle < 0)
        return;
    const DvdIfo::Title *title = currentDvdTitle();
    if (!title)
        return;

    // The specific cell run actually being played (see applyDvdTitle()'s own
    // single-timeline slicing, and m_dvdRunStartCell/m_dvdRunEndCell) -- not
    // necessarily the whole title -- and its ms-since-run-start timings, both
    // needed for DvdSubtitle::Track::build() to interpolate event timing
    // consistently with QMediaPlayer::position().
    if (m_dvdRunStartCell < 0 || m_dvdRunEndCell <= m_dvdRunStartCell)
        return;
    const QList<DvdIfo::Cell> cells = title->cells.mid(
            m_dvdRunStartCell, m_dvdRunEndCell - m_dvdRunStartCell);
    QList<qint64> cellStartsMs = title->cellStartsMs.mid(
            m_dvdRunStartCell, m_dvdRunEndCell - m_dvdRunStartCell);
    const qint64 base = cellStartsMs.isEmpty() ? 0 : cellStartsMs.first();
    for (qint64 &ms : cellStartsMs)
        ms -= base;
    const qint64 runDurationMs = title->durationMs - base;
    const QString videoTsDir = m_dvdDir;
    const int vtsNumber = title->vtsNumber;
    const int timeMapUnitSec = title->timeMapUnitSec;
    const QList<quint32> timeMapSectors = title->timeMapSectors;

    const int generation = ++m_dvdSubtitleBuildGeneration;
    m_dvdSubtitleBuildInFlight = true;
    emit dvdSubtitleLoadingChanged();

    QPointer<PlayerController> self(this);
    QThreadPool::globalInstance()->start(
            [self, generation, videoTsDir, vtsNumber, cells, cellStartsMs,
             runDurationMs, streamIndex, timeMapUnitSec, timeMapSectors,
             base]() {
        DvdSubtitle::Track track = DvdSubtitle::Track::build(
                videoTsDir, vtsNumber, cells, cellStartsMs, runDurationMs,
                streamIndex, timeMapUnitSec, timeMapSectors, base);
        QMetaObject::invokeMethod(qApp, [self, generation, track]() mutable {
            if (!self || generation != self->m_dvdSubtitleBuildGeneration)
                return; // title/track changed while this was building
            dvdLog(QStringLiteral("dvd subtitle: track built, events=%1")
                           .arg(track.eventCount()));
            // TEMP DIAGNOSTIC (2026-08-15): dump every event's start/stop/size
            // so a "only the first line ever shows" report can be checked
            // against the raw build data directly, not just live playback.
            {
                const QList<DvdSubtitle::Event> &evs = track.events();
                for (int i = 0; i < evs.size(); ++i) {
                    dvdLog(QStringLiteral("  event[%1] start=%2 stop=%3 "
                                          "spuBytes=%4")
                                   .arg(i).arg(evs.at(i).startMs)
                                   .arg(evs.at(i).stopMs)
                                   .arg(evs.at(i).spu.size()));
                }
            }
            self->m_dvdSubtitleTrack = std::move(track);
            self->m_dvdSubtitleBuildInFlight = false;
            emit self->dvdSubtitleLoadingChanged();
            self->updateDvdSubtitleImage();
        }, Qt::QueuedConnection);
    });
}

void PlayerController::updateDvdSubtitleImage()
{
    if (m_dvdActiveSubtitleTrack < 0 || m_dvdSubtitleTrack.isEmpty()) {
        if (!m_dvdSubtitleImageUrl.isEmpty()) {
            m_dvdSubtitleImageUrl.clear();
            emit dvdSubtitleImageChanged();
        }
        return;
    }
    const DvdIfo::Title *title = currentDvdTitle();
    // m_dvdSubtitleTrack's event times are "ms since the run's TRUE start"
    // (title->cellStartsMs.at(m_dvdRunStartCell), the cell's own un-truncated
    // start -- see DvdSubtitle::Track::build()'s own doc comment). The raw
    // player position is NOT that: it resets near 0 for whatever device is
    // currently open, and m_dvdPositionOffsetMs converts it to true
    // title-global time (the same "position + offset" convention already
    // used for the seek bar/status display and relative seeks elsewhere in
    // this file) -- but title-global time still needs the run's own true
    // start subtracted back out to land in the Track's coordinate system.
    // For every path that ISN'T a mid-cell time-map seek,
    // m_dvdPositionOffsetMs already equals runBaseMs, so this reduces to
    // plain m_player->position() (today's unchanged behavior); a mid-cell
    // seek is the one case where they differ, and that mismatch was
    // silently discarding every subtitle lookup after such a seek (2026-08-15
    // bug report: subtitles stopped appearing at all once real seeking was
    // exercised, only working via uninterrupted playback from the very
    // start of a run).
    qint64 runBaseMs = 0;
    if (title && m_dvdRunStartCell >= 0
        && m_dvdRunStartCell < title->cellStartsMs.size())
        runBaseMs = title->cellStartsMs.at(m_dvdRunStartCell);
    const qint64 pos = m_player->position() + m_dvdPositionOffsetMs - runBaseMs;
    const int prevIndex = m_dvdSubtitleTrack.lastResolvedIndex();
    const DvdMenu::Subpicture sp = m_dvdSubtitleTrack.activeAt(pos);
    const int newIndex = m_dvdSubtitleTrack.lastResolvedIndex();
    // TEMP DIAGNOSTIC (2026-08-15, tracking down "only the first subtitle
    // line ever displays" report): log every time the resolved event index
    // changes, so we can see whether activeAt() ever advances past event 0
    // during a real continuous playthrough, and if it does, whether decode/
    // render is what silently fails from then on.
    if (newIndex != prevIndex) {
        dvdLog(QStringLiteral("dvd subtitle: index %1 -> %2 at pos=%3 "
                              "spValid=%4 spSize=%5x%6")
                       .arg(prevIndex).arg(newIndex).arg(pos)
                       .arg(sp.isValid()).arg(sp.width).arg(sp.height));
    }
    QString url;
    if (sp.isValid() && title) {
        QImage img = DvdMenu::renderHighlight(sp, title->palette,
                                              QRect(), 0);
        // Only for real subtitles, not the SEPARATE dvdMenuHighlightUrl()
        // call site (renderHighlight is shared by both, but a blurred
        // button highlight would look less crisp for an interactive
        // element, and the menu highlight was never the subject of the
        // "jaggy glyph" report). User-configurable (0 = off) per the
        // 2026-08-16 follow-up request -- not everyone wants this.
        img = softenSubtitleEdges(img, m_dvdSubtitleSmoothing);
        if (!img.isNull()) {
            QByteArray png;
            QBuffer buf(&png);
            buf.open(QIODevice::WriteOnly);
            img.save(&buf, "PNG");
            url = QStringLiteral("data:image/png;base64,")
                    + QString::fromLatin1(png.toBase64());
        } else if (newIndex != prevIndex) {
            dvdLog(QStringLiteral("dvd subtitle: index %1 decoded but "
                                  "renderHighlight returned a null image "
                                  "(all-transparent?)").arg(newIndex));
        }
    }
    if (url != m_dvdSubtitleImageUrl) {
        m_dvdSubtitleImageUrl = url;
        emit dvdSubtitleImageChanged();
    }
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
    // Clear before running ANYTHING this click triggers (the button's own
    // command below, and whatever performNavAction()/runPgcPostCommands()
    // recursively runs afterward) -- see m_dvdAudioChosenByUser's own doc
    // comment for why a real button click, not an automatic disc-internal
    // command chain, is what should mark these as a genuine user choice.
    // A before/after VALUE comparison was tried first and doesn't work: a
    // disc's "no subtitle" button can write the very same value (0) an
    // earlier, unrelated command chain already left in sprm[2], so the
    // click's real System-Set instruction still EXECUTES but produces no
    // detectable value change -- touchedSprm1/2 (see their own doc
    // comment) track the execution itself, not the resulting value.
    m_vm.touchedSprm1 = false;
    m_vm.touchedSprm2 = false;
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
        if (m_vm.touchedSprm1)
            m_dvdAudioChosenByUser = true;
        if (m_vm.touchedSprm2)
            m_dvdSubtitleChosenByUser = true;
        return;
    }
    performNavAction(a, vts, m_menuPgc, 0);
    if (m_vm.touchedSprm1)
        m_dvdAudioChosenByUser = true;
    if (m_vm.touchedSprm2)
        m_dvdSubtitleChosenByUser = true;
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

void PlayerController::setDvdSubtitleSmoothing(int radius)
{
    radius = qBound(0, radius, 3);
    if (radius == m_dvdSubtitleSmoothing)
        return;
    m_dvdSubtitleSmoothing = radius;
    // Mirrors the same value into the environment so the CUSTOM PATCHED
    // Qt Multimedia's own bitmap-subtitle decode path
    // (qffmpegstreamdecoder.cpp's vivaceSoftenBitmapSubtitle(), added by
    // patches/qtmultimedia-subtitle-bitmap.patch) can honor the same
    // Preferences setting -- that code renders embedded dvd_subtitle-codec
    // tracks in ordinary media files (as opposed to this class's own
    // DVD-disc-specific rendering, softenSubtitleEdges() below), lives
    // entirely inside qtmultimedia, and has no way to call back into this
    // class or read Settings directly. Read fresh via
    // qEnvironmentVariableIntValue() on every decode there rather than
    // cached, so this takes effect immediately with no restart, exactly
    // like the DVD-disc case below.
    qputenv("VIVACE_SUBTITLE_BITMAP_SMOOTHING", QByteArray::number(radius));
    emit dvdSubtitleSmoothingChanged();
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
    detachCurrentPlaylistFile();
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

void PlayerController::setDeinterlaceMode(int mode)
{
    mode = qBound(0, mode, 2);
    if (mode == m_deinterlaceMode)
        return;
    m_deinterlaceMode = mode;
    // Mirrors the value into the environment so the CUSTOM PATCHED Qt
    // Multimedia's VideoRenderer (patches/qtmultimedia-deinterlace.patch,
    // playbackengine/qffmpegdeinterlacer.cpp) can build/select the right
    // avfilter graph -- that code lives entirely inside qtmultimedia and has
    // no reach into this class or Settings, so an env var is the same relay
    // mechanism already used for VIVACE_SUBTITLE_BITMAP_SMOOTHING above.
    // Read fresh via qEnvironmentVariableIntValue() on every frame there, so
    // this takes effect immediately with no restart.
    qputenv("VIVACE_DEINTERLACE_MODE", QByteArray::number(m_deinterlaceMode));
    emit deinterlaceModeChanged();
    // Confirms via OSD that the change took effect immediately (it does --
    // no reopening the file needed, see the comment above), matching the
    // same OSD-on-change convention as A/V delay, subtitle delay, etc.
    static const char *const kModeNames[] = { QT_TR_NOOP("None"), QT_TR_NOOP("Yadif"),
                                               QT_TR_NOOP("Bwdif") };
    emit osdMessage(tr("Deinterlace: %1").arg(tr(kModeNames[m_deinterlaceMode])));
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
    // Avoid double subtitles: turn off any embedded subtitle track. Via this
    // class's own setActiveSubtitleTrack(), not m_player's directly -- see
    // the comment in restoreTrackSelections() for why (the FFmpeg backend
    // doesn't announce a track *set*, only the list changing, so going
    // straight to m_player would leave the Subtitles > Track menu's "Off"
    // item unchecked after this call).
    if (m_player->activeSubtitleTrack() >= 0)
        setActiveSubtitleTrack(-1);
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
    // QML's RichText TextArea inherits its own color/background for any
    // HTML that doesn't set one explicitly, but bgcolor here is set on every
    // row, so it needs its own light/dark pair -- "lavender"/"powderblue"
    // stayed correct-looking in Light mode, but not in Dark (see
    // MediaInfoDialog.qml's TextArea, whose own color/background switched
    // to palette.text/palette.base -- text there would otherwise go light
    // on these still-light rows).
    const bool dark = QGuiApplication::styleHints()->colorScheme()
                       == Qt::ColorScheme::Dark;
    const auto openItem = [&row, dark]() {
        return QStringLiteral("<tr bgcolor=\"%1\">")
                .arg(dark ? (row++ % 2 ? QStringLiteral("#3a3a4a")
                                       : QStringLiteral("#2a3a42"))
                          : (row++ % 2 ? QStringLiteral("lavender")
                                       : QStringLiteral("powderblue")));
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
    // `languageOverride`, when non-empty, replaces the raw FFmpeg-reported
    // Language metadata for the row at the same index -- used for Blu-ray
    // streams, whose language Qt's FFmpeg backend has no way to know (see
    // blurayAudioTrackLabels()'s own doc comment) but libbluray declares.
    const auto addTrackRows = [&](const QList<QMediaMetaData> &tracks,
                                   const QStringList &languageOverride = {}) {
        QString out;
        for (qsizetype n = 0; n < tracks.size(); ++n) {
            const QMediaMetaData &md = tracks.at(n);
            // Native/translated language name for every source, not just
            // Blu-ray/DVD (whose per-track Qt metadata never carries a
            // language at all -- see languageOverride's own callers): an
            // ordinary file's QMediaMetaData::Language is a QLocale::Language
            // enum value, which stringValue() would otherwise render as the
            // plain English name (QLocale::languageToString) -- shown here as
            // the native name instead, matching the Track menus' own format.
            QString language = (n < languageOverride.size() && !languageOverride.at(n).isEmpty())
                    ? languageOverride.at(n)
                    : nativeLanguageDisplayName(
                              md.value(QMediaMetaData::Language).value<QLocale::Language>());
            out += openItem()
                   + QStringLiteral("<td>%1</td><td>%2</td><td>%3</td></tr>")
                             .arg(n)
                             .arg(language.toHtmlEscaped(),
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
        if (fi.exists()) {
            // Blu-ray/DVD: the "file" is really the disc's own folder (the
            // source URL is a synthetic device-backed hint, not a real
            // playable file), so QFileInfo::size() is a directory-entry
            // size (always 0) -- use the real logical size instead:
            // libbluray's own bd_get_title_size() for BD, or the already-
            // open DvdTitleDevice's own size() for DVD (it already knows
            // the total across whichever cells/VOB files it's serving).
            qint64 sizeBytes = fi.size();
            if (m_blurayDisc && m_blurayCurrentTitleIndex >= 0)
                sizeBytes = m_blurayDisc->currentTitleSizeBytes();
            else if (m_dvdDevice)
                sizeBytes = m_dvdDevice->size();
            s += addItem(tr("Size"), tr("%1 KB (%2 MB)")
                                 .arg(sizeBytes / 1024)
                                 .arg(sizeBytes / 1048576));
        }
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
    clip += addItem(tr("Language"), nativeLanguageDisplayName(
                             md.value(QMediaMetaData::Language).value<QLocale::Language>()));
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
        // Blu-ray: Qt's QMediaMetaData::AudioCodec enum has no entry for
        // most BD-native audio codecs (the DTS family, TrueHD), so it always
        // reports "Unspecified" for them -- use libbluray's own declared
        // coding_type for the ACTIVE audio track instead, when available.
        QString audioFormat = md.stringValue(QMediaMetaData::AudioCodec);
        if (m_blurayDisc && m_blurayCurrentTitleIndex >= 0) {
            const auto &codingTypes =
                    m_blurayDisc->titles().at(m_blurayCurrentTitleIndex).audioCodingTypes;
            const int activeIdx = qMax(0, m_player->activeAudioTrack());
            if (activeIdx < codingTypes.size()) {
                const QString real = blurayAudioCodecName(codingTypes.at(activeIdx));
                if (!real.isEmpty())
                    audioFormat = real;
            }
        }
        s += addItem(tr("Format"), audioFormat);
        const int abr = md.value(QMediaMetaData::AudioBitRate).toInt();
        if (abr > 0)
            s += addItem(tr("Bitrate"), tr("%1 kbps").arg(abr / 1000));
        s += closePar();
    }

    // Audio streams table
    QList<QMediaMetaData> audioTracks = m_player->audioTracks();
    // Blu-ray: drop any track beyond what the disc's own stream table
    // declares -- same reasoning/mechanism as blurayAudioTrackLabels()
    // (Audio > Track menu), so an undeclared stream physically present in
    // the raw mux doesn't show up here either. blurayVisibleAudioTrackCount()
    // returns -1 when not playing a Blu-ray title, leaving these untouched.
    // Also build the SAME native-language names the Track menu shows, since
    // Qt's FFmpeg backend has no way to read language from a BD stream at
    // all (it's libbluray-declared metadata, not in the MPEG-TS mux itself).
    const int blurayAudioCount = blurayVisibleAudioTrackCount();
    QStringList audioLangs;
    if (blurayAudioCount >= 0) {
        if (blurayAudioCount < audioTracks.size())
            audioTracks = audioTracks.first(blurayAudioCount);
        const auto &declared =
                m_blurayDisc->titles().at(m_blurayCurrentTitleIndex).audioLanguages;
        for (int i = 0; i < blurayAudioCount; ++i)
            audioLangs << discLanguageDisplayName(declared.at(i));
    } else if (dvdPlayback()) {
        // DVD: same idea, but DVD's own audio-track model caps to what
        // FFmpeg actually finds (m_dvdAudioStreams can declare MORE streams
        // than a disc actually records) rather than capping the disc's own
        // declared count -- matches dvdAudioTrackLabels()'s own loaded-title
        // branch exactly, so the Track menu and this table always agree.
        for (int i = 0; i < audioTracks.size(); ++i) {
            audioLangs << (i < m_dvdAudioStreams.size()
                                   ? discLanguageDisplayName(m_dvdAudioStreams.at(i).language)
                                   : QString());
        }
    }
    if (!audioTracks.isEmpty()) {
        s += openPar(tr("Audio Streams"));
        s += addColumns({ QStringLiteral("#"), tr("Language"), tr("Name") });
        s += addTrackRows(audioTracks, audioLangs);
        s += closePar();
    }

    // Subtitles table
    QList<QMediaMetaData> subtitleTracks = m_player->subtitleTracks();
    const int blurayVisibleSubtitleCount = blurayVisibleSubtitleTrackCount();
    QStringList subtitleLangs;
    if (blurayVisibleSubtitleCount >= 0) {
        if (blurayVisibleSubtitleCount < subtitleTracks.size())
            subtitleTracks = subtitleTracks.first(blurayVisibleSubtitleCount);
        const auto &declared =
                m_blurayDisc->titles().at(m_blurayCurrentTitleIndex).subtitleLanguages;
        for (int i = 0; i < blurayVisibleSubtitleCount; ++i)
            subtitleLangs << discLanguageDisplayName(declared.at(i));
    } else if (dvdPlayback()) {
        // DVD subtitles are decoded by Vivace's own bitmap decoder, not
        // through Qt's subtitleTracks() at all (see Subtitles > Track's own
        // dvdPlayback branch) -- so this table is normally just empty/hidden
        // for a DVD. Still supply the language override in case a future
        // change (or some disc's own raw MPEG-PS demux) ever populates it.
        for (int i = 0; i < subtitleTracks.size(); ++i) {
            subtitleLangs << (i < m_dvdSubtitleStreams.size()
                                      ? discLanguageDisplayName(m_dvdSubtitleStreams.at(i).language)
                                      : QString());
        }
    }
    if (!subtitleTracks.isEmpty()) {
        s += openPar(tr("Subtitles"));
        s += addColumns({ QStringLiteral("#"), tr("Language"), tr("Name") });
        s += addTrackRows(subtitleTracks, subtitleLangs);
        s += closePar();
    }

    return QStringLiteral("<html><body>%1</body></html>").arg(s);
}

bool PlayerController::savePlaylist(const QUrl &file)
{
    if (!PlaylistParser::save(file, m_playlist->entries()))
        return false;
    if (m_currentPlaylistFile != file) {
        m_currentPlaylistFile = file;
        emit currentPlaylistFileChanged();
    }
    return true;
}

bool PlayerController::addToPlaylistFile(const QUrl &playlistFile,
                                          const QVariantList &newEntries,
                                          bool createNew)
{
    if (!playlistFile.isLocalFile() || newEntries.isEmpty())
        return false;

    QList<PlaylistEntry> entries =
            createNew ? QList<PlaylistEntry>() : PlaylistParser::load(playlistFile);

    // Skip an entry already in the playlist (by real file identity, not just
    // URL string) -- otherwise re-saving a video into a playlist it's
    // already part of (e.g. after overwriting it in the same folder) would
    // add a duplicate row instead of a no-op.
    QList<PlaylistEntry> added;
    added.reserve(newEntries.size());
    for (const QVariant &v : newEntries) {
        const QVariantMap m = v.toMap();
        const QUrl url(m.value(QStringLiteral("fileUrl")).toString());
        const bool alreadyPresent =
                std::any_of(entries.cbegin(), entries.cend(),
                            [&](const PlaylistEntry &e) { return sameLocalFile(e.url, url); })
                || std::any_of(added.cbegin(), added.cend(),
                                [&](const PlaylistEntry &e) { return sameLocalFile(e.url, url); });
        if (!alreadyPresent)
            added.append({ url, m.value(QStringLiteral("title")).toString() });
    }

    if (!added.isEmpty()) {
        entries += added;
        if (!PlaylistParser::save(playlistFile, entries))
            return false;

        // If this file is the one currently shown in the Playlist panel, keep
        // it in sync live instead of requiring the user to reload it by hand.
        if (m_currentPlaylistFile == playlistFile)
            m_playlist->add(added);
    }

    if (m_currentPlaylistFile != playlistFile) {
        m_currentPlaylistFile = playlistFile;
        emit currentPlaylistFileChanged();
    }
    return true;
}

bool PlayerController::addToCurrentPlaylist(const QVariantList &newEntries)
{
    if (newEntries.isEmpty())
        return false;

    // Same de-duplication as addToPlaylistFile(): don't add an entry that's
    // already in the current playlist by real file identity.
    const QList<PlaylistEntry> existing = m_playlist->entries();
    QList<PlaylistEntry> added;
    added.reserve(newEntries.size());
    for (const QVariant &v : newEntries) {
        const QVariantMap m = v.toMap();
        const QUrl url(m.value(QStringLiteral("fileUrl")).toString());
        const bool alreadyPresent =
                std::any_of(existing.cbegin(), existing.cend(),
                            [&](const PlaylistEntry &e) { return sameLocalFile(e.url, url); })
                || std::any_of(added.cbegin(), added.cend(),
                                [&](const PlaylistEntry &e) { return sameLocalFile(e.url, url); });
        if (!alreadyPresent)
            added.append({ url, m.value(QStringLiteral("title")).toString() });
    }
    if (added.isEmpty())
        return true;

    m_playlist->add(added);

    if (!m_currentPlaylistFile.isEmpty())
        PlaylistParser::save(m_currentPlaylistFile, m_playlist->entries());

    return true;
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

void PlayerController::pause()
{
    m_intentionalPause = true;
    m_player->pause();
}

void PlayerController::togglePlayPause()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        pause();
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
    // Release the file handle (not just stop playback) so a stopped file
    // isn't left open -- and therefore un-deletable/un-renameable on Windows
    // -- for the rest of the process's lifetime.
    closeSource();
}

QUrl PlayerController::closeSource()
{
    const QUrl was = m_player->source();
    saveCurrentPosition();
    m_player->stop();
    m_player->setSource(QUrl()); // release the file handle
    // dvdInMenu (and therefore Main.qml's dvdMenuOverlay visibility) derives
    // from m_menuVts, not from the player's own source/state -- without
    // this, stopping playback while a DVD menu was showing left the button-
    // highlight overlay visible over the now-blank video, since nothing
    // else clears DVD menu state on a plain stop/close. A no-op for
    // ordinary (non-DVD) sources, since leaveMenu() itself early-returns
    // when m_menuVts is already < 0.
    leaveMenu();
    return was;
}

// Same effect as closeSource()'s setSource(QUrl()), but queued: called from
// within handleMediaStatus() (itself a mediaStatusChanged handler), so
// clearing the source synchronously here would re-enter Qt's media status
// machinery from inside its own signal emission. A queued call runs on the
// next event-loop turn instead.
void PlayerController::releaseSourceDeferred()
{
    QMetaObject::invokeMethod(
            this, [this] { m_player->setSource(QUrl()); }, Qt::QueuedConnection);
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
    if (m_blurayDevice)
        return blurayChapters();
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
    if (m_blurayDevice) {
        if (!m_blurayDisc)
            return;
        const auto &titles = m_blurayDisc->titles();
        if (m_blurayCurrentTitleIndex < 0 || m_blurayCurrentTitleIndex >= titles.size())
            return;
        const auto &chapters = titles.at(m_blurayCurrentTitleIndex).chapters;
        if (index < 0 || index >= chapters.size())
            return;
        const qint64 target = chapters.at(index).startMs;
        m_player->setPosition(target);
        emit seeked(target);
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
    if (m_blurayDevice) {
        const int current = blurayChapterIndexAt(m_player->position());
        const auto &titles = m_blurayDisc ? m_blurayDisc->titles() : QList<BlurayDisc::Title>();
        const int count = (m_blurayCurrentTitleIndex >= 0 && m_blurayCurrentTitleIndex < titles.size())
                ? titles.at(m_blurayCurrentTitleIndex).chapters.size() : 0;
        if (current + 1 < count)
            playChapter(current + 1);
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
    if (m_blurayDevice) {
        if (!m_blurayDisc)
            return;
        const auto &titles = m_blurayDisc->titles();
        if (m_blurayCurrentTitleIndex < 0 || m_blurayCurrentTitleIndex >= titles.size())
            return;
        const auto &chapters = titles.at(m_blurayCurrentTitleIndex).chapters;
        const qint64 pos = m_player->position();
        const int current = blurayChapterIndexAt(pos);
        if (current < 0) {
            if (!chapters.isEmpty())
                playChapter(0);
            return;
        }
        // More than 3 s into the chapter restarts it; otherwise step back one.
        if (pos - chapters.at(current).startMs > 3000)
            playChapter(current);
        else
            playChapter(qMax(0, current - 1));
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

    pause();
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
            // A same-title rebuild (seek / run-boundary advance / chapter
            // jump within it) must PRESERVE whichever audio track was
            // actually playing a moment ago -- including an explicit
            // Audio > Track pick that has nothing to do with the
            // language-preference default or the disc's own on-screen
            // audio menu -- rather than recomputing a default from
            // scratch on every rebuild (found 2026-08-16: dragging the
            // seek bar was silently resetting the audio track, same root
            // cause as the subtitle case in applyDvdTitle() -- a fresh
            // QMediaPlayer source load does not preserve the previously
            // active track index on its own). See
            // m_dvdPendingAudioTrackRestore's own doc comment.
            if (m_dvdPendingAudioTrackRestore >= 0) {
                setActiveAudioTrack(m_dvdPendingAudioTrackRestore);
                m_dvdPendingAudioTrackRestore = -1;
                return;
            }
            selectPreferredTracks();
            // The disc's own audio menu (e.g. "音声") sets m_vm.sprm[1] (the
            // VM's audio-stream register) via a button's System-Set command
            // -- same mechanism/bug as the subtitle case in applyDvdTitle(),
            // see its own comment. Only meaningful once we're actually
            // playing a TITLE (m_dvdCurrentTitle > 0), not a menu PGC (which
            // also loads via a device source and would reach this same
            // branch) -- menus have no real "audio track" of their own to
            // apply this to. Gated on m_dvdAudioChosenByUser (see its own
            // doc comment): this disc's commands-only "VTS menu entry" PGC
            // also unconditionally sets sprm[1]=0 before ever showing a real
            // menu, so a plain bounds check alone would silently override
            // the language-preference default on EVERY playback, same
            // class of bug as the subtitle case. Overriding AFTER
            // selectPreferredTracks() so a real disc-menu choice wins over
            // the language-preference default. (Only reached for a fresh
            // title -- the same-title case above already returned -- so
            // this can't resurrect a stale sprm[1] over a since-changed
            // explicit Audio > Track pick either.)
            if (m_dvdCurrentTitle > 0 && m_dvdAudioChosenByUser) {
                const DvdIfo::Title *title = currentDvdTitle();
                if (title && m_vm.sprm[1] >= 0
                    && m_vm.sprm[1] < title->audioStreams.size()) {
                    setActiveAudioTrack(m_vm.sprm[1]);
                }
            }
            return;
        }

        // Qt's FFmpeg backend can emit LoadedMedia a second time for the same
        // source well after playback has already started (observed only in
        // installed/deployed builds, not a locally-run one) -- rerunning the
        // one-time setup below, in particular the resume seek, on an
        // already-playing pipeline this early froze playback entirely
        // (position/audio/video all stuck, playbackState still Playing).
        // Only do this setup once per sourceChanged.
        if (m_loadedMediaSetupDone)
            return;
        m_loadedMediaSetupDone = true;

        const QUrl url = m_player->source();
        m_bookmarks->setCurrentKey(url.toString());
        if (m_externalSubs.isEmpty())
            autoloadSubtitlesFor(url);

        QString title = m_player->metaData()
                                .stringValue(QMediaMetaData::Title);
        if (title.isEmpty())
            title = url.fileName();
        m_recents->add(url, title);

        // Track selection (selectPreferredTracks()/restoreTrackSelections(),
        // via setActiveAudioTrack()/setActiveSubtitleTrack()) and the resume
        // seek below all reconfigure the player this early, shortly after
        // load. Doing that while actively playing appears able to wedge the
        // pipeline in some deployed-build environments (position and
        // audio/video all freeze, though playbackState keeps reporting
        // Playing) -- confirmed for the resume seek, and a stored track
        // selection that actually differs from the default is the likely
        // reason some files still freeze even once the seek alone is
        // bracketed. Pause across the whole setup and resume afterward,
        // mirroring the sequence that reliably un-freezes it manually (press
        // Pause, then Play).
        const bool wasPlaying = m_player->playbackState() == QMediaPlayer::PlayingState;
        if (wasPlaying)
            pause();

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

        if (wasPlaying)
            m_player->play();
        return;
    }

    if (status == QMediaPlayer::EndOfMedia) {
        // DVD menu end-of-cell.
        if (m_menuVts >= 0) {
            if (m_menuFreezeAtEnd) {
                // The disc's own per-cell still_time says to freeze right
                // here and wait (see m_menuFreezeAtEnd's doc comment) —
                // don't restart the clip, don't run post-commands. A
                // natural EndOfMedia leaves QMediaPlayer in StoppedState,
                // which clears the video frame to black (confirmed via
                // screenshot) rather than holding the last picture — so
                // explicitly re-seek just before the true end and pause
                // there, which forces a fresh decode/render at that
                // position and keeps a visible frame on screen. The
                // button stays clickable and the QML menu-idle timer
                // (Settings.dvdMenuTimeout) is what eventually moves on if
                // the user does nothing.
                const qint64 dur = m_player->duration();
                if (dur > 0)
                    m_player->setPosition(qMax<qint64>(0, dur - 40));
                m_player->pause();
                return;
            }
            if (m_menuWaitsForInput) {
                // No explicit still-cell, but there's a button and nothing
                // else to fall through to: loop the clip (belt-and-
                // suspenders — setLoops Infinite normally handles it)
                // while waiting for a choice.
                m_player->setPosition(0);
                m_player->play();
                return;
            }
            // Neither case applies: a one-shot intro/transition PGC
            // (regardless of whether it happens to declare a button) --
            // run its post-commands to move on (e.g. the title-menu intro
            // that JumpTT's into the movie).
            runPgcPostCommands(m_menuVts, m_menuPgc, 0);
            return;
        }
        // DVD: continue with the next timeline run of the title.
        if (m_dvdDevice) {
            const DvdIfo::Title *title = currentDvdTitle();
            if (title && m_dvdRunEndCell >= 0
                && m_dvdRunEndCell < title->cells.size()) {
                applyDvdTitle(*title, m_dvdRunEndCell,
                              title->cellStartsMs.at(m_dvdRunEndCell));
            } else if (title && !title->postCommands.isEmpty()
                       && runTitlePostCommands(*title)) {
                // Handled below -- the title's own post-commands chained
                // onward (e.g. into the disc's real menu, or another short
                // mandatory title) instead of just stopping. See
                // runTitlePostCommands()'s own doc comment for why this
                // exists at all: most titles have no post-commands and hit
                // the plain stop path below exactly as before.
            } else {
                releaseSourceDeferred();
                emit playbackFinished();
            }
            return;
        }

        m_fileSettings->remove(m_player->source()); // watched to the end
        const int index = m_autoPlayNext ? pickNextIndex() : -1;
        if (index >= 0) {
            playAt(index);
        } else {
            releaseSourceDeferred();
            emit playbackFinished();
        }
    }
}

void PlayerController::restoreTrackSelections()
{
    if (!m_rememberTrackSelections)
        return;

    const QUrl url = m_player->source();
    const int audio = m_fileSettings->audioTrack(url);
    // setActiveAudioTrack()/setActiveSubtitleTrack() here (unqualified, this
    // class's own wrapper -- see their declaration in playercontroller.h)
    // NOT m_player->setActiveAudioTrack()/setActiveSubtitleTrack() directly:
    // the FFmpeg backend doesn't emit activeTracksChanged when a track is
    // merely *set* (only when the list itself changes), so going straight to
    // m_player leaves PlayerController's own activeAudioTrack/
    // activeSubtitleTrack properties stale -- the restored track plays
    // correctly (m_player's own state is right), but the Track menus'
    // checked items don't move to reflect it, since nothing tells QML to
    // re-read them. This exact bug was already fixed once for direct user
    // track selection (menu clicks); this function bypassed that fix by
    // calling m_player directly instead of going through it.
    if (audio >= 0 && audio < m_player->audioTracks().size())
        setActiveAudioTrack(audio);

    const int subtitle = m_fileSettings->subtitleTrack(url);
    if (subtitle != FileSettings::notStored
        && subtitle >= -1 && subtitle < m_player->subtitleTracks().size()) {
        setActiveSubtitleTrack(subtitle);
    }
}

void PlayerController::selectPreferredTracks()
{
    // See restoreTrackSelections() above for why these call this class's own
    // setActiveAudioTrack()/setActiveSubtitleTrack() rather than m_player's.
    if (!m_preferredAudioLanguages.trimmed().isEmpty()) {
        const int index = findTrackByLanguages(m_player->audioTracks(),
                                               m_preferredAudioLanguages);
        if (index >= 0)
            setActiveAudioTrack(index);
    }

    if (!m_subtitlesByDefault) {
        setActiveSubtitleTrack(-1);
    } else if (!m_preferredSubtitleLanguages.trimmed().isEmpty()) {
        const int index = findTrackByLanguages(m_player->subtitleTracks(),
                                               m_preferredSubtitleLanguages);
        if (index >= 0)
            setActiveSubtitleTrack(index);
    }
}

void PlayerController::detachCurrentPlaylistFile()
{
    if (!m_currentPlaylistFile.isEmpty()) {
        m_currentPlaylistFile = QUrl();
        emit currentPlaylistFileChanged();
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
