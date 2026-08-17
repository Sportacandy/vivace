/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <QAudioOutput>
#include <QElapsedTimer>
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QObject>
#include <QTimer>
#include <QVariant>
#include <QVideoFrame>
#include <QtQml/qqmlregistration.h>

#include <optional>

class QVideoSink;

#include "blurayplayer.h"
#include "bookmarks.h"
#include "chapterparser.h"
#include "dvdifoparser.h"
#include "dvdmenuparser.h"
#include "dvdpciparser.h"
#include "dvdspu.h"
#include "dvdsubtitletrack.h"
#include "dvdvm.h"
#include "favoritesmodel.h"
#include "playlistmodel.h"
#include "recentfiles.h"
#include "screensaver.h"
#include "subtitleparser.h"

class DvdTitleDevice;
class FileSettings;
class HttpTsSource;

/*  Playback coordinator: owns the QMediaPlayer, its audio output and the
    playlist. QML gets the raw player/audioOutput for status, metadata and
    property bindings; transport actions and playlist sequencing go through
    the controller. (Rough equivalent of SMPlayer's Core, minus the process
    management.)
*/
class PlayerController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QMediaPlayer *player READ player CONSTANT)
    Q_PROPERTY(QAudioOutput *audioOutput READ audioOutput CONSTANT)
    Q_PROPERTY(PlaylistModel *playlist READ playlist CONSTANT)
    Q_PROPERTY(RecentFiles *recents READ recents CONSTANT)
    Q_PROPERTY(FavoritesModel *tvChannels READ tvChannels CONSTANT)
    Q_PROPERTY(FavoritesModel *radioChannels READ radioChannels CONSTANT)
    Q_PROPERTY(FavoritesModel *favorites READ favorites CONSTANT)
    Q_PROPERTY(Bookmarks *bookmarks READ bookmarks CONSTANT)
    Q_PROPERTY(qint64 abMarkerA READ abMarkerA NOTIFY abMarkersChanged)
    Q_PROPERTY(qint64 abMarkerB READ abMarkerB NOTIFY abMarkersChanged)
    // Active-track selection, re-exposed with a notifier QML tracks reliably
    // (QMediaPlayer's shared activeTracksChanged isn't seen through the bare
    // QtObject `player` reference).
    Q_PROPERTY(int activeAudioTrack READ activeAudioTrack
               WRITE setActiveAudioTrack NOTIFY activeTracksChanged)
    Q_PROPERTY(int activeVideoTrack READ activeVideoTrack
               WRITE setActiveVideoTrack NOTIFY activeTracksChanged)
    Q_PROPERTY(int activeSubtitleTrack READ activeSubtitleTrack
               WRITE setActiveSubtitleTrack NOTIFY activeTracksChanged)
    Q_PROPERTY(QString currentSubtitleText READ currentSubtitleText
               NOTIFY currentSubtitleTextChanged)
    Q_PROPERTY(bool hasExternalSubtitles READ hasExternalSubtitles
               NOTIFY externalSubtitlesChanged)
    Q_PROPERTY(int subtitleDelay READ subtitleDelay WRITE setSubtitleDelay
               NOTIFY subtitleDelayChanged)
    Q_PROPERTY(bool autoloadSubtitles READ autoloadSubtitles
               WRITE setAutoloadSubtitles NOTIFY autoloadSubtitlesChanged)
    Q_PROPERTY(bool seekPreviewAvailable READ seekPreviewAvailable
                   NOTIFY seekPreviewAvailableChanged)
    // Like player.position, but suppresses the one-frame backward blip the
    // resume guard corrects — bind UI position displays to this to avoid a
    // visible slider flash on the first play-after-pause of a cold-loaded file.
    Q_PROPERTY(qint64 smoothPosition READ smoothPosition NOTIFY smoothPositionChanged)
    // A friendly title for the current media (e.g. a resolved stream's video
    // title); empty for ordinary files, whose title is derived from the URL.
    Q_PROPERTY(QString mediaTitle READ mediaTitle NOTIFY mediaTitleChanged)
    // The .m3u/.m3u8 file the in-memory playlist currently corresponds to,
    // if any -- set when a playlist file is loaded (open()) or saved
    // (savePlaylist()/addToPlaylistFile()); empty once plain media (not a
    // playlist file) is opened, since the in-memory list is then no longer
    // tied to any one file. Lets "Add to an existing playlist" (the YouTube
    // cache Save menu) tell whether the file the user picked is the one
    // currently shown in the Playlist panel, so it can update it live
    // instead of only writing to disk.
    Q_PROPERTY(QUrl currentPlaylistFile READ currentPlaylistFile
               NOTIFY currentPlaylistFileChanged)
    Q_PROPERTY(QObject *videoOutput READ videoOutput WRITE setVideoOutput
               NOTIFY videoOutputChanged)
    // A/V sync offset, in ms. The delay actually applied to the current file is
    // the SUM of the global delay (device-level, common to all files) and the
    // per-file delay. Negative = audio should play earlier than the video,
    // applied by delaying the video (fixes e.g. Bluetooth output latency). The
    // effective (summed) value is clamped to <= 0; positive (delay audio) is
    // not implemented yet.
    Q_PROPERTY(int globalAudioDelay READ globalAudioDelay WRITE setGlobalAudioDelay
               NOTIFY audioDelayChanged)
    Q_PROPERTY(int fileAudioDelay READ fileAudioDelay WRITE setFileAudioDelay
               NOTIFY audioDelayChanged)
    Q_PROPERTY(int effectiveAudioDelay READ effectiveAudioDelay
               NOTIFY audioDelayChanged)
    // View transforms (per-file, reset on each new source, not persisted):
    // aspect 0 = auto/native, else a forced display aspect ratio; zoom 1.0 =
    // normal; pan in device px; rotation 0/90/180/270 CW.
    Q_PROPERTY(qreal videoAspect READ videoAspect WRITE setVideoAspect
               NOTIFY videoTransformChanged)
    Q_PROPERTY(qreal videoZoom READ videoZoom NOTIFY videoTransformChanged)
    Q_PROPERTY(int videoPanX READ videoPanX NOTIFY videoTransformChanged)
    Q_PROPERTY(int videoPanY READ videoPanY NOTIFY videoTransformChanged)
    Q_PROPERTY(int videoRotation READ videoRotation WRITE setVideoRotation
               NOTIFY videoTransformChanged)
    Q_PROPERTY(bool videoFlip READ videoFlip WRITE setVideoFlip
               NOTIFY videoTransformChanged)
    Q_PROPERTY(bool videoMirror READ videoMirror WRITE setVideoMirror
               NOTIFY videoTransformChanged)
    // Deinterlace mode (0 = off, 1 = Yadif, 2 = Bwdif), per-file/session like
    // the view transforms above -- reset from Settings.deinterlaceMode (the
    // user's configurable global default, SMPlayer's Preferences > General >
    // Video "Initial deinterlace") on each genuinely new source, but NOT
    // itself persisted (mirrors SMPlayer's two-level default+override model,
    // Preferences::initial_deinterlace seeding MediaSettings::
    // current_deinterlacer). The setter relays into VIVACE_DEINTERLACE_MODE,
    // an env var read fresh by the patched qtmultimedia FFmpeg plugin's
    // VideoRenderer on every frame -- the plugin has no reach into Settings
    // or any other Qt property, so this is the same env-var-relay pattern
    // already used for the DVD/embedded bitmap-subtitle smoothing patch.
    Q_PROPERTY(int deinterlaceMode READ deinterlaceMode WRITE setDeinterlaceMode
               NOTIFY deinterlaceModeChanged)
    Q_PROPERTY(bool shuffle READ shuffle WRITE setShuffle NOTIFY shuffleChanged)
    Q_PROPERTY(bool repeatAll READ repeatAll WRITE setRepeatAll
               NOTIFY repeatAllChanged)
    Q_PROPERTY(bool resumeEnabled READ resumeEnabled WRITE setResumeEnabled
               NOTIFY resumeEnabledChanged)
    Q_PROPERTY(QString preferredAudioLanguages READ preferredAudioLanguages
               WRITE setPreferredAudioLanguages NOTIFY preferredAudioLanguagesChanged)
    Q_PROPERTY(QString preferredSubtitleLanguages READ preferredSubtitleLanguages
               WRITE setPreferredSubtitleLanguages NOTIFY preferredSubtitleLanguagesChanged)
    Q_PROPERTY(bool subtitlesByDefault READ subtitlesByDefault
               WRITE setSubtitlesByDefault NOTIFY subtitlesByDefaultChanged)
    // Mirrors Settings.dvdSubtitleSmoothing (0 = off, 1-3 = increasing blur
    // radius) -- one-way bound from QML like the other DVD-subtitle
    // preferences above, and read directly by updateDvdSubtitleImage() on
    // every subtitle image rebuild, so changing it in Preferences takes
    // effect on the very next subtitle event with no rebuild needed.
    Q_PROPERTY(int dvdSubtitleSmoothing READ dvdSubtitleSmoothing
               WRITE setDvdSubtitleSmoothing NOTIFY dvdSubtitleSmoothingChanged)
    Q_PROPERTY(bool sessionPlaylistEnabled READ sessionPlaylistEnabled
               WRITE setSessionPlaylistEnabled NOTIFY sessionPlaylistEnabledChanged)
    Q_PROPERTY(bool autosavePlaylistOnExit READ autosavePlaylistOnExit
               WRITE setAutosavePlaylistOnExit NOTIFY autosavePlaylistOnExitChanged)
    Q_PROPERTY(bool autoAddFolderFiles READ autoAddFolderFiles
               WRITE setAutoAddFolderFiles NOTIFY autoAddFolderFilesChanged)
    Q_PROPERTY(bool rememberTrackSelections READ rememberTrackSelections
               WRITE setRememberTrackSelections NOTIFY rememberTrackSelectionsChanged)
    Q_PROPERTY(bool autoPlayNext READ autoPlayNext WRITE setAutoPlayNext
               NOTIFY autoPlayNextChanged)
    Q_PROPERTY(bool disableScreensaver READ disableScreensaver
               WRITE setDisableScreensaver NOTIFY disableScreensaverChanged)
    Q_PROPERTY(bool playOnLoadPlaylist READ playOnLoadPlaylist
               WRITE setPlayOnLoadPlaylist NOTIFY playOnLoadPlaylistChanged)
    Q_PROPERTY(bool ignorePlaybackErrors READ ignorePlaybackErrors
               WRITE setIgnorePlaybackErrors NOTIFY ignorePlaybackErrorsChanged)
    Q_PROPERTY(int mediaToAdd READ mediaToAdd WRITE setMediaToAdd
               NOTIFY mediaToAddChanged)
    // Network I/O timeout (seconds) for the FFmpeg backend — long enough for
    // live TV tuners that stall while the channel locks.
    Q_PROPERTY(int networkTimeout READ networkTimeout WRITE setNetworkTimeout
               NOTIFY networkTimeoutChanged)
    Q_PROPERTY(bool playFilesFromStart READ playFilesFromStart
               WRITE setPlayFilesFromStart NOTIFY playFilesFromStartChanged)
    Q_PROPERTY(QString mediaInfoHtml READ mediaInfoHtml NOTIFY mediaInfoChanged)
    Q_PROPERTY(int currentFileFormat READ currentFileFormat NOTIFY mediaInfoChanged)
    Q_PROPERTY(int currentVideoCodec READ currentVideoCodec NOTIFY mediaInfoChanged)
    Q_PROPERTY(int currentAudioCodec READ currentAudioCodec NOTIFY mediaInfoChanged)
    // Basic/primitive Blu-ray Disc playback (see blurayplayer.h): title
    // enumeration + playback via libbluray, no on-disc HDMV/BD-J menu
    // navigation (unlike the DVD support's menu-lite). Chapters reuse the
    // unified `chapters` property below, same as DVD.
    Q_PROPERTY(bool blurayPlayback READ blurayPlayback NOTIFY blurayPlaybackChanged)
    Q_PROPERTY(QVariantList blurayTitles READ blurayTitles NOTIFY blurayPlaybackChanged)
    Q_PROPERTY(int blurayCurrentTitle READ blurayCurrentTitle NOTIFY blurayPlaybackChanged)
    Q_PROPERTY(bool dvdPlayback READ dvdPlayback NOTIFY dvdPlaybackChanged)
    Q_PROPERTY(QVariantList dvdTitles READ dvdTitles NOTIFY dvdPlaybackChanged)
    Q_PROPERTY(int dvdCurrentTitle READ dvdCurrentTitle NOTIFY dvdPlaybackChanged)
    Q_PROPERTY(QVariantList dvdChapters READ dvdChapters NOTIFY dvdPlaybackChanged)
    // Unified chapter list (DVD chapters when playing a disc, else the current
    // file's parsed chapters): rows of { label, startMs }.
    Q_PROPERTY(QVariantList chapters READ chapters NOTIFY chaptersChanged)
    Q_PROPERTY(qint64 dvdTitleDurationMs READ dvdTitleDurationMs
               NOTIFY dvdPlaybackChanged)
    Q_PROPERTY(qint64 dvdPositionOffsetMs READ dvdPositionOffsetMs
               NOTIFY dvdPlaybackChanged)
    // Experimental DVD menu ("menu-lite"): whether a menu is currently shown,
    // its clickable buttons, the highlighted button, and the coordinate space
    // the button rectangles live in (the menu video's resolution).
    Q_PROPERTY(bool dvdInMenu READ dvdInMenu NOTIFY dvdMenuChanged)
    Q_PROPERTY(bool dvdHasMenu READ dvdHasMenu NOTIFY dvdPlaybackChanged)
    Q_PROPERTY(bool dvdMenusEnabled READ dvdMenusEnabled WRITE setDvdMenusEnabled
               NOTIFY dvdMenusEnabledChanged)
    Q_PROPERTY(bool dvdUseFirstPlay READ dvdUseFirstPlay WRITE setDvdUseFirstPlay
               NOTIFY dvdUseFirstPlayChanged)
    Q_PROPERTY(QVariantList dvdMenuButtons READ dvdMenuButtons NOTIFY dvdMenuChanged)
    Q_PROPERTY(int dvdMenuSelected READ dvdMenuSelected NOTIFY dvdMenuChanged)
    Q_PROPERTY(int dvdMenuSpaceWidth READ dvdMenuSpaceWidth NOTIFY dvdMenuChanged)
    Q_PROPERTY(int dvdMenuSpaceHeight READ dvdMenuSpaceHeight NOTIFY dvdMenuChanged)
    // The disc's own subpicture highlight for the current selection, as a PNG
    // data URL (empty when the menu has no subpicture); "" -> QML draws its own.
    Q_PROPERTY(QString dvdMenuHighlightUrl READ dvdMenuHighlightUrl
               NOTIFY dvdMenuChanged)
    // Real movie subtitles, demuxed/decoded directly from the IFO-declared
    // subpicture stream table instead of relying on FFmpeg's own (unreliable
    // for DVD subpicture) track detection -- see dvdsubtitletrack.h. Labels
    // rebuild whenever the current title changes; -1 = off.
    Q_PROPERTY(QStringList dvdSubtitleTrackLabels READ dvdSubtitleTrackLabels
               NOTIFY dvdPlaybackChanged)
    Q_PROPERTY(int activeDvdSubtitleTrack READ activeDvdSubtitleTrack
               WRITE setActiveDvdSubtitleTrack NOTIFY activeDvdSubtitleTrackChanged)
    // The decoded subtitle bitmap active at the current playback position, as
    // a PNG data URL (same technique as dvdMenuHighlightUrl); empty when no
    // track is selected or nothing is showing right now.
    Q_PROPERTY(QString dvdSubtitleImageUrl READ dvdSubtitleImageUrl
               NOTIFY dvdSubtitleImageChanged)
    Q_PROPERTY(bool dvdSubtitleLoading READ dvdSubtitleLoading
               NOTIFY dvdSubtitleLoadingChanged)
    Q_PROPERTY(QVariantList audioDevices READ audioDevices NOTIFY audioDevicesChanged)
    Q_PROPERTY(QString audioDeviceId READ audioDeviceId WRITE setAudioDeviceId
               NOTIFY audioDeviceIdChanged)
    // The audio output device actually in use (may be the system default), used
    // to key the per-device audio delay.
    Q_PROPERTY(QString currentAudioDeviceId READ currentAudioDeviceId
               NOTIFY currentAudioDeviceChanged)
    Q_PROPERTY(QString currentAudioDeviceDescription READ currentAudioDeviceDescription
               NOTIFY currentAudioDeviceChanged)
    Q_PROPERTY(QStringList videoTrackLabels READ videoTrackLabels
               NOTIFY trackLabelsChanged)
    Q_PROPERTY(QStringList audioTrackLabels READ audioTrackLabels
               NOTIFY trackLabelsChanged)
    // Audio > Track labelled with the DVD's own IFO-declared language per
    // real, selectable FFmpeg audio track (same idea as dvdSubtitleTrackLabels,
    // but positional against m_player->audioTracks() rather than a separate
    // Vivace-side decoder -- audio selection still goes through the ordinary
    // FFmpeg track mechanism unchanged, only the label text differs from
    // plain audioTrackLabels()). Falls back to "Track N" per entry that has
    // no corresponding declared stream or language.
    Q_PROPERTY(QStringList dvdAudioTrackLabels READ dvdAudioTrackLabels
               NOTIFY trackLabelsChanged)
    // Same idea as dvdAudioTrackLabels()/dvdSubtitleTrackLabels(), but
    // sourced from libbluray's own declared per-stream language
    // (BlurayDisc::Title::audioLanguages/subtitleLanguages) instead of DVD's
    // IFO tables -- Qt Multimedia's generic track metadata carries no
    // language for these BD-sourced MPEG-TS streams (confirmed empirically:
    // QMediaMetaData::Language was empty for every track on a real disc).
    // Selection itself is unchanged, still the plain activeAudioTrack/
    // activeSubtitleTrack FFmpeg track index.
    Q_PROPERTY(QStringList blurayAudioTrackLabels READ blurayAudioTrackLabels
               NOTIFY trackLabelsChanged)
    Q_PROPERTY(QStringList bluraySubtitleTrackLabels READ bluraySubtitleTrackLabels
               NOTIFY trackLabelsChanged)
    Q_PROPERTY(QStringList subtitleTrackLabels READ subtitleTrackLabels
               NOTIFY trackLabelsChanged)

public:
    explicit PlayerController(QObject *parent = nullptr);
    ~PlayerController() override;

    QMediaPlayer *player() const { return m_player; }
    QAudioOutput *audioOutput() const { return m_audioOutput; }
    PlaylistModel *playlist() const { return m_playlist; }
    RecentFiles *recents() const { return m_recents; }
    FavoritesModel *tvChannels() const { return m_tvChannels; }
    FavoritesModel *radioChannels() const { return m_radioChannels; }
    FavoritesModel *favorites() const { return m_favorites; }
    Bookmarks *bookmarks() const { return m_bookmarks; }

    // Adds what is currently playing (file, stream or DVD folder) to the
    // given list (favorites, TV or radio).
    Q_INVOKABLE void addCurrentTo(FavoritesModel *list);

    // Bookmarks the current playback position (title-global time for DVDs).
    Q_INVOKABLE void addBookmark(const QString &name = QString());
    // Seeks to a bookmarked time (title-global for DVDs).
    Q_INVOKABLE void goToBookmark(qint64 timeMs);

    // A-B repeat: loop playback between marker A and B (ms; -1 = unset).
    qint64 abMarkerA() const { return m_abMarkerA; }
    qint64 abMarkerB() const { return m_abMarkerB; }
    Q_INVOKABLE void setAMarker();       // A = current position
    Q_INVOKABLE void setBMarker();       // B = current position
    Q_INVOKABLE void clearABMarkers();

    // External subtitles (SRT/VTT/basic ASS), rendered by the QML overlay.
    QString currentSubtitleText() const { return m_currentSubtitleText; }
    bool hasExternalSubtitles() const { return !m_externalSubs.isEmpty(); }
    int subtitleDelay() const { return m_subtitleDelayMs; }
    void setSubtitleDelay(int ms);
    bool autoloadSubtitles() const { return m_autoloadSubtitles; }
    void setAutoloadSubtitles(bool autoload);
    Q_INVOKABLE bool loadSubtitles(const QUrl &url);
    Q_INVOKABLE void unloadSubtitles();
    Q_INVOKABLE void adjustSubtitleDelay(int deltaMs);

    QObject *videoOutput() const;
    void setVideoOutput(QObject *videoOutput);

    // Seek-preview thumbnails: a hidden second player grabs the frame at a
    // hovered time and relays it to a small VideoOutput in the seek-bar popup.
    // File playback only.
    bool seekPreviewAvailable() const { return m_seekPreviewAvailable; }
    qint64 smoothPosition() const { return m_smoothPosition; }
    QString mediaTitle() const { return m_mediaTitle; }
    QUrl currentPlaylistFile() const { return m_currentPlaylistFile; }
    Q_INVOKABLE void setPreviewVideoOutput(QObject *item);
    Q_INVOKABLE void requestSeekPreview(qint64 ms);

    int globalAudioDelay() const { return m_globalAudioDelay; }
    void setGlobalAudioDelay(int ms);
    int fileAudioDelay() const { return m_fileAudioDelay; }
    void setFileAudioDelay(int ms);
    int effectiveAudioDelay() const;
    Q_INVOKABLE void adjustFileAudioDelay(int deltaMs);

    // View transforms (SMPlayer's aspect ratio / zoom-pan / rotate / flip /
    // mirror). Reset per file; menu-driven, not persisted.
    qreal videoAspect() const { return m_videoAspect; }
    void setVideoAspect(qreal aspect);
    qreal videoZoom() const { return m_videoZoom; }
    int videoPanX() const { return m_videoPanX; }
    int videoPanY() const { return m_videoPanY; }
    int videoRotation() const { return m_videoRotation; }
    void setVideoRotation(int degrees);
    bool videoFlip() const { return m_videoFlip; }
    void setVideoFlip(bool flip);
    bool videoMirror() const { return m_videoMirror; }
    void setVideoMirror(bool mirror);
    int deinterlaceMode() const { return m_deinterlaceMode; }
    void setDeinterlaceMode(int mode);
    Q_INVOKABLE void zoomIn();               // E: enlarge by one step
    Q_INVOKABLE void zoomOut();              // W: shrink by one step
    Q_INVOKABLE void panBy(int dx, int dy);  // Alt+arrows: move the frame
    Q_INVOKABLE void resetZoomAndPan();      // Shift+E: zoom 1.0, pan 0
    void resetVideoTransform();              // full reset (new file)

    bool shuffle() const { return m_shuffle; }
    void setShuffle(bool shuffle);
    bool repeatAll() const { return m_repeatAll; }
    void setRepeatAll(bool repeatAll);
    bool resumeEnabled() const { return m_resumeEnabled; }
    void setResumeEnabled(bool enabled);
    QString preferredAudioLanguages() const { return m_preferredAudioLanguages; }
    void setPreferredAudioLanguages(const QString &languages);
    QString preferredSubtitleLanguages() const { return m_preferredSubtitleLanguages; }
    void setPreferredSubtitleLanguages(const QString &languages);
    bool subtitlesByDefault() const { return m_subtitlesByDefault; }
    void setSubtitlesByDefault(bool enabled);
    int dvdSubtitleSmoothing() const { return m_dvdSubtitleSmoothing; }
    void setDvdSubtitleSmoothing(int radius);
    bool sessionPlaylistEnabled() const { return m_sessionPlaylistEnabled; }
    void setSessionPlaylistEnabled(bool enabled);
    bool autosavePlaylistOnExit() const { return m_autosavePlaylistOnExit; }
    void setAutosavePlaylistOnExit(bool autosave);
    bool autoAddFolderFiles() const { return m_autoAddFolderFiles; }
    void setAutoAddFolderFiles(bool autoAdd);
    bool rememberTrackSelections() const { return m_rememberTrackSelections; }
    void setRememberTrackSelections(bool remember);
    bool autoPlayNext() const { return m_autoPlayNext; }
    void setAutoPlayNext(bool autoPlay);
    bool disableScreensaver() const { return m_disableScreensaver; }
    void setDisableScreensaver(bool disable);
    bool playOnLoadPlaylist() const { return m_playOnLoadPlaylist; }
    void setPlayOnLoadPlaylist(bool play);
    bool ignorePlaybackErrors() const { return m_ignorePlaybackErrors; }
    void setIgnorePlaybackErrors(bool ignore);
    int mediaToAdd() const { return m_mediaToAdd; }
    void setMediaToAdd(int mode);
    int networkTimeout() const { return m_networkTimeout; } // seconds
    void setNetworkTimeout(int seconds);
    bool playFilesFromStart() const { return m_playFilesFromStart; }
    void setPlayFilesFromStart(bool fromStart);

    Q_INVOKABLE void clearFileSettings();

    int currentFileFormat() const;
    int currentVideoCodec() const;
    int currentAudioCodec() const;

    QVariantList audioDevices() const;
    QString audioDeviceId() const { return m_audioDeviceId; }
    QString currentAudioDeviceId() const;
    QString currentAudioDeviceDescription() const;
    void setAudioDeviceId(const QString &deviceId);

    QString mediaInfoHtml() const;

    Q_INVOKABLE void restoreSessionPlaylist();

    // Saves the current video frame; returns the file path, or "" on failure.
    Q_INVOKABLE QString takeScreenshot(const QString &folder,
                                       const QString &format);

    // Replaces the playlist with all media files found in the directory.
    Q_INVOKABLE void openDirectory(const QUrl &directory, bool recursive);

    // Plays the main title of a DVD-Video folder (or a folder/drive
    // containing VIDEO_TS). Returns false when no title set is found.
    Q_INVOKABLE bool openDvd(const QUrl &folder);

    // Opens a Blu-ray Disc folder (or drive/image root containing BDMV/) and
    // plays its main title. Returns false when no libbluray-openable BD
    // structure is found (including a build without libbluray, or an
    // AACS-encrypted disc libbluray can't decrypt).
    Q_INVOKABLE bool openBluray(const QUrl &folder);
    bool blurayPlayback() const { return m_blurayDisc != nullptr; }
    QVariantList blurayTitles() const;
    int blurayCurrentTitle() const { return m_blurayCurrentTitleIndex; }
    // Plays titleListIndex (an index into blurayTitles(), not a raw
    // libbluray title number).
    Q_INVOKABLE void playBlurayTitle(int titleListIndex);

    bool dvdPlayback() const { return m_dvdDevice != nullptr; }
    QVariantList dvdTitles() const;
    int dvdCurrentTitle() const { return m_dvdCurrentTitle; }
    QVariantList dvdChapters() const;
    qint64 dvdTitleDurationMs() const;
    qint64 dvdPositionOffsetMs() const { return m_dvdPositionOffsetMs; }

    Q_INVOKABLE void playDvdTitle(int titleNumber);
    Q_INVOKABLE void playDvdChapter(int chapterIndex);
    Q_INVOKABLE void previousDvdChapter();
    Q_INVOKABLE void nextDvdChapter();
    Q_INVOKABLE void seekDvd(qint64 titleMs);

    // DVD menu (experimental).
    bool dvdMenusEnabled() const { return m_dvdMenusEnabled; }
    void setDvdMenusEnabled(bool enabled)
    {
        if (enabled == m_dvdMenusEnabled)
            return;
        m_dvdMenusEnabled = enabled;
        emit dvdMenusEnabledChanged();
    }
    bool dvdUseFirstPlay() const { return m_dvdUseFirstPlay; }
    void setDvdUseFirstPlay(bool enabled)
    {
        if (enabled == m_dvdUseFirstPlay)
            return;
        m_dvdUseFirstPlay = enabled;
        emit dvdUseFirstPlayChanged();
    }
    bool dvdInMenu() const { return m_menuVts >= 0; }
    bool dvdHasMenu() const { return m_dvdDevice && m_menus.hasMenus(); }
    QVariantList dvdMenuButtons() const;
    int dvdMenuSelected() const { return m_menuSelected; }
    int dvdMenuSpaceWidth() const { return m_menuSpaceW; }
    int dvdMenuSpaceHeight() const { return m_menuSpaceH; }
    QString dvdMenuHighlightUrl() const;
    QStringList dvdSubtitleTrackLabels() const;
    int activeDvdSubtitleTrack() const { return m_dvdActiveSubtitleTrack; }
    void setActiveDvdSubtitleTrack(int index);
    QString dvdSubtitleImageUrl() const { return m_dvdSubtitleImageUrl; }
    bool dvdSubtitleLoading() const { return m_dvdSubtitleBuildInFlight; }
    Q_INVOKABLE void dvdMenuActivate(int buttonNumber);  // 1-based
    Q_INVOKABLE void dvdMenuActivateSelected();
    Q_INVOKABLE void dvdMenuMove(const QString &direction); // up/down/left/right
    Q_INVOKABLE void dvdMenuHover(int buttonNumber); // set highlight (no activate)
    Q_INVOKABLE void showDvdMenu(); // return to the disc's main/root menu
    // Leave the menu and play the disc's main (longest) title from the start —
    // used by the menu idle-timeout and as a "just play it" escape hatch.
    Q_INVOKABLE bool dvdPlayMainTitle();

    // Chapters of the current media (DVD or parsed file). playChapter/next/
    // previous dispatch to the DVD path or a plain seek as appropriate.
    QVariantList chapters() const;
    Q_INVOKABLE void playChapter(int index);
    Q_INVOKABLE void nextChapter();
    Q_INVOKABLE void previousChapter();

    QStringList videoTrackLabels() const;
    QStringList audioTrackLabels() const;
    QStringList dvdAudioTrackLabels() const;
    QStringList blurayAudioTrackLabels() const;
    QStringList bluraySubtitleTrackLabels() const;
    QStringList subtitleTrackLabels() const;

    // QMediaPlayer's FFmpeg backend does not emit activeTracksChanged when the
    // active track is *set* (only when the track list changes), so the setters
    // emit it here to keep QML bindings (the track menus) in sync.
    int activeAudioTrack() const { return m_player->activeAudioTrack(); }
    void setActiveAudioTrack(int index)
    {
        m_player->setActiveAudioTrack(index);
        emit activeTracksChanged();
    }
    int activeVideoTrack() const { return m_player->activeVideoTrack(); }
    void setActiveVideoTrack(int index)
    {
        m_player->setActiveVideoTrack(index);
        emit activeTracksChanged();
    }
    int activeSubtitleTrack() const { return m_player->activeSubtitleTrack(); }
    void setActiveSubtitleTrack(int index)
    {
        m_player->setActiveSubtitleTrack(index);
        emit activeTracksChanged();
    }

    Q_INVOKABLE void open(const QList<QUrl> &urls);
    // Play a single resolved stream (e.g. a yt-dlp media URL) while showing a
    // friendly title instead of the raw URL.
    Q_INVOKABLE void openStream(const QUrl &mediaUrl, const QString &title);
    Q_INVOKABLE void enqueue(const QList<QUrl> &urls);
    Q_INVOKABLE void playAt(int index);
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void togglePlayPause();
    // Gateway for every legitimate pause request (also used internally by
    // togglePlayPause()/frameStep()); see the playbackStateChanged handler in
    // the .cpp for why this matters -- a pause that didn't come through here
    // is treated as a spurious backend transition and reversed.
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    // Stop AND clear the source, releasing the file handle (so the file can be
    // deleted, e.g. from the YouTube cache browser). Returns the URL that was
    // playing, so the caller can tell whether a file was in use.
    Q_INVOKABLE QUrl closeSource();
    Q_INVOKABLE void seekRelative(qint64 deltaMs);
    Q_INVOKABLE void frameStep(int frames);
    Q_INVOKABLE bool savePlaylist(const QUrl &file);
    // Appends newEntries ({title, fileUrl} maps -- the shape YoutubeResolver::
    // copyOrMoveToFolder() returns per relocated file) to playlistFile and
    // re-saves it; createNew starts from an empty list instead of loading
    // playlistFile's existing content first (overwrite rather than append).
    // If playlistFile equals currentPlaylistFile(), the new entries are also
    // enqueued into the live in-memory playlist. Returns false if the file
    // can't be written.
    Q_INVOKABLE bool addToPlaylistFile(const QUrl &playlistFile,
                                       const QVariantList &newEntries,
                                       bool createNew);
    // Appends newEntries ({title, fileUrl} maps) straight to the live
    // playlist -- no playlist FILE to pick -- and, if the playlist is
    // already backed by a file (currentPlaylistFile() non-empty), re-saves
    // that file so the addition isn't lost on exit. The YouTube cache Save
    // menu's "Add to current playlist" option; the caller is expected to
    // have already relocated the file(s) out of the cache (a limited-size
    // rotation) before calling this, same as the other playlist options.
    Q_INVOKABLE bool addToCurrentPlaylist(const QVariantList &newEntries);

signals:
    void videoOutputChanged();
    void videoTransformChanged();
    void deinterlaceModeChanged();
    void seekPreviewAvailableChanged();
    void smoothPositionChanged();
    void mediaTitleChanged();
    void currentPlaylistFileChanged();
    void audioDelayChanged();
    void trackLabelsChanged();
    void activeTracksChanged();
    void seeked(qint64 positionMs);
    void shuffleChanged();
    void repeatAllChanged();
    void resumeEnabledChanged();
    void preferredAudioLanguagesChanged();
    void preferredSubtitleLanguagesChanged();
    void subtitlesByDefaultChanged();
    void dvdSubtitleSmoothingChanged();
    void sessionPlaylistEnabledChanged();
    void autosavePlaylistOnExitChanged();
    void autoAddFolderFilesChanged();
    void rememberTrackSelectionsChanged();
    void autoPlayNextChanged();
    void disableScreensaverChanged();
    void playOnLoadPlaylistChanged();
    void ignorePlaybackErrorsChanged();
    void mediaToAddChanged();
    void networkTimeoutChanged();
    void playFilesFromStartChanged();
    void audioDevicesChanged();
    void audioDeviceIdChanged();
    void currentAudioDeviceChanged();
    void dvdPlaybackChanged();
    void blurayPlaybackChanged();
    void dvdMenuChanged();
    void dvdMenusEnabledChanged();
    void activeDvdSubtitleTrackChanged();
    void dvdSubtitleImageChanged();
    void dvdSubtitleLoadingChanged();
    void dvdUseFirstPlayChanged();
    void chaptersChanged();
    void mediaInfoChanged();
    void playbackFinished();
    void errorMessage(const QString &message);
    void abMarkersChanged();
    void osdMessage(const QString &message); // brief status for the OSD
    void currentSubtitleTextChanged();
    void externalSubtitlesChanged();
    void subtitleDelayChanged();
    void autoloadSubtitlesChanged();

private:
    void handleMediaStatus(QMediaPlayer::MediaStatus status);
    // Clears the player's source on the next event-loop turn, releasing the
    // file handle without re-entering media-status handling synchronously.
    // Used when playback genuinely has nothing left to do (end of media,
    // no next item) so a finished file isn't left open/undeletable.
    void releaseSourceDeferred();
    // Load playlist item `index` and play; `resume` seeks to the stored
    // position (opening a file) vs starting from the beginning (playlist pick).
    void playIndex(int index, bool resume);
    void applyStreamOptions();
    void startHttpTsStream(const QUrl &url); // buffered MPEG-TS network source
    void teardownHttpTsSource();
    HttpTsSource *m_httpTsSource = nullptr;
    void saveCurrentPosition();
    // Detaches the in-memory playlist from any backing .m3u/.m3u8 file
    // (clears currentPlaylistFile if set). Called whenever the whole
    // playlist is replaced by something not loaded from a playlist file
    // (a folder, a DVD, a single stream, ...) -- see currentPlaylistFile's
    // doc comment for why this must hold.
    void detachCurrentPlaylistFile();
    void selectPreferredTracks();
    void restoreTrackSelections();
    int pickNextIndex(); // advances the shuffle cursor when shuffling
    void fillShuffleOrder(); // (re)builds a random permutation of all indices
    void updateScreensaver(); // inhibit the screensaver while playing video
    void updateSeekPreviewSource(); // (re)point the hidden preview player
    void onPreviewFrameChanged();   // prime / relay preview frames
    void applyVideoDelayRouting();  // (re)wire the player->display video path
    void enqueueDelayedFrame(const QVideoFrame &frame);
    void presentDueFrames();
    void flushVideoDelayQueue();
    void updateSubtitle(qint64 positionMs); // sets the on-screen external cue
    void autoloadSubtitlesFor(const QUrl &mediaUrl);
    void saveSessionPlaylist();
    void applyAudioDevice();
    bool applyDvdTitle(const DvdIfo::Title &title, int fromCellIndex = 0,
                       qint64 positionOffsetMs = 0, qint64 startSector = -1);
    // DVD menu navigation (experimental "menu-lite").
    const DvdMenu::Domain *menuDomain(int vts) const;
    bool menuPgcHasButtons(const DvdMenu::Domain *dom, int pgcNumber) const;
    // Does entering this candidate PGC EVENTUALLY reach a button-bearing
    // menu, by actually simulating its pre-commands (and, if it's a
    // non-interactive auto-advancing PGC, its post-commands too) through a
    // throwaway VM -- rather than only checking whether the entry PGC's
    // OWN cells happen to contain a NAV pack with buttons. Found necessary
    // 2026-08-16 (CARS disc): a real disc's "root menu" entry PGC can be a
    // pure decision-tree redirector (dozens of pre-commands, its own cell
    // buttonless or a placeholder) that only reaches the real, visible,
    // button-bearing menu pages several LinkPgcn/JumpSS hops later --
    // menuPgcHasButtons() alone can never see past that, since it never
    // runs any commands at all. Uses a SCRATCH DvdVm::Machine (not m_vm),
    // so probing has zero effect on real navigation state; once a
    // candidate probes true, the caller re-enters it for real via the
    // ordinary playMenuPgc(..., runPre=true) path, which naturally replays
    // the exact same chain (with real side effects this time) and arrives
    // at the same destination.
    bool probeMenuLeadsToButtons(int vts, int pgcNumber) const;
    bool probeMenuLeadsToButtonsRec(int vts, int pgcNumber, DvdVm::Machine &vm,
                                    int depth) const;
    // Dispatches a probed Action read-only, mirroring performNavAction()'s
    // shape: std::nullopt means "the command block fell through with no
    // link" (caller should check the CURRENT pgc's own cells next);
    // otherwise the boolean is the final, resolved verdict for this whole
    // candidate (true = found a button-bearing menu downstream; false = a
    // dead end, most importantly a jump straight to real title content).
    std::optional<bool> probeFollowAction(const DvdVm::Action &a, int vts,
                                          DvdVm::Machine &vm, int depth) const;
    bool runFirstPlay(); // execute the disc's First-Play PGC via the VM
    bool enterDefaultMenu();
    bool enterMenu(int vts, int menuId, int depth);
    // startCell (0-based, into the PGC's filtered cell list) is where
    // playback/button-scanning begins -- normally 0 (the PGC's own start),
    // but a LinkPgn(N) command jumps straight into the middle of a
    // multi-page PGC at program N's own entry cell (see
    // DvdMenu::Pgc::programEntryCells's own doc comment).
    bool playMenuPgc(int vts, int pgcNumber, bool runPre, int depth,
                     int startCell = 0);
    // currentPgc is the 1-based PGC number whose command block actually
    // produced `action` (0 if none, e.g. the VMGI First-Play block) -- NOT
    // necessarily m_menuPgc, which only reflects the last FULLY ENTERED/
    // displayed menu. A PGC's own pre/post-commands can resolve to a
    // "current PGC"-relative action (LinkPgn/LinkTailPgc/LinkCn) before
    // that PGC is ever fully entered (e.g. while still evaluating another
    // PGC's pre-commands after a redirect), so the action must be resolved
    // against the PGC that was ACTUALLY just evaluated, or it silently
    // resolves against whatever menu happened to be showing before --
    // found 2026-08-15 on a real disc: PGC5's own pre-commands (re-run on
    // every entry) decode to LinkPgn(2), and resolving that against the
    // stale m_menuPgc (still 8, the chapter submenu we'd just left) sent
    // the user right back into the chapter submenu instead of the root menu.
    bool performNavAction(const DvdVm::Action &action, int currentVts,
                         int currentPgc, int depth);
    // Runs `pgcNumber`'s own post-commands and resolves the result (falling
    // back to dvdPlayMainTitle() if there's nothing to chain to) -- shared
    // by the EndOfMedia handler's own "one-shot PGC naturally finished"
    // path and performNavAction()'s LinkTailPgc case (a button whose own
    // command explicitly asks to jump straight to this PGC's tail/end,
    // e.g. a "Play Feature" button on a menu that's otherwise just an
    // intro clip with a skip button -- see the 2026-08-15 fix notes).
    bool runPgcPostCommands(int vts, int pgcNumber, int depth);
    // Runs a plain (non-menu) TITLE's own post-commands through the VM when
    // it naturally finishes playing, and follows the resulting action --
    // e.g. a title that's really just a mandatory warning/logo clip whose
    // post-commands chain onward into the disc's real menu system or the
    // main feature, rather than the title simply ending playback. Returns
    // false (do nothing) when there's no actionable command (None/Nop, or
    // the title has no post-commands at all), leaving the caller's own
    // ordinary "reached the end" handling in place. See DvdIfo::Title::
    // postCommands' own doc comment for the real disc this was needed for.
    bool runTitlePostCommands(const DvdIfo::Title &title);
    bool playGlobalTitle(int titleNumber);
    bool playVtsTitle(int vts, int vtsTitleNumber, int part);
    void leaveMenu(); // clear menu state before playing a title
    int firstVtsWithTitle() const;
    QString dvdDiscName() const; // parent folder of VIDEO_TS (for the title)
    const DvdIfo::Title *currentDvdTitle() const;
    int currentDvdChapterIndex() const;
    // Index of the file chapter containing position ms (-1 if none/before first).
    int fileChapterIndexAt(qint64 ms) const;
    // Blu-ray equivalents of dvdChapters()/currentDvdChapterIndex() — chapters
    // of the currently selected title (blurayplayer.h), mirroring the file-
    // chapters shape ({label, startMs}) rather than DVD's own separate model,
    // since a BD title's chapters need no per-run-rebuild handling (libbluray
    // presents one continuous stream per title, unlike DVD's PS timeline
    // restarts) — plain time-based seeks (m_player->setPosition()) suffice.
    QVariantList blurayChapters() const;
    int blurayChapterIndexAt(qint64 ms) const;
    // Shared by blurayAudioTrackLabels()/bluraySubtitleTrackLabels() (the
    // Audio/Subtitles > Track menus) AND mediaInfoHtml() (the Information
    // dialog's Audio/Subtitles Streams tables) -- how many of Qt's own
    // detected tracks should ever be surfaced anywhere in the UI: the
    // SMALLER of what the disc's own libbluray-declared stream table lists
    // and what Qt's FFmpeg backend actually found multiplexed. See
    // blurayAudioTrackLabels()'s own doc comment for why this can be
    // smaller than Qt's count (an undeclared stream physically present in
    // the raw mux that a real BD player would never expose). Returns -1
    // when not currently playing a Blu-ray title (caller falls back to the
    // plain FFmpeg-only count in that case).
    int blurayVisibleAudioTrackCount() const;
    int blurayVisibleSubtitleTrackCount() const;

    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    PlaylistModel *m_playlist = nullptr;
    RecentFiles *m_recents = nullptr;
    FileSettings *m_fileSettings = nullptr;
    FavoritesModel *m_tvChannels = nullptr;
    FavoritesModel *m_radioChannels = nullptr;
    FavoritesModel *m_favorites = nullptr;
    Bookmarks *m_bookmarks = nullptr;
    qint64 m_abMarkerA = -1;
    qint64 m_abMarkerB = -1;
    qreal m_videoAspect = 0.0; // 0 = auto/native
    qreal m_videoZoom = 1.0;
    int m_videoPanX = 0;
    int m_videoPanY = 0;
    int m_videoRotation = 0; // 0/90/180/270, clockwise
    bool m_videoFlip = false;
    bool m_videoMirror = false;
    int m_deinterlaceMode = 0; // 0 = off, 1 = Yadif, 2 = Bwdif
    // A/V sync: delay the video path by |audioDelay| when audioDelay < 0.
    // Seek-preview: hidden player + its sink; frames relayed to m_previewTarget.
    QMediaPlayer *m_previewPlayer = nullptr;
    QVideoSink *m_previewSink = nullptr;
    QVideoSink *m_previewTargetSink = nullptr;
    bool m_previewPriming = false;
    bool m_previewReady = false;
    bool m_seekPreviewAvailable = false;
    qint64 m_previewPendingMs = -1;
    QString m_mediaTitle;
    QUrl m_currentPlaylistFile;
    QString m_pendingStreamTitle; // title to adopt on the next source change

    // Resume guard: the Qt FFmpeg backend can spuriously rewind to the start on
    // the first play-after-pause of a cold-loaded file (seen on the first run
    // after install). We remember the paused position and undo such a rewind.
    QMediaPlayer::PlaybackState m_lastPlaybackState = QMediaPlayer::StoppedState;
    qint64 m_resumeGuardPos = -1;
    // Set right before the only four call sites that ever legitimately pause
    // the player (pause(), frameStep(), SMTC's Pause button, pause-on-minimize).
    // A transition to PausedState seen without this flag is the backend
    // pausing itself spuriously (a race installed builds seem to hit more
    // than a locally-run dev build) -- auto-resume rather than leave the user
    // staring at a paused video they never asked to pause.
    bool m_intentionalPause = false;
    // Guards the one-time per-file setup in handleMediaStatus()'s LoadedMedia
    // branch (recents, track restore, resume seek) against a redundant second
    // LoadedMedia for the same source; reset in the sourceChanged handler.
    bool m_loadedMediaSetupDone = false;
    // Mirrors player.position but skips the one backward blip the guard undoes,
    // so UI position displays never flash to the start on resume.
    qint64 m_smoothPosition = 0;

    // Set alongside a play() call issued right after setSource()/
    // setSourceDevice() for a freshly opened item; re-checked on the next
    // LoadedMedia so a play() request the backend drops (a race between the
    // call and the backend's async pipeline setup, seen intermittently in
    // Release builds) gets reissued instead of leaving playback paused.
    bool m_pendingAutoPlay = false;

    QObject *m_videoOutputItem = nullptr; // the QML VideoOutput
    int m_globalAudioDelay = 0; // device-level, common to all files
    int m_fileAudioDelay = 0;   // per-file, reset to 0 on each new source
    QVideoSink *m_videoDelaySink = nullptr; // intercepts frames from the player
    QVideoSink *m_targetVideoSink = nullptr; // the VideoOutput's own sink
    struct DelayedFrame { qint64 dueMs; QVideoFrame frame; };
    QList<DelayedFrame> m_videoQueue;
    QElapsedTimer m_videoClock;
    QTimer m_videoDelayTimer;
    QList<ChapterParse::Chapter> m_chapters; // parsed file chapters
    QList<SubtitleCue> m_externalSubs;
    QString m_currentSubtitleText;
    int m_subtitleDelayMs = 0;
    bool m_autoloadSubtitles = true;
    bool m_shuffle = false;
    // Exhaustive shuffle: a random permutation played once per cycle.
    QList<int> m_shuffleOrder;
    int m_shufflePos = -1;
    bool m_repeatAll = false;
    bool m_resumeEnabled = true;
    bool m_resumeOnLoad = false; // seek to stored position on the next load?
    QString m_preferredAudioLanguages;
    QString m_preferredSubtitleLanguages;
    bool m_subtitlesByDefault = true;
    // Sentinel, not the real default (see Settings::m_dvdSubtitleSmoothing
    // for the actual default of 1) -- deliberately a value the Settings
    // Binding in Main.qml can never apply unchanged to, so the very FIRST
    // Binding application at startup always falls through
    // setDvdSubtitleSmoothing()'s "did this actually change" guard and
    // sets the VIVACE_SUBTITLE_BITMAP_SMOOTHING env var at least once, even
    // when the user's persisted preference happens to equal 1.
    int m_dvdSubtitleSmoothing = -1;
    bool m_sessionPlaylistEnabled = false;
    bool m_autosavePlaylistOnExit = false;
    bool m_autoAddFolderFiles = false;
    bool m_rememberTrackSelections = true;
    bool m_autoPlayNext = true;
    bool m_disableScreensaver = true;
    ScreenSaver *m_screenSaver = nullptr;
    bool m_playOnLoadPlaylist = true;
    bool m_ignorePlaybackErrors = false;
    int m_mediaToAdd = 3; // 0 none / 1 video / 2 audio / 3 both / 4 consecutive
    int m_networkTimeout = 60; // seconds; applied via QPlaybackOptions
    bool m_playFilesFromStart = false;
    QString m_audioDeviceId;
    QMediaDevices *m_mediaDevices = nullptr;
    DvdTitleDevice *m_dvdDevice = nullptr;
    QString m_dvdDir;
    // Basic/primitive Blu-ray Disc playback (see blurayplayer.h). m_blurayDisc
    // owns the open BLURAY handle + title/chapter list for as long as any
    // title is being browsed/played (persists across Stop, like m_dvdDevice's
    // own DVD-title-list lifetime — only replaced by a genuinely new open());
    // m_blurayDevice is the QIODevice for whichever title is currently
    // selected, parented to `this` so Qt's normal parent-child cleanup
    // handles its deletion (matching m_dvdDevice's own convention).
    BlurayDisc *m_blurayDisc = nullptr;
    QIODevice *m_blurayDevice = nullptr;
    QString m_blurayDir;
    int m_blurayCurrentTitleIndex = -1;
    QList<DvdIfo::Title> m_dvdTitles;
    int m_dvdCurrentTitle = -1;
    qint64 m_dvdPositionOffsetMs = 0;
    int m_dvdRunStartCell = -1; // first cell of the current timeline run
    int m_dvdRunEndCell = -1; // first cell after the current timeline run
    // DVD menu state (experimental).
    DvdMenu::Structure m_menus;
    DvdVm::Machine m_vm;
    int m_menuVts = -1;   // -1 = not in a menu; 0 = VMGM; >=1 = that VTSM
    int m_menuPgc = 0;    // 1-based PGC in the current menu domain
    DvdMenu::ButtonSet m_menuButtons;
    DvdMenu::Subpicture m_menuSpu;   // decoded menu subpicture (highlight bitmap)
    quint32 m_menuPalette[16] = { 0 }; // PGC highlight palette for the subpicture
    mutable QString m_menuHighlightCache; // cached data-URL for m_menuHighlightSel
    mutable int m_menuHighlightSel = -2;  // selection the cache was built for
    bool m_dvdMenusEnabled = true;  // show disc menus (bound to Settings)
    bool m_dvdUseFirstPlay = false; // run First-Play on open (bound to Settings)
    // Set only when a REAL menu-button interaction (via dvdMenuActivate(),
    // not an automatic precommand/postcommand chain the disc runs on its
    // own) actually EXECUTED a System-Set of sprm[1]/[2]
    // (DvdVm::Machine::touchedSprm1/2 -- execution, not a before/after
    // value comparison; see that field's own doc comment for why a value
    // diff misses a "choose the same value the disc already had" click)
    // -- distinguishes an explicit user audio/subtitle choice from a
    // disc's own internal VTS-menu-domain "entry" PGC (commands-only, no
    // cells, never shown to the user) initializing these same registers
    // to its own built-in defaults on first opening the disc. Found
    // 2026-08-16 on a real disc: such a PGC unconditionally zeroed both
    // sprm[1] and sprm[2] before ever linking into the real, visible root
    // menu -- reading sprm[1]/[2] without this guard would silently
    // override Settings' preferred-subtitle-language default on EVERY
    // playback of this disc, even when the user never touched its
    // audio/subtitle menu at all. Reset alongside m_vm.reset() in
    // openDvd() (a fresh disc-open session starts with no real user
    // choice yet).
    bool m_dvdAudioChosenByUser = false;
    bool m_dvdSubtitleChosenByUser = false;
    bool m_menuHasButtons = false;  // current menu PGC declares any clickable buttons
    // Whether the current menu PGC's LAST cell has a nonzero still_time
    // (DvdIfo::Cell::stillTime, PER-CELL -- a different field from the
    // whole-PGC still_time at PGC + 0xA2, which two real discs proved does
    // NOT distinguish "one-shot logo/intro" from "genuine wait-for-input
    // menu": both had PGC-level still_time == 0). When true, the disc
    // itself says to freeze on this cell's last frame and hold there --
    // Vivace approximates this by simply not auto-advancing at
    // EndOfMedia, leaving the last frame on screen (see the EndOfMedia
    // handler and playMenuPgc()'s setLoops() call).
    bool m_menuFreezeAtEnd = false;
    // Whether the current menu PGC must NOT auto-advance on its own at
    // all -- covers m_menuFreezeAtEnd (see above) plus a fallback for a
    // menu with buttons and no post-commands to fall through to (nothing
    // else to do but wait, even absent an explicit still-cell). Gates
    // both the QML menu-idle-timeout timer and the debug
    // VIVACE_DVD_AUTOSELECT hook.
    bool m_menuWaitsForInput = false;
    // ms (relative to the CURRENT playback run's own start -- see
    // playMenuPgc()'s `startCell` parameter, not necessarily the whole
    // PGC's cell 0) at which its buttons actually become valid -- 0 shows
    // them immediately (a single-cell run, or one that isn't freezing at
    // its end). Only meaningful when
    // m_menuFreezeAtEnd is true: a transition cell (e.g. a brief logo
    // animation) playing before the real, frozen menu cell has its OWN
    // (irrelevant, possibly dummy) button layout, which must not be shown
    // as if it belonged to the content currently on screen. See
    // dvdMenuButtons()/dvdMenuHighlightUrl() and the positionChanged
    // handler that re-emits dvdMenuChanged() once this is crossed.
    qint64 m_menuButtonsRevealMs = 0;
    bool m_menuButtonsRevealed = false; // one-shot latch, see positionChanged
    int m_menuSelected = 0; // 1-based highlighted button (0 = none)
    int m_menuSpaceW = 720; // button coordinate space (menu video resolution)
    int m_menuSpaceH = 480;

    // IFO-declared audio streams for the current title -- used only to give
    // Audio > Track menu entries a real language label (audioTrackLabels()),
    // since the raw MPEG-PS stream FFmpeg demuxes carries no language
    // metadata of its own for DVD titles (unlike the subtitle case below,
    // this doesn't change which/how many tracks are selectable -- that
    // still goes through m_player->audioTracks()/setActiveAudioTrack()
    // untouched, since a DVD's own declared audio-stream count doesn't
    // always match how many FFmpeg actually finds multiplexed in the VOB;
    // see the 2026-08-16 fix notes).
    QList<DvdIfo::AudioStream> m_dvdAudioStreams;
    // Set by applyDvdTitle() when the rebuild it just started is for the
    // SAME title (a seek, run-boundary auto-advance, or chapter jump within
    // it, all of which go through applyDvdTitle() again) rather than a
    // genuinely new title -- handleMediaStatus()'s LoadedMedia handler
    // reads this once, right after the resulting source finishes loading,
    // to decide whether to recompute the preferred-audio-language default
    // (a fresh title) or just restore whatever audio track was actually
    // active a moment ago (same title -- a fresh QMediaPlayer source load
    // does not on its own preserve the previously active track index, and
    // recomputing the language default every time silently overrode an
    // explicit Audio > Track choice on every seek; found 2026-08-16,
    // same underlying cause as m_dvdSubtitleChosenByUser's own preserved-
    // choice handling in applyDvdTitle(), just needed for audio too since
    // that goes through the ordinary selectPreferredTracks() path instead).
    // -1 = nothing to restore (this rebuild IS a fresh title).
    int m_dvdPendingAudioTrackRestore = -1;
    // Real movie subtitles (see dvdsubtitletrack.h) -- declared streams for
    // the current title, which one (if any) is selected, its lazily-built
    // event index, and the live rendered-image state a timer refreshes.
    QList<DvdIfo::SubtitleStream> m_dvdSubtitleStreams;
    int m_dvdActiveSubtitleTrack = -1; // -1 = off
    DvdSubtitle::Track m_dvdSubtitleTrack;
    bool m_dvdSubtitleBuildInFlight = false;
    int m_dvdSubtitleBuildGeneration = 0; // guards a stale async build result
    QString m_dvdSubtitleImageUrl;
    void startDvdSubtitleTrackBuild(int streamIndex);
    void updateDvdSubtitleImage();
};

#endif // PLAYERCONTROLLER_H
