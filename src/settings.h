/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QList>
#include <QObject>
#include <QPair>
#include <QRect>
#include <QSettings>
#include <QString>
#include <QUrl>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

/*  Persistent application settings, exposed to QML as a singleton.
    The UI writes here; the player is driven from these values via
    Binding elements, so everything written survives restarts.
*/
class Settings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
    Q_PROPERTY(qreal playbackRate READ playbackRate WRITE setPlaybackRate
               NOTIFY playbackRateChanged)
    Q_PROPERTY(bool pitchCompensation READ pitchCompensation
               WRITE setPitchCompensation NOTIFY pitchCompensationChanged)
    Q_PROPERTY(QUrl lastOpenFolder READ lastOpenFolder WRITE setLastOpenFolder
               NOTIFY lastOpenFolderChanged)
    Q_PROPERTY(bool playlistShuffle READ playlistShuffle WRITE setPlaylistShuffle
               NOTIFY playlistShuffleChanged)
    Q_PROPERTY(bool playlistRepeat READ playlistRepeat WRITE setPlaylistRepeat
               NOTIFY playlistRepeatChanged)
    Q_PROPERTY(bool playlistAsWindow READ playlistAsWindow WRITE setPlaylistAsWindow
               NOTIFY playlistAsWindowChanged)
    Q_PROPERTY(bool resumePlayback READ resumePlayback WRITE setResumePlayback
               NOTIFY resumePlaybackChanged)
    Q_PROPERTY(bool osdEnabled READ osdEnabled WRITE setOsdEnabled
               NOTIFY osdEnabledChanged)
    Q_PROPERTY(int osdDuration READ osdDuration WRITE setOsdDuration
               NOTIFY osdDurationChanged)
    Q_PROPERTY(int networkTimeout READ networkTimeout WRITE setNetworkTimeout
               NOTIFY networkTimeoutChanged)
    Q_PROPERTY(QString preferredAudioLanguage READ preferredAudioLanguage
               WRITE setPreferredAudioLanguage NOTIFY preferredAudioLanguageChanged)
    Q_PROPERTY(QString preferredSubtitleLanguage READ preferredSubtitleLanguage
               WRITE setPreferredSubtitleLanguage NOTIFY preferredSubtitleLanguageChanged)
    Q_PROPERTY(bool subtitlesByDefault READ subtitlesByDefault
               WRITE setSubtitlesByDefault NOTIFY subtitlesByDefaultChanged)
    Q_PROPERTY(bool closeOnFinish READ closeOnFinish WRITE setCloseOnFinish
               NOTIFY closeOnFinishChanged)
    Q_PROPERTY(bool disableScreensaver READ disableScreensaver
               WRITE setDisableScreensaver NOTIFY disableScreensaverChanged)
    Q_PROPERTY(bool pauseWhenMinimized READ pauseWhenMinimized
               WRITE setPauseWhenMinimized NOTIFY pauseWhenMinimizedChanged)
    Q_PROPERTY(int volumeStep READ volumeStep WRITE setVolumeStep
               NOTIFY volumeStepChanged)
    Q_PROPERTY(int seekShortStep READ seekShortStep WRITE setSeekShortStep
               NOTIFY seekShortStepChanged)
    Q_PROPERTY(int seekMediumStep READ seekMediumStep WRITE setSeekMediumStep
               NOTIFY seekMediumStepChanged)
    Q_PROPERTY(int seekLongStep READ seekLongStep WRITE setSeekLongStep
               NOTIFY seekLongStepChanged)
    Q_PROPERTY(int seekWheelStep READ seekWheelStep WRITE setSeekWheelStep
               NOTIFY seekWheelStepChanged)
    // Per-audio-device global delay store (bumped whenever the map changes, so
    // QML bindings on audioDelayForDevice() can re-evaluate).
    Q_PROPERTY(int audioDeviceDelaysRevision READ audioDeviceDelaysRevision
               NOTIFY audioDeviceDelaysChanged)
    Q_PROPERTY(bool rememberGeometry READ rememberGeometry
               WRITE setRememberGeometry NOTIFY rememberGeometryChanged)
    Q_PROPERTY(QRect windowGeometry READ windowGeometry WRITE setWindowGeometry
               NOTIFY windowGeometryChanged)
    Q_PROPERTY(bool restorePlaylist READ restorePlaylist WRITE setRestorePlaylist
               NOTIFY restorePlaylistChanged)
    Q_PROPERTY(bool playOnLoadPlaylist READ playOnLoadPlaylist
               WRITE setPlayOnLoadPlaylist NOTIFY playOnLoadPlaylistChanged)
    Q_PROPERTY(bool ignorePlaybackErrors READ ignorePlaybackErrors
               WRITE setIgnorePlaybackErrors NOTIFY ignorePlaybackErrorsChanged)
    Q_PROPERTY(int mediaToAdd READ mediaToAdd WRITE setMediaToAdd
               NOTIFY mediaToAddChanged)
    Q_PROPERTY(bool displayTitleName READ displayTitleName
               WRITE setDisplayTitleName NOTIFY displayTitleNameChanged)
    Q_PROPERTY(bool playlistAutoSort READ playlistAutoSort
               WRITE setPlaylistAutoSort NOTIFY playlistAutoSortChanged)
    Q_PROPERTY(bool caseSensitiveSearch READ caseSensitiveSearch
               WRITE setCaseSensitiveSearch NOTIFY caseSensitiveSearchChanged)
    Q_PROPERTY(bool autosavePlaylistOnExit READ autosavePlaylistOnExit
               WRITE setAutosavePlaylistOnExit NOTIFY autosavePlaylistOnExitChanged)
    Q_PROPERTY(QString screenshotFolder READ screenshotFolder
               WRITE setScreenshotFolder NOTIFY screenshotFolderChanged)
    Q_PROPERTY(QString opensubtitlesApiKey READ opensubtitlesApiKey
               WRITE setOpensubtitlesApiKey NOTIFY opensubtitlesChanged)
    Q_PROPERTY(QString opensubtitlesUsername READ opensubtitlesUsername
               WRITE setOpensubtitlesUsername NOTIFY opensubtitlesChanged)
    Q_PROPERTY(QString opensubtitlesPassword READ opensubtitlesPassword
               WRITE setOpensubtitlesPassword NOTIFY opensubtitlesChanged)
    // Network proxy: applied application-wide (QNetworkProxy, covering
    // OpenSubtitles/update-check HTTP requests) and, for the HTTP type only,
    // as http_proxy/https_proxy environment variables — the convention the
    // FFmpeg-backend media network code and the yt-dlp child process both
    // read directly, since neither goes through QNetworkAccessManager. See
    // NetworkProxyController.
    Q_PROPERTY(bool proxyEnabled READ proxyEnabled WRITE setProxyEnabled
               NOTIFY proxyChanged)
    // 0 = HTTP, 1 = SOCKS5 (SOCKS5 only reaches QNetworkAccessManager-based
    // requests; there is no env-var convention for it, so it doesn't affect
    // FFmpeg playback or yt-dlp).
    Q_PROPERTY(int proxyType READ proxyType WRITE setProxyType
               NOTIFY proxyChanged)
    Q_PROPERTY(QString proxyHost READ proxyHost WRITE setProxyHost
               NOTIFY proxyChanged)
    Q_PROPERTY(int proxyPort READ proxyPort WRITE setProxyPort
               NOTIFY proxyChanged)
    Q_PROPERTY(QString proxyUsername READ proxyUsername WRITE setProxyUsername
               NOTIFY proxyChanged)
    Q_PROPERTY(QString proxyPassword READ proxyPassword WRITE setProxyPassword
               NOTIFY proxyChanged)
    Q_PROPERTY(bool youtubeEnabled READ youtubeEnabled WRITE setYoutubeEnabled
               NOTIFY youtubeChanged)
    Q_PROPERTY(QString ytdlPath READ ytdlPath WRITE setYtdlPath
               NOTIFY youtubeChanged)
    Q_PROPERTY(int youtubeQuality READ youtubeQuality WRITE setYoutubeQuality
               NOTIFY youtubeChanged)
    // How to open a YouTube URL: 0 = stream (yt-dlp -> direct URL, no cookies,
    // ~720p), 1 = download & play (yt-dlp downloads an HD file with cookies,
    // played then deleted), 2 = external downloader tool.
    Q_PROPERTY(int youtubeMode READ youtubeMode WRITE setYoutubeMode
               NOTIFY youtubeChanged)
    // Cookies (download mode) and ffmpeg location (yt-dlp needs ffmpeg to merge
    // the separate HD video+audio streams; empty = rely on PATH).
    Q_PROPERTY(QString youtubeCookiesFile READ youtubeCookiesFile
               WRITE setYoutubeCookiesFile NOTIFY youtubeChanged)
    Q_PROPERTY(QString youtubeFfmpegLocation READ youtubeFfmpegLocation
               WRITE setYoutubeFfmpegLocation NOTIFY youtubeChanged)
    // Download-mode cache: downloaded videos are kept here as an LRU cache
    // (a cached video is replayed without re-downloading); cacheSize caps the
    // number of files kept (least-recently-used evicted).
    Q_PROPERTY(QString youtubeCacheDir READ youtubeCacheDir
               WRITE setYoutubeCacheDir NOTIFY youtubeChanged)
    Q_PROPERTY(int youtubeCacheSize READ youtubeCacheSize
               WRITE setYoutubeCacheSize NOTIFY youtubeChanged)
    // Seconds into the downloaded video to grab the cache thumbnail from
    // (avoids black intro frames).
    Q_PROPERTY(int youtubeThumbnailOffset READ youtubeThumbnailOffset
               WRITE setYoutubeThumbnailOffset NOTIFY youtubeChanged)
    // Cache-browser thumbnail size: 0 = small, 1 = medium, 2 = large.
    Q_PROPERTY(int youtubeCacheThumbSize READ youtubeCacheThumbSize
               WRITE setYoutubeCacheThumbSize NOTIFY youtubeChanged)
    // External downloader tool (mode 2): run a user tool that downloads the
    // video to a folder, then play the produced file.
    Q_PROPERTY(QString youtubeDownloaderCommand READ youtubeDownloaderCommand
               WRITE setYoutubeDownloaderCommand NOTIFY youtubeChanged)
    Q_PROPERTY(QString youtubeDownloaderArgs READ youtubeDownloaderArgs
               WRITE setYoutubeDownloaderArgs NOTIFY youtubeChanged)
    Q_PROPERTY(QString youtubeDownloadFolder READ youtubeDownloadFolder
               WRITE setYoutubeDownloadFolder NOTIFY youtubeChanged)
    Q_PROPERTY(bool updateCheckEnabled READ updateCheckEnabled
               WRITE setUpdateCheckEnabled NOTIFY updateSettingsChanged)
    Q_PROPERTY(int updateCheckIntervalDays READ updateCheckIntervalDays
               WRITE setUpdateCheckIntervalDays NOTIFY updateSettingsChanged)
    Q_PROPERTY(QString updateLastCheck READ updateLastCheck
               WRITE setUpdateLastCheck NOTIFY updateSettingsChanged)
    Q_PROPERTY(QString updateLastKnownVersion READ updateLastKnownVersion
               WRITE setUpdateLastKnownVersion NOTIFY updateSettingsChanged)
    Q_PROPERTY(QString screenshotFormat READ screenshotFormat
               WRITE setScreenshotFormat NOTIFY screenshotFormatChanged)
    Q_PROPERTY(int recentsMaxItems READ recentsMaxItems WRITE setRecentsMaxItems
               NOTIFY recentsMaxItemsChanged)
    Q_PROPERTY(int mainwindowResizeMode READ mainwindowResizeMode
               WRITE setMainwindowResizeMode NOTIFY mainwindowResizeModeChanged)
    Q_PROPERTY(bool preventOutsideScreen READ preventOutsideScreen
               WRITE setPreventOutsideScreen NOTIFY preventOutsideScreenChanged)
    Q_PROPERTY(bool centerWindow READ centerWindow WRITE setCenterWindow
               NOTIFY centerWindowChanged)
    Q_PROPERTY(bool hideVideoOnAudio READ hideVideoOnAudio
               WRITE setHideVideoOnAudio NOTIFY hideVideoOnAudioChanged)
    Q_PROPERTY(bool useNativeFileDialog READ useNativeFileDialog
               WRITE setUseNativeFileDialog NOTIFY useNativeFileDialogChanged)
    Q_PROPERTY(bool seekOnDrag READ seekOnDrag WRITE setSeekOnDrag
               NOTIFY seekOnDragChanged)
    Q_PROPERTY(bool singleInstance READ singleInstance WRITE setSingleInstance
               NOTIFY singleInstanceChanged)
    Q_PROPERTY(int urlHistoryMax READ urlHistoryMax WRITE setUrlHistoryMax
               NOTIFY urlHistoryMaxChanged)
    // Play > Cast > Smartphone/tablet: the embedded HTTP server's listening
    // port (see CastServer). Persisted for convenience; the server itself is
    // only ever started explicitly from the Cast dialog.
    Q_PROPERTY(int castPort READ castPort WRITE setCastPort NOTIFY castPortChanged)
    Q_PROPERTY(bool rememberLastDir READ rememberLastDir WRITE setRememberLastDir
               NOTIFY rememberLastDirChanged)
    Q_PROPERTY(QString qtStyle READ qtStyle WRITE setQtStyle NOTIFY qtStyleChanged)
    // UI language: "" = follow the system locale, otherwise a code like "en"
    // or "ja". Applied at startup (restart required), like qtStyle.
    Q_PROPERTY(QString uiLanguage READ uiLanguage WRITE setUiLanguage
               NOTIFY uiLanguageChanged)
    Q_PROPERTY(qreal interfaceScaleFactor READ interfaceScaleFactor
               WRITE setInterfaceScaleFactor NOTIFY interfaceScaleFactorChanged)
    Q_PROPERTY(int wheelFunction READ wheelFunction WRITE setWheelFunction
               NOTIFY wheelFunctionChanged)
    Q_PROPERTY(int doubleClickFunction READ doubleClickFunction
               WRITE setDoubleClickFunction NOTIFY doubleClickFunctionChanged)
    Q_PROPERTY(int middleClickFunction READ middleClickFunction
               WRITE setMiddleClickFunction NOTIFY middleClickFunctionChanged)
    Q_PROPERTY(bool autoAddFolderFiles READ autoAddFolderFiles
               WRITE setAutoAddFolderFiles NOTIFY autoAddFolderFilesChanged)
    Q_PROPERTY(QString audioDevice READ audioDevice WRITE setAudioDevice
               NOTIFY audioDeviceChanged)
    Q_PROPERTY(bool rememberFileSettings READ rememberFileSettings
               WRITE setRememberFileSettings NOTIFY rememberFileSettingsChanged)
    Q_PROPERTY(bool rememberVolume READ rememberVolume WRITE setRememberVolume
               NOTIFY rememberVolumeChanged)
    Q_PROPERTY(int initialVolume READ initialVolume WRITE setInitialVolume
               NOTIFY initialVolumeChanged)
    Q_PROPERTY(int osdFontSize READ osdFontSize WRITE setOsdFontSize
               NOTIFY osdFontSizeChanged)
    Q_PROPERTY(bool hideMouseInFullscreen READ hideMouseInFullscreen
               WRITE setHideMouseInFullscreen NOTIFY hideMouseInFullscreenChanged)
    Q_PROPERTY(int mouseHideDelay READ mouseHideDelay WRITE setMouseHideDelay
               NOTIFY mouseHideDelayChanged)
    // Show the disc's menu (experimental) when opening a DVD, instead of
    // jumping straight into the main title.
    Q_PROPERTY(bool dvdUseMenus READ dvdUseMenus WRITE setDvdUseMenus
               NOTIFY dvdUseMenusChanged)
    // Run the disc's First-Play sequence (studio logos / warnings / intro that
    // the disc shows on insert) instead of going straight to the menu.
    Q_PROPERTY(bool dvdUseFirstPlay READ dvdUseFirstPlay WRITE setDvdUseFirstPlay
               NOTIFY dvdUseFirstPlayChanged)
    // Seconds a DVD menu stays open with no interaction before Vivace returns
    // to playback (the main title). 0 = never (the menu stays until a choice).
    Q_PROPERTY(int dvdMenuTimeout READ dvdMenuTimeout WRITE setDvdMenuTimeout
               NOTIFY dvdMenuTimeoutChanged)
    Q_PROPERTY(int leftClickFunction READ leftClickFunction
               WRITE setLeftClickFunction NOTIFY leftClickFunctionChanged)
    Q_PROPERTY(bool autoPlayNext READ autoPlayNext WRITE setAutoPlayNext
               NOTIFY autoPlayNextChanged)
    Q_PROPERTY(bool addDirectoriesRecursively READ addDirectoriesRecursively
               WRITE setAddDirectoriesRecursively
               NOTIFY addDirectoriesRecursivelyChanged)
    Q_PROPERTY(bool startInFullscreen READ startInFullscreen
               WRITE setStartInFullscreen NOTIFY startInFullscreenChanged)
    Q_PROPERTY(bool subtitlesAutoload READ subtitlesAutoload
               WRITE setSubtitlesAutoload NOTIFY subtitlesAutoloadChanged)
    Q_PROPERTY(QString subFontFamily READ subFontFamily WRITE setSubFontFamily
               NOTIFY subFontFamilyChanged)
    Q_PROPERTY(int subFontSize READ subFontSize WRITE setSubFontSize
               NOTIFY subFontSizeChanged)
    Q_PROPERTY(int subPosition READ subPosition WRITE setSubPosition
               NOTIFY subPositionChanged)
    Q_PROPERTY(bool playFilesFromStart READ playFilesFromStart
               WRITE setPlayFilesFromStart NOTIFY playFilesFromStartChanged)
    Q_PROPERTY(QString uiFontFamily READ uiFontFamily WRITE setUiFontFamily
               NOTIFY uiFontFamilyChanged)
    Q_PROPERTY(int uiFontSize READ uiFontSize WRITE setUiFontSize
               NOTIFY uiFontSizeChanged)
    Q_PROPERTY(QStringList urlHistory READ urlHistory NOTIFY urlHistoryChanged)
    Q_PROPERTY(QString iconSet READ iconSet WRITE setIconSet NOTIFY iconSetChanged)
    Q_PROPERTY(QString gui READ gui WRITE setGui NOTIFY guiChanged)
    // Video equalizer (each -100..100, 0 = neutral).
    Q_PROPERTY(int eqBrightness READ eqBrightness WRITE setEqBrightness
               NOTIFY eqChanged)
    Q_PROPERTY(int eqContrast READ eqContrast WRITE setEqContrast NOTIFY eqChanged)
    Q_PROPERTY(int eqHue READ eqHue WRITE setEqHue NOTIFY eqChanged)
    Q_PROPERTY(int eqSaturation READ eqSaturation WRITE setEqSaturation
               NOTIFY eqChanged)
    Q_PROPERTY(int eqGamma READ eqGamma WRITE setEqGamma NOTIFY eqChanged)
    // Saved "default" equalizer values (Set as default / Reset restores these).
    Q_PROPERTY(int eqDefBrightness READ eqDefBrightness WRITE setEqDefBrightness
               NOTIFY eqDefaultsChanged)
    Q_PROPERTY(int eqDefContrast READ eqDefContrast WRITE setEqDefContrast
               NOTIFY eqDefaultsChanged)
    Q_PROPERTY(int eqDefHue READ eqDefHue WRITE setEqDefHue NOTIFY eqDefaultsChanged)
    Q_PROPERTY(int eqDefSaturation READ eqDefSaturation WRITE setEqDefSaturation
               NOTIFY eqDefaultsChanged)
    Q_PROPERTY(int eqDefGamma READ eqDefGamma WRITE setEqDefGamma
               NOTIFY eqDefaultsChanged)
    Q_PROPERTY(bool toolbarGradient READ toolbarGradient WRITE setToolbarGradient
               NOTIFY toolbarGradientChanged)
    Q_PROPERTY(QStringList mainToolbarItems READ mainToolbarItems
               WRITE setMainToolbarItems NOTIFY mainToolbarItemsChanged)
    Q_PROPERTY(QStringList controlBarItems READ controlBarItems
               WRITE setControlBarItems NOTIFY controlBarItemsChanged)
    Q_PROPERTY(int mainToolbarIconSize READ mainToolbarIconSize
               WRITE setMainToolbarIconSize NOTIFY mainToolbarIconSizeChanged)
    Q_PROPERTY(int controlBarIconSize READ controlBarIconSize
               WRITE setControlBarIconSize NOTIFY controlBarIconSizeChanged)
    Q_PROPERTY(bool showTrayIcon READ showTrayIcon WRITE setShowTrayIcon
               NOTIFY showTrayIconChanged)
    Q_PROPERTY(bool showToolbar READ showToolbar WRITE setShowToolbar
               NOTIFY showToolbarChanged)
    Q_PROPERTY(bool showControlBar READ showControlBar WRITE setShowControlBar
               NOTIFY showControlBarChanged)
    Q_PROPERTY(bool showStatusBar READ showStatusBar WRITE setShowStatusBar
               NOTIFY showStatusBarChanged)
    Q_PROPERTY(bool statusVideoInfo READ statusVideoInfo WRITE setStatusVideoInfo
               NOTIFY statusVideoInfoChanged)
    Q_PROPERTY(bool statusAudioInfo READ statusAudioInfo WRITE setStatusAudioInfo
               NOTIFY statusAudioInfoChanged)
    Q_PROPERTY(bool statusFormatInfo READ statusFormatInfo WRITE setStatusFormatInfo
               NOTIFY statusFormatInfoChanged)
    Q_PROPERTY(bool statusBitrateInfo READ statusBitrateInfo
               WRITE setStatusBitrateInfo NOTIFY statusBitrateInfoChanged)
    Q_PROPERTY(bool statusFrameCounter READ statusFrameCounter
               WRITE setStatusFrameCounter NOTIFY statusFrameCounterChanged)
    Q_PROPERTY(bool timeDisplayRemaining READ timeDisplayRemaining
               WRITE setTimeDisplayRemaining NOTIFY timeDisplayRemainingChanged)
    Q_PROPERTY(bool showMilliseconds READ showMilliseconds WRITE setShowMilliseconds
               NOTIFY showMillisecondsChanged)
    // Touch-friendly UI: enlarges fonts, controls and toolbar icons so the app
    // is usable by finger on a tablet as well as by mouse on the desktop.
    Q_PROPERTY(bool touchMode READ touchMode WRITE setTouchMode
               NOTIFY touchModeChanged)
    // Swipe left/right across the video to seek (works with touch or mouse).
    Q_PROPERTY(bool gestureSeek READ gestureSeek WRITE setGestureSeek
               NOTIFY gestureSeekChanged)

public:
    explicit Settings(QObject *parent = nullptr);

    qreal volume() const { return m_volume; }
    void setVolume(qreal volume);

    bool isMuted() const { return m_muted; }
    void setMuted(bool muted);

    qreal playbackRate() const { return m_playbackRate; }
    void setPlaybackRate(qreal rate);

    bool pitchCompensation() const { return m_pitchCompensation; }
    void setPitchCompensation(bool enabled);

    QUrl lastOpenFolder() const { return m_lastOpenFolder; }
    void setLastOpenFolder(const QUrl &folder);

    bool playlistShuffle() const { return m_playlistShuffle; }
    void setPlaylistShuffle(bool shuffle);

    bool playlistRepeat() const { return m_playlistRepeat; }
    void setPlaylistRepeat(bool repeat);

    bool playlistAsWindow() const { return m_playlistAsWindow; }
    void setPlaylistAsWindow(bool asWindow);

    bool resumePlayback() const { return m_resumePlayback; }
    void setResumePlayback(bool resume);

    bool osdEnabled() const { return m_osdEnabled; }
    void setOsdEnabled(bool enabled);

    int osdDuration() const { return m_osdDuration; }
    void setOsdDuration(int durationMs);

    int networkTimeout() const { return m_networkTimeout; } // seconds
    void setNetworkTimeout(int seconds);

    QString preferredAudioLanguage() const { return m_preferredAudioLanguage; }
    void setPreferredAudioLanguage(const QString &languages);

    QString preferredSubtitleLanguage() const { return m_preferredSubtitleLanguage; }
    void setPreferredSubtitleLanguage(const QString &languages);

    bool subtitlesByDefault() const { return m_subtitlesByDefault; }
    void setSubtitlesByDefault(bool enabled);

    bool closeOnFinish() const { return m_closeOnFinish; }
    void setCloseOnFinish(bool close);

    bool disableScreensaver() const { return m_disableScreensaver; }
    void setDisableScreensaver(bool disable);

    bool pauseWhenMinimized() const { return m_pauseWhenMinimized; }
    void setPauseWhenMinimized(bool pause);

    int volumeStep() const { return m_volumeStep; }
    void setVolumeStep(int stepPercent);

    int seekShortStep() const { return m_seekShortStep; }
    void setSeekShortStep(int seconds);

    int seekMediumStep() const { return m_seekMediumStep; }
    void setSeekMediumStep(int seconds);

    int seekLongStep() const { return m_seekLongStep; }
    void setSeekLongStep(int seconds);

    int seekWheelStep() const { return m_seekWheelStep; }

    // Global A/V delay remembered per audio output device (device latency, e.g.
    // Bluetooth, differs per device). Returns 0 for an unknown device.
    int audioDeviceDelaysRevision() const { return m_audioDeviceDelaysRevision; }
    Q_INVOKABLE int audioDelayForDevice(const QString &deviceId) const;
    Q_INVOKABLE void setAudioDelayForDevice(const QString &deviceId, int ms);
    void setSeekWheelStep(int seconds);

    bool rememberGeometry() const { return m_rememberGeometry; }
    void setRememberGeometry(bool remember);

    QRect windowGeometry() const { return m_windowGeometry; }
    void setWindowGeometry(const QRect &geometry);

    bool restorePlaylist() const { return m_restorePlaylist; }
    void setRestorePlaylist(bool restore);

    bool playOnLoadPlaylist() const { return m_playOnLoadPlaylist; }
    void setPlayOnLoadPlaylist(bool play);

    bool ignorePlaybackErrors() const { return m_ignorePlaybackErrors; }
    void setIgnorePlaybackErrors(bool ignore);

    // 0 = none, 1 = video, 2 = audio, 3 = video+audio, 4 = consecutive
    int mediaToAdd() const { return m_mediaToAdd; }
    void setMediaToAdd(int mode);

    bool displayTitleName() const { return m_displayTitleName; }
    void setDisplayTitleName(bool display);

    bool playlistAutoSort() const { return m_playlistAutoSort; }
    void setPlaylistAutoSort(bool autoSort);

    bool caseSensitiveSearch() const { return m_caseSensitiveSearch; }
    void setCaseSensitiveSearch(bool caseSensitive);

    bool autosavePlaylistOnExit() const { return m_autosavePlaylistOnExit; }
    void setAutosavePlaylistOnExit(bool autosave);

    QString screenshotFolder() const { return m_screenshotFolder; }

    QString opensubtitlesApiKey() const { return m_opensubtitlesApiKey; }
    void setOpensubtitlesApiKey(const QString &key);
    QString opensubtitlesUsername() const { return m_opensubtitlesUsername; }
    void setOpensubtitlesUsername(const QString &user);
    QString opensubtitlesPassword() const { return m_opensubtitlesPassword; }
    void setOpensubtitlesPassword(const QString &pass);
    bool proxyEnabled() const { return m_proxyEnabled; }
    void setProxyEnabled(bool enabled);
    int proxyType() const { return m_proxyType; }
    void setProxyType(int type);
    QString proxyHost() const { return m_proxyHost; }
    void setProxyHost(const QString &host);
    int proxyPort() const { return m_proxyPort; }
    void setProxyPort(int port);
    QString proxyUsername() const { return m_proxyUsername; }
    void setProxyUsername(const QString &user);
    QString proxyPassword() const { return m_proxyPassword; }
    void setProxyPassword(const QString &pass);
    bool youtubeEnabled() const { return m_youtubeEnabled; }
    void setYoutubeEnabled(bool enabled);
    QString ytdlPath() const { return m_ytdlPath; }
    void setYtdlPath(const QString &path);
    int youtubeQuality() const { return m_youtubeQuality; }
    void setYoutubeQuality(int height);
    int youtubeMode() const { return m_youtubeMode; }
    void setYoutubeMode(int mode);
    QString youtubeCookiesFile() const { return m_youtubeCookiesFile; }
    void setYoutubeCookiesFile(const QString &path);
    QString youtubeFfmpegLocation() const { return m_youtubeFfmpegLocation; }
    void setYoutubeFfmpegLocation(const QString &path);
    QString youtubeCacheDir() const { return m_youtubeCacheDir; }
    void setYoutubeCacheDir(const QString &path);
    int youtubeCacheSize() const { return m_youtubeCacheSize; }
    void setYoutubeCacheSize(int maxFiles);
    int youtubeThumbnailOffset() const { return m_youtubeThumbnailOffset; }
    void setYoutubeThumbnailOffset(int seconds);
    int youtubeCacheThumbSize() const { return m_youtubeCacheThumbSize; }
    void setYoutubeCacheThumbSize(int size);
    QString youtubeDownloaderCommand() const { return m_youtubeDownloaderCommand; }
    void setYoutubeDownloaderCommand(const QString &command);
    QString youtubeDownloaderArgs() const { return m_youtubeDownloaderArgs; }
    void setYoutubeDownloaderArgs(const QString &args);
    QString youtubeDownloadFolder() const { return m_youtubeDownloadFolder; }
    void setYoutubeDownloadFolder(const QString &folder);
    void setScreenshotFolder(const QString &folder);

    bool updateCheckEnabled() const { return m_updateCheckEnabled; }
    void setUpdateCheckEnabled(bool enabled);
    int updateCheckIntervalDays() const { return m_updateCheckIntervalDays; }
    void setUpdateCheckIntervalDays(int days);
    QString updateLastCheck() const { return m_updateLastCheck; }
    void setUpdateLastCheck(const QString &isoDate);
    QString updateLastKnownVersion() const { return m_updateLastKnownVersion; }
    void setUpdateLastKnownVersion(const QString &version);

    QString screenshotFormat() const { return m_screenshotFormat; }
    void setScreenshotFormat(const QString &format);

    int recentsMaxItems() const { return m_recentsMaxItems; }
    void setRecentsMaxItems(int maxItems);

    // 0 = never, 1 = whenever needed, 2 = only after loading a new video
    int mainwindowResizeMode() const { return m_mainwindowResizeMode; }
    void setMainwindowResizeMode(int mode);

    bool preventOutsideScreen() const { return m_preventOutsideScreen; }
    void setPreventOutsideScreen(bool prevent);

    bool centerWindow() const { return m_centerWindow; }
    void setCenterWindow(bool center);

    bool hideVideoOnAudio() const { return m_hideVideoOnAudio; }
    void setHideVideoOnAudio(bool hide);

    bool useNativeFileDialog() const { return m_useNativeFileDialog; }
    void setUseNativeFileDialog(bool useNative);

    // true = seek while dragging, false = seek when released
    bool seekOnDrag() const { return m_seekOnDrag; }
    void setSeekOnDrag(bool onDrag);

    bool singleInstance() const { return m_singleInstance; }
    void setSingleInstance(bool single);

    int urlHistoryMax() const { return m_urlHistoryMax; }
    void setUrlHistoryMax(int max);
    int castPort() const { return m_castPort; }
    void setCastPort(int port);

    bool rememberLastDir() const { return m_rememberLastDir; }
    void setRememberLastDir(bool remember);

    QString qtStyle() const { return m_qtStyle; }
    QString uiLanguage() const { return m_uiLanguage; }
    void setUiLanguage(const QString &language);
    void setQtStyle(const QString &style);
    // Qt Quick Controls styles installed in this build (for the selector).
    Q_INVOKABLE QStringList availableQtStyles() const;

    // Snapshot/restore of every writable property, for the Preferences
    // Cancel/Apply buttons: the UI applies instantly, so Cancel reverts by
    // restoring the snapshot taken when the dialog opened (or last Applied).
    Q_INVOKABLE QVariantMap snapshot() const;
    Q_INVOKABLE void restore(const QVariantMap &state);

    // 0 = automatic (let Qt decide); otherwise a manual UI scale factor.
    qreal interfaceScaleFactor() const { return m_interfaceScaleFactor; }
    void setInterfaceScaleFactor(qreal factor);

    // 0 = seek, 1 = volume, 2 = none
    int wheelFunction() const { return m_wheelFunction; }
    void setWheelFunction(int function);

    // 0 = toggle fullscreen, 1 = play/pause, 2 = none
    int doubleClickFunction() const { return m_doubleClickFunction; }
    void setDoubleClickFunction(int function);

    // 0 = mute, 1 = play/pause, 2 = toggle fullscreen, 3 = none
    int middleClickFunction() const { return m_middleClickFunction; }
    void setMiddleClickFunction(int function);

    bool autoAddFolderFiles() const { return m_autoAddFolderFiles; }
    void setAutoAddFolderFiles(bool autoAdd);

    QString audioDevice() const { return m_audioDevice; }
    void setAudioDevice(const QString &deviceId);

    bool rememberFileSettings() const { return m_rememberFileSettings; }
    void setRememberFileSettings(bool remember);

    bool rememberVolume() const { return m_rememberVolume; }
    void setRememberVolume(bool remember);

    int initialVolume() const { return m_initialVolume; }
    void setInitialVolume(int volumePercent);

    int osdFontSize() const { return m_osdFontSize; }
    void setOsdFontSize(int pixelSize);

    bool hideMouseInFullscreen() const { return m_hideMouseInFullscreen; }
    void setHideMouseInFullscreen(bool hide);

    int mouseHideDelay() const { return m_mouseHideDelay; }
    void setMouseHideDelay(int delayMs);
    bool dvdUseMenus() const { return m_dvdUseMenus; }
    void setDvdUseMenus(bool use);
    bool dvdUseFirstPlay() const { return m_dvdUseFirstPlay; }
    void setDvdUseFirstPlay(bool use);
    int dvdMenuTimeout() const { return m_dvdMenuTimeout; } // seconds; 0 = never
    void setDvdMenuTimeout(int seconds);

    // 0 = none, 1 = play/pause, 2 = toggle fullscreen
    int leftClickFunction() const { return m_leftClickFunction; }
    void setLeftClickFunction(int function);

    bool autoPlayNext() const { return m_autoPlayNext; }
    void setAutoPlayNext(bool autoPlay);

    bool addDirectoriesRecursively() const { return m_addDirectoriesRecursively; }
    void setAddDirectoriesRecursively(bool recursive);

    bool startInFullscreen() const { return m_startInFullscreen; }
    void setStartInFullscreen(bool fullscreen);

    // Subtitle options below are persisted now and consumed by the
    // external-subtitle renderer planned for Phase 4.
    bool subtitlesAutoload() const { return m_subtitlesAutoload; }
    void setSubtitlesAutoload(bool autoload);

    QString subFontFamily() const { return m_subFontFamily; }
    void setSubFontFamily(const QString &family);

    int subFontSize() const { return m_subFontSize; }
    void setSubFontSize(int pixelSize);

    int subPosition() const { return m_subPosition; }
    void setSubPosition(int percentFromTop);

    bool playFilesFromStart() const { return m_playFilesFromStart; }
    void setPlayFilesFromStart(bool fromStart);

    // Empty family / 0 size = system default. Applied at startup
    // (QGuiApplication::setFont); a note in the UI asks for a restart.
    QString uiFontFamily() const { return m_uiFontFamily; }
    void setUiFontFamily(const QString &family);

    int uiFontSize() const { return m_uiFontSize; }
    void setUiFontSize(int pointSize);

    QStringList urlHistory() const { return m_urlHistory; }
    Q_INVOKABLE void addUrlToHistory(const QString &url);

    QString iconSet() const { return m_iconSet; }
    void setIconSet(const QString &iconSet);
    QString gui() const { return m_gui; }
    void setGui(const QString &gui);

    int eqBrightness() const { return m_eqBrightness; }
    void setEqBrightness(int v);
    int eqContrast() const { return m_eqContrast; }
    void setEqContrast(int v);
    int eqHue() const { return m_eqHue; }
    void setEqHue(int v);
    int eqSaturation() const { return m_eqSaturation; }
    void setEqSaturation(int v);
    int eqGamma() const { return m_eqGamma; }
    void setEqGamma(int v);

    int eqDefBrightness() const { return m_eqDefBrightness; }
    void setEqDefBrightness(int v);
    int eqDefContrast() const { return m_eqDefContrast; }
    void setEqDefContrast(int v);
    int eqDefHue() const { return m_eqDefHue; }
    void setEqDefHue(int v);
    int eqDefSaturation() const { return m_eqDefSaturation; }
    void setEqDefSaturation(int v);
    int eqDefGamma() const { return m_eqDefGamma; }
    void setEqDefGamma(int v);

    bool toolbarGradient() const { return m_toolbarGradient; }
    void setToolbarGradient(bool enabled);

    // Empty list = use the QML default layout (ToolbarItems.js).
    QStringList mainToolbarItems() const { return m_mainToolbarItems; }
    void setMainToolbarItems(const QStringList &items);

    QStringList controlBarItems() const { return m_controlBarItems; }
    void setControlBarItems(const QStringList &items);

    int mainToolbarIconSize() const { return m_mainToolbarIconSize; }
    void setMainToolbarIconSize(int size);

    int controlBarIconSize() const { return m_controlBarIconSize; }
    void setControlBarIconSize(int size);

    bool showTrayIcon() const { return m_showTrayIcon; }
    void setShowTrayIcon(bool show);

    bool showToolbar() const { return m_showToolbar; }
    void setShowToolbar(bool show);

    bool showControlBar() const { return m_showControlBar; }
    void setShowControlBar(bool show);

    bool showStatusBar() const { return m_showStatusBar; }
    void setShowStatusBar(bool show);

    bool statusVideoInfo() const { return m_statusVideoInfo; }
    void setStatusVideoInfo(bool show);

    bool statusAudioInfo() const { return m_statusAudioInfo; }
    void setStatusAudioInfo(bool show);

    bool statusFormatInfo() const { return m_statusFormatInfo; }
    void setStatusFormatInfo(bool show);

    bool statusBitrateInfo() const { return m_statusBitrateInfo; }
    void setStatusBitrateInfo(bool show);

    bool statusFrameCounter() const { return m_statusFrameCounter; }
    void setStatusFrameCounter(bool show);

    bool timeDisplayRemaining() const { return m_timeDisplayRemaining; }
    void setTimeDisplayRemaining(bool remaining);

    bool showMilliseconds() const { return m_showMilliseconds; }
    void setShowMilliseconds(bool show);

    bool touchMode() const { return m_touchMode; }
    void setTouchMode(bool enabled);

    bool gestureSeek() const { return m_gestureSeek; }
    void setGestureSeek(bool enabled);

signals:
    // Emitted whenever any writable setting changes (wired to every property's
    // notify signal in the constructor). Lets the Preferences dialog tell
    // whether there are unsaved changes.
    void changed();

    void volumeChanged();
    void mutedChanged();
    void playbackRateChanged();
    void pitchCompensationChanged();
    void lastOpenFolderChanged();
    void playlistShuffleChanged();
    void playlistRepeatChanged();
    void playlistAsWindowChanged();
    void resumePlaybackChanged();
    void osdEnabledChanged();
    void osdDurationChanged();
    void networkTimeoutChanged();
    void preferredAudioLanguageChanged();
    void preferredSubtitleLanguageChanged();
    void subtitlesByDefaultChanged();
    void closeOnFinishChanged();
    void disableScreensaverChanged();
    void pauseWhenMinimizedChanged();
    void volumeStepChanged();
    void seekShortStepChanged();
    void seekMediumStepChanged();
    void seekLongStepChanged();
    void seekWheelStepChanged();
    void audioDeviceDelaysChanged();
    void rememberGeometryChanged();
    void windowGeometryChanged();
    void restorePlaylistChanged();
    void playOnLoadPlaylistChanged();
    void ignorePlaybackErrorsChanged();
    void mediaToAddChanged();
    void displayTitleNameChanged();
    void playlistAutoSortChanged();
    void caseSensitiveSearchChanged();
    void autosavePlaylistOnExitChanged();
    void screenshotFolderChanged();
    void opensubtitlesChanged();
    void proxyChanged();
    void youtubeChanged();
    void updateSettingsChanged();
    void screenshotFormatChanged();
    void recentsMaxItemsChanged();
    void mainwindowResizeModeChanged();
    void preventOutsideScreenChanged();
    void centerWindowChanged();
    void hideVideoOnAudioChanged();
    void useNativeFileDialogChanged();
    void seekOnDragChanged();
    void singleInstanceChanged();
    void urlHistoryMaxChanged();
    void castPortChanged();
    void rememberLastDirChanged();
    void qtStyleChanged();
    void uiLanguageChanged();
    void interfaceScaleFactorChanged();
    void wheelFunctionChanged();
    void doubleClickFunctionChanged();
    void middleClickFunctionChanged();
    void autoAddFolderFilesChanged();
    void audioDeviceChanged();
    void rememberFileSettingsChanged();
    void rememberVolumeChanged();
    void initialVolumeChanged();
    void osdFontSizeChanged();
    void hideMouseInFullscreenChanged();
    void mouseHideDelayChanged();
    void dvdUseMenusChanged();
    void dvdUseFirstPlayChanged();
    void dvdMenuTimeoutChanged();
    void leftClickFunctionChanged();
    void autoPlayNextChanged();
    void addDirectoriesRecursivelyChanged();
    void startInFullscreenChanged();
    void subtitlesAutoloadChanged();
    void subFontFamilyChanged();
    void subFontSizeChanged();
    void subPositionChanged();
    void playFilesFromStartChanged();
    void uiFontFamilyChanged();
    void uiFontSizeChanged();
    void urlHistoryChanged();
    void iconSetChanged();
    void guiChanged();
    void eqChanged();
    void eqDefaultsChanged();
    void toolbarGradientChanged();
    void mainToolbarItemsChanged();
    void controlBarItemsChanged();
    void mainToolbarIconSizeChanged();
    void controlBarIconSizeChanged();
    void showTrayIconChanged();
    void showToolbarChanged();
    void showControlBarChanged();
    void showStatusBarChanged();
    void statusVideoInfoChanged();
    void statusAudioInfoChanged();
    void statusFormatInfoChanged();
    void statusBitrateInfoChanged();
    void statusFrameCounterChanged();
    void timeDisplayRemainingChanged();
    void showMillisecondsChanged();
    void touchModeChanged();
    void gestureSeekChanged();

private:
    QSettings m_store;
    qreal m_volume;
    bool m_muted;
    qreal m_playbackRate;
    bool m_pitchCompensation;
    QUrl m_lastOpenFolder;
    bool m_playlistShuffle;
    bool m_playlistRepeat;
    bool m_playlistAsWindow;
    bool m_resumePlayback;
    bool m_osdEnabled;
    int m_osdDuration;
    int m_networkTimeout;
    QString m_preferredAudioLanguage;
    QString m_preferredSubtitleLanguage;
    bool m_subtitlesByDefault;
    bool m_closeOnFinish;
    bool m_disableScreensaver;
    bool m_pauseWhenMinimized;
    int m_volumeStep;
    int m_seekShortStep;
    int m_seekMediumStep;
    int m_seekLongStep;
    int m_seekWheelStep;
    // Most-recently-used first; capped. Each entry: (deviceId, delayMs).
    QList<QPair<QString, int>> m_audioDeviceDelays;
    int m_audioDeviceDelaysRevision = 0;
    bool m_rememberGeometry;
    QRect m_windowGeometry;
    bool m_restorePlaylist;
    bool m_playOnLoadPlaylist;
    bool m_ignorePlaybackErrors;
    int m_mediaToAdd;
    bool m_displayTitleName;
    bool m_playlistAutoSort;
    bool m_caseSensitiveSearch;
    bool m_autosavePlaylistOnExit;
    QString m_screenshotFolder;
    bool m_proxyEnabled;
    int m_proxyType;
    QString m_proxyHost;
    int m_proxyPort;
    QString m_proxyUsername;
    QString m_proxyPassword;
    QString m_opensubtitlesApiKey;
    QString m_opensubtitlesUsername;
    QString m_opensubtitlesPassword;
    bool m_youtubeEnabled;
    QString m_ytdlPath;
    int m_youtubeQuality;
    int m_youtubeMode;
    QString m_youtubeCookiesFile;
    QString m_youtubeFfmpegLocation;
    QString m_youtubeCacheDir;
    int m_youtubeCacheSize;
    int m_youtubeThumbnailOffset;
    int m_youtubeCacheThumbSize;
    QString m_youtubeDownloaderCommand;
    QString m_youtubeDownloaderArgs;
    QString m_youtubeDownloadFolder;
    bool m_updateCheckEnabled;
    int m_updateCheckIntervalDays;
    QString m_updateLastCheck;
    QString m_updateLastKnownVersion;
    QString m_screenshotFormat;
    int m_recentsMaxItems;
    int m_mainwindowResizeMode;
    bool m_preventOutsideScreen;
    bool m_centerWindow;
    bool m_hideVideoOnAudio;
    bool m_useNativeFileDialog;
    bool m_seekOnDrag;
    bool m_singleInstance;
    int m_urlHistoryMax;
    int m_castPort;
    bool m_rememberLastDir;
    QString m_qtStyle;
    QString m_uiLanguage;
    qreal m_interfaceScaleFactor;
    int m_wheelFunction;
    int m_doubleClickFunction;
    int m_middleClickFunction;
    bool m_autoAddFolderFiles;
    QString m_audioDevice;
    bool m_rememberFileSettings;
    bool m_rememberVolume;
    int m_initialVolume;
    int m_osdFontSize;
    bool m_hideMouseInFullscreen;
    int m_mouseHideDelay;
    bool m_dvdUseMenus;
    bool m_dvdUseFirstPlay;
    int m_dvdMenuTimeout;
    int m_leftClickFunction;
    bool m_autoPlayNext;
    bool m_addDirectoriesRecursively;
    bool m_startInFullscreen;
    bool m_subtitlesAutoload;
    QString m_subFontFamily;
    int m_subFontSize;
    int m_subPosition;
    bool m_playFilesFromStart;
    QString m_uiFontFamily;
    int m_uiFontSize;
    QStringList m_urlHistory;
    QString m_iconSet;
    QString m_gui;
    int m_eqBrightness;
    int m_eqContrast;
    int m_eqHue;
    int m_eqSaturation;
    int m_eqGamma;
    int m_eqDefBrightness;
    int m_eqDefContrast;
    int m_eqDefHue;
    int m_eqDefSaturation;
    int m_eqDefGamma;
    bool m_toolbarGradient;
    QStringList m_mainToolbarItems;
    QStringList m_controlBarItems;
    int m_mainToolbarIconSize;
    int m_controlBarIconSize;
    bool m_showTrayIcon;
    bool m_showToolbar;
    bool m_showControlBar;
    bool m_showStatusBar;
    bool m_statusVideoInfo;
    bool m_statusAudioInfo;
    bool m_statusFormatInfo;
    bool m_statusBitrateInfo;
    bool m_statusFrameCounter;
    bool m_timeDisplayRemaining;
    bool m_showMilliseconds;
    bool m_touchMode;
    bool m_gestureSeek;
};

#endif // SETTINGS_H
