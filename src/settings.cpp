/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "settings.h"

#include "securestore.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaMethod>
#include <QMetaProperty>
#include <QStandardPaths>

namespace Keys {
constexpr auto volume = "playback/volume";
constexpr auto muted = "playback/muted";
constexpr auto playbackRate = "playback/rate";
constexpr auto pitchCompensation = "playback/pitchCompensation";
constexpr auto lastOpenFolder = "ui/lastOpenFolder";
constexpr auto playlistShuffle = "playlist/shuffle";
constexpr auto playlistRepeat = "playlist/repeat";
constexpr auto playlistAsWindow = "ui/playlistAsWindow";
constexpr auto resumePlayback = "playback/resume";
constexpr auto osdEnabled = "ui/osd";
constexpr auto osdDuration = "ui/osdDuration";
constexpr auto networkTimeout = "network/timeout";
constexpr auto preferredAudioLanguage = "playback/preferredAudioLanguage";
constexpr auto preferredSubtitleLanguage = "playback/preferredSubtitleLanguage";
constexpr auto subtitlesByDefault = "playback/subtitlesByDefault";
constexpr auto closeOnFinish = "playback/closeOnFinish";
constexpr auto disableScreensaver = "playback/disableScreensaver";
constexpr auto pauseWhenMinimized = "playback/pauseWhenMinimized";
constexpr auto volumeStep = "playback/volumeStep";
constexpr auto seekShortStep = "playback/seekShortStep";
constexpr auto seekMediumStep = "playback/seekMediumStep";
constexpr auto seekLongStep = "playback/seekLongStep";
constexpr auto seekWheelStep = "playback/seekWheelStep";
constexpr auto audioDeviceDelays = "playback/audioDeviceDelays";
constexpr auto rememberGeometry = "ui/rememberGeometry";
constexpr auto windowGeometry = "ui/windowGeometry";
constexpr auto restorePlaylist = "playlist/restoreOnStartup";
constexpr auto playOnLoadPlaylist = "playlist/playOnLoad";
constexpr auto ignorePlaybackErrors = "playlist/ignoreErrors";
constexpr auto mediaToAdd = "playlist/mediaToAdd";
constexpr auto displayTitleName = "playlist/displayTitleName";
constexpr auto playlistAutoSort = "playlist/autoSort";
constexpr auto caseSensitiveSearch = "playlist/caseSensitiveSearch";
constexpr auto autosavePlaylistOnExit = "playlist/autosaveOnExit";
constexpr auto screenshotFolder = "screenshots/folder";
constexpr auto screenshotFormat = "screenshots/format";
constexpr auto opensubtitlesApiKey = "opensubtitles/apiKey";
constexpr auto opensubtitlesUsername = "opensubtitles/username";
constexpr auto opensubtitlesPassword = "opensubtitles/password";
constexpr auto proxyEnabled = "network/proxyEnabled";
constexpr auto proxyType = "network/proxyType";
constexpr auto proxyHost = "network/proxyHost";
constexpr auto proxyPort = "network/proxyPort";
constexpr auto proxyUsername = "network/proxyUsername";
constexpr auto proxyPassword = "network/proxyPassword";
constexpr auto youtubeEnabled = "youtube/enabled";
constexpr auto ytdlPath = "youtube/ytdlPath";
constexpr auto youtubeQuality = "youtube/quality";
constexpr auto youtubeMode = "youtube/mode";
constexpr auto youtubeCookiesFile = "youtube/cookiesFile";
constexpr auto youtubeFfmpegLocation = "youtube/ffmpegLocation";
constexpr auto youtubeCacheDir = "youtube/cacheDir";
constexpr auto youtubeCacheSize = "youtube/cacheSize";
constexpr auto youtubeThumbnailOffset = "youtube/thumbnailOffset";
constexpr auto youtubeCacheThumbSize = "youtube/cacheThumbSize";
constexpr auto youtubeDownloaderCommand = "youtube/downloaderCommand";
constexpr auto youtubeDownloaderArgs = "youtube/downloaderArgs";
constexpr auto youtubeDownloadFolder = "youtube/downloadFolder";
constexpr auto updateCheckEnabled = "updates/checkEnabled";
constexpr auto updateCheckIntervalDays = "updates/checkIntervalDays";
constexpr auto updateLastCheck = "updates/lastCheck";
constexpr auto updateLastKnownVersion = "updates/lastKnownVersion";
constexpr auto recentsMaxItems = "history/recentsMaxItems";
constexpr auto mainwindowResizeMode = "ui/mainwindowResizeMode";
constexpr auto preventOutsideScreen = "ui/preventOutsideScreen";
constexpr auto centerWindow = "ui/centerWindow";
constexpr auto hideVideoOnAudio = "ui/hideVideoOnAudio";
constexpr auto useNativeFileDialog = "ui/useNativeFileDialog";
constexpr auto seekOnDrag = "ui/seekOnDrag";
constexpr auto singleInstance = "ui/singleInstance";
constexpr auto urlHistoryMax = "history/urlHistoryMax";
constexpr auto castPort = "cast/port";
constexpr auto rememberLastDir = "ui/rememberLastDir";
constexpr auto qtStyle = "ui/qtStyle";
constexpr auto uiLanguage = "ui/language";
constexpr auto interfaceScaleFactor = "ui/interfaceScaleFactor";
constexpr auto wheelFunction = "input/wheelFunction";
constexpr auto doubleClickFunction = "input/doubleClickFunction";
constexpr auto middleClickFunction = "input/middleClickFunction";
constexpr auto autoAddFolderFiles = "playlist/autoAddFolderFiles";
constexpr auto audioDevice = "audio/device";
constexpr auto rememberFileSettings = "playback/rememberFileSettings";
constexpr auto rememberVolume = "playback/rememberVolume";
constexpr auto initialVolume = "playback/initialVolume";
constexpr auto osdFontSize = "ui/osdFontSize";
constexpr auto hideMouseInFullscreen = "ui/hideMouseFullscreen";
constexpr auto mouseHideDelay = "ui/mouseHideDelay";
constexpr auto dvdUseMenus = "dvd/useMenus";
constexpr auto dvdUseFirstPlay = "dvd/useFirstPlay";
constexpr auto dvdMenuTimeout = "dvd/menuTimeout";
constexpr auto leftClickFunction = "input/leftClickFunction";
constexpr auto autoPlayNext = "playlist/autoPlayNext";
constexpr auto addDirectoriesRecursively = "playlist/addDirectoriesRecursively";
constexpr auto startInFullscreen = "video/startInFullscreen";
constexpr auto subtitlesAutoload = "subtitles/autoload";
constexpr auto subFontFamily = "subtitles/fontFamily";
constexpr auto subFontSize = "subtitles/fontSize";
constexpr auto subPosition = "subtitles/position";
constexpr auto playFilesFromStart = "playlist/playFromStart";
constexpr auto uiFontFamily = "ui/fontFamily";
constexpr auto uiFontSize = "ui/fontSize";
constexpr auto urlHistory = "history/urls";
constexpr auto iconSet = "ui/iconSet";
constexpr auto gui = "ui/gui";
constexpr auto eqBrightness = "video/eqBrightness";
constexpr auto eqContrast = "video/eqContrast";
constexpr auto eqHue = "video/eqHue";
constexpr auto eqSaturation = "video/eqSaturation";
constexpr auto eqGamma = "video/eqGamma";
constexpr auto eqDefBrightness = "video/eqDefBrightness";
constexpr auto eqDefContrast = "video/eqDefContrast";
constexpr auto eqDefHue = "video/eqDefHue";
constexpr auto eqDefSaturation = "video/eqDefSaturation";
constexpr auto eqDefGamma = "video/eqDefGamma";
constexpr auto toolbarGradient = "ui/toolbarGradient";
constexpr auto mainToolbarItems = "ui/mainToolbarItems";
constexpr auto controlBarItems = "ui/controlBarItems";
constexpr auto mainToolbarIconSize = "ui/mainToolbarIconSize";
constexpr auto controlBarIconSize = "ui/controlBarIconSize";
constexpr auto showTrayIcon = "ui/showTrayIcon";
constexpr auto showToolbar = "ui/showToolbar";
constexpr auto showControlBar = "ui/showControlBar";
constexpr auto showStatusBar = "ui/showStatusBar";
constexpr auto statusVideoInfo = "ui/statusVideoInfo";
constexpr auto statusAudioInfo = "ui/statusAudioInfo";
constexpr auto statusFormatInfo = "ui/statusFormatInfo";
constexpr auto statusBitrateInfo = "ui/statusBitrateInfo";
constexpr auto statusFrameCounter = "ui/statusFrameCounter";
constexpr auto timeDisplayRemaining = "ui/timeDisplayRemaining";
constexpr auto showMilliseconds = "ui/showMilliseconds";
constexpr auto touchMode = "ui/touchMode";
constexpr auto gestureSeek = "ui/gestureSeek";
}

static QString defaultScreenshotFolder()
{
    return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
            + QStringLiteral("/Vivace");
}

static QString defaultYoutubeCacheDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)
            + QStringLiteral("/Vivace YouTube");
}

// Reads a password from the OS secure store; one-time migration for anyone
// upgrading from a version that kept it in plaintext QSettings under the
// same key: adopt the legacy value into the keychain, then erase it from
// QSettings so it stops being readable on disk.
static QString readSecurePassword(QSettings &store, const QString &key)
{
    QString value = SecureCredentials::read(key);
    if (value.isEmpty()) {
        const QString legacy = store.value(key).toString();
        if (!legacy.isEmpty()) {
            SecureCredentials::write(key, legacy);
            store.remove(key);
            value = legacy;
        }
    }
    return value;
}

Settings::Settings(QObject *parent)
    : QObject(parent),
      m_volume(qBound(0.0, m_store.value(Keys::volume, 1.0).toDouble(), 1.0)),
      m_muted(m_store.value(Keys::muted, false).toBool()),
      m_playbackRate(m_store.value(Keys::playbackRate, 1.0).toDouble()),
      m_pitchCompensation(m_store.value(Keys::pitchCompensation, true).toBool()),
      m_lastOpenFolder(m_store.value(Keys::lastOpenFolder).toUrl()),
      m_playlistShuffle(m_store.value(Keys::playlistShuffle, false).toBool()),
      m_playlistRepeat(m_store.value(Keys::playlistRepeat, false).toBool()),
      m_playlistAsWindow(m_store.value(Keys::playlistAsWindow, false).toBool()),
      m_resumePlayback(m_store.value(Keys::resumePlayback, true).toBool()),
      m_osdEnabled(m_store.value(Keys::osdEnabled, true).toBool()),
      m_osdDuration(m_store.value(Keys::osdDuration, 2000).toInt()),
      m_networkTimeout(m_store.value(Keys::networkTimeout, 60).toInt()),
      m_preferredAudioLanguage(
              m_store.value(Keys::preferredAudioLanguage).toString()),
      m_preferredSubtitleLanguage(
              m_store.value(Keys::preferredSubtitleLanguage).toString()),
      m_subtitlesByDefault(m_store.value(Keys::subtitlesByDefault, true).toBool()),
      m_closeOnFinish(m_store.value(Keys::closeOnFinish, false).toBool()),
      m_disableScreensaver(m_store.value(Keys::disableScreensaver, true).toBool()),
      m_pauseWhenMinimized(m_store.value(Keys::pauseWhenMinimized, false).toBool()),
      m_volumeStep(qBound(1, m_store.value(Keys::volumeStep, 5).toInt(), 25)),
      m_seekShortStep(qBound(1, m_store.value(Keys::seekShortStep, 10).toInt(), 60)),
      m_seekMediumStep(
              qBound(5, m_store.value(Keys::seekMediumStep, 60).toInt(), 600)),
      m_seekLongStep(
              qBound(30, m_store.value(Keys::seekLongStep, 600).toInt(), 3600)),
      m_seekWheelStep(
              qBound(1, m_store.value(Keys::seekWheelStep, 10).toInt(), 600)),
      m_rememberGeometry(m_store.value(Keys::rememberGeometry, true).toBool()),
      m_windowGeometry(m_store.value(Keys::windowGeometry).toRect()),
      m_restorePlaylist(m_store.value(Keys::restorePlaylist, false).toBool()),
      m_playOnLoadPlaylist(
              m_store.value(Keys::playOnLoadPlaylist, true).toBool()),
      m_ignorePlaybackErrors(
              m_store.value(Keys::ignorePlaybackErrors, false).toBool()),
      m_mediaToAdd(qBound(0, m_store.value(Keys::mediaToAdd, 3).toInt(), 4)),
      m_displayTitleName(m_store.value(Keys::displayTitleName, true).toBool()),
      m_playlistAutoSort(m_store.value(Keys::playlistAutoSort, false).toBool()),
      m_caseSensitiveSearch(
              m_store.value(Keys::caseSensitiveSearch, false).toBool()),
      m_autosavePlaylistOnExit(
              m_store.value(Keys::autosavePlaylistOnExit, false).toBool()),
      m_screenshotFolder(m_store.value(Keys::screenshotFolder,
                                       defaultScreenshotFolder()).toString()),
      m_opensubtitlesApiKey(
              m_store.value(Keys::opensubtitlesApiKey).toString()),
      m_opensubtitlesUsername(
              m_store.value(Keys::opensubtitlesUsername).toString()),
      m_opensubtitlesPassword(
              readSecurePassword(m_store, Keys::opensubtitlesPassword)),
      m_proxyEnabled(m_store.value(Keys::proxyEnabled, false).toBool()),
      m_proxyType(qBound(0, m_store.value(Keys::proxyType, 0).toInt(), 1)),
      m_proxyHost(m_store.value(Keys::proxyHost).toString()),
      m_proxyPort(m_store.value(Keys::proxyPort, 0).toInt()),
      m_proxyUsername(m_store.value(Keys::proxyUsername).toString()),
      m_proxyPassword(readSecurePassword(m_store, Keys::proxyPassword)),
      m_youtubeEnabled(m_store.value(Keys::youtubeEnabled, false).toBool()),
      m_ytdlPath(m_store.value(Keys::ytdlPath, QStringLiteral("yt-dlp"))
                         .toString()),
      m_youtubeQuality(m_store.value(Keys::youtubeQuality, 720).toInt()),
      m_youtubeMode(qBound(0, m_store.value(Keys::youtubeMode, 0).toInt(), 2)),
      m_youtubeCookiesFile(m_store.value(Keys::youtubeCookiesFile).toString()),
      m_youtubeFfmpegLocation(
              m_store.value(Keys::youtubeFfmpegLocation).toString()),
      m_youtubeCacheDir(m_store.value(Keys::youtubeCacheDir,
                                      defaultYoutubeCacheDir()).toString()),
      m_youtubeCacheSize(
              qBound(1, m_store.value(Keys::youtubeCacheSize, 100).toInt(), 10000)),
      m_youtubeThumbnailOffset(
              qBound(0, m_store.value(Keys::youtubeThumbnailOffset, 10).toInt(), 3600)),
      m_youtubeCacheThumbSize(
              qBound(0, m_store.value(Keys::youtubeCacheThumbSize, 1).toInt(), 2)),
      m_youtubeDownloaderCommand(
              m_store.value(Keys::youtubeDownloaderCommand).toString()),
      m_youtubeDownloaderArgs(m_store.value(Keys::youtubeDownloaderArgs,
                                            QStringLiteral("{url}")).toString()),
      m_youtubeDownloadFolder(
              m_store.value(Keys::youtubeDownloadFolder).toString()),
      m_updateCheckEnabled(
              m_store.value(Keys::updateCheckEnabled, true).toBool()),
      m_updateCheckIntervalDays(
              m_store.value(Keys::updateCheckIntervalDays, 7).toInt()),
      m_updateLastCheck(m_store.value(Keys::updateLastCheck).toString()),
      m_updateLastKnownVersion(
              m_store.value(Keys::updateLastKnownVersion).toString()),
      m_screenshotFormat(
              m_store.value(Keys::screenshotFormat, QStringLiteral("png"))
                      .toString()),
      m_recentsMaxItems(
              qBound(1, m_store.value(Keys::recentsMaxItems, 10).toInt(), 50)),
      m_mainwindowResizeMode(
              qBound(0, m_store.value(Keys::mainwindowResizeMode, 1).toInt(), 2)),
      m_preventOutsideScreen(
              m_store.value(Keys::preventOutsideScreen, true).toBool()),
      m_centerWindow(m_store.value(Keys::centerWindow, false).toBool()),
      m_hideVideoOnAudio(m_store.value(Keys::hideVideoOnAudio, false).toBool()),
      m_useNativeFileDialog(
              m_store.value(Keys::useNativeFileDialog, true).toBool()),
      m_seekOnDrag(m_store.value(Keys::seekOnDrag, true).toBool()),
      m_singleInstance(m_store.value(Keys::singleInstance, false).toBool()),
      m_urlHistoryMax(
              qBound(1, m_store.value(Keys::urlHistoryMax, 20).toInt(), 100)),
      // Default deliberately sits BELOW every major OS's ephemeral/dynamic
      // client-port range, not just IANA's 49152-65535 dynamic/private
      // range: Windows and macOS both default their outgoing ephemeral
      // ports to exactly that IANA range, but Linux's default
      // (/proc/sys/net/ipv4/ip_local_port_range) commonly starts lower, at
      // 32768 - so the union of "ephemeral somewhere" is ~32768-65535.
      // Staying under 32768 means this fixed listening port can't collide
      // with a transient outgoing connection on any of the three. Not a
      // registered/well-known service port either (checked against the
      // common ones: FTP/SSH/HTTP(S)/MySQL/Postgres/Redis/MongoDB/RDP/VNC/
      // BitTorrent/etc.).
      m_castPort(qBound(1024, m_store.value(Keys::castPort, 23890).toInt(), 65535)),
      m_rememberLastDir(m_store.value(Keys::rememberLastDir, true).toBool()),
      m_qtStyle(m_store.value(Keys::qtStyle, QStringLiteral("Fusion")).toString()),
      m_uiLanguage(m_store.value(Keys::uiLanguage).toString()),
      m_interfaceScaleFactor(
              m_store.value(Keys::interfaceScaleFactor, 0.0).toReal()),
      m_wheelFunction(qBound(0, m_store.value(Keys::wheelFunction, 0).toInt(), 2)),
      m_doubleClickFunction(
              qBound(0, m_store.value(Keys::doubleClickFunction, 0).toInt(), 2)),
      m_middleClickFunction(
              qBound(0, m_store.value(Keys::middleClickFunction, 0).toInt(), 3)),
      m_autoAddFolderFiles(
              m_store.value(Keys::autoAddFolderFiles, false).toBool()),
      m_audioDevice(m_store.value(Keys::audioDevice).toString()),
      m_rememberFileSettings(
              m_store.value(Keys::rememberFileSettings, true).toBool()),
      m_rememberVolume(m_store.value(Keys::rememberVolume, true).toBool()),
      m_initialVolume(
              qBound(0, m_store.value(Keys::initialVolume, 100).toInt(), 100)),
      m_osdFontSize(qBound(10, m_store.value(Keys::osdFontSize, 22).toInt(), 60)),
      m_hideMouseInFullscreen(
              m_store.value(Keys::hideMouseInFullscreen, true).toBool()),
      m_mouseHideDelay(
              qBound(500, m_store.value(Keys::mouseHideDelay, 1000).toInt(), 10000)),
      m_dvdUseMenus(m_store.value(Keys::dvdUseMenus, true).toBool()),
      m_dvdUseFirstPlay(m_store.value(Keys::dvdUseFirstPlay, false).toBool()),
      m_dvdMenuTimeout(
              qBound(0, m_store.value(Keys::dvdMenuTimeout, 30).toInt(), 600)),
      m_leftClickFunction(
              qBound(0, m_store.value(Keys::leftClickFunction, 0).toInt(), 2)),
      m_autoPlayNext(m_store.value(Keys::autoPlayNext, true).toBool()),
      m_addDirectoriesRecursively(
              m_store.value(Keys::addDirectoriesRecursively, false).toBool()),
      m_startInFullscreen(m_store.value(Keys::startInFullscreen, false).toBool()),
      m_subtitlesAutoload(m_store.value(Keys::subtitlesAutoload, true).toBool()),
      m_subFontFamily(m_store.value(Keys::subFontFamily).toString()),
      m_subFontSize(qBound(10, m_store.value(Keys::subFontSize, 28).toInt(), 80)),
      m_subPosition(qBound(0, m_store.value(Keys::subPosition, 95).toInt(), 100)),
      m_playFilesFromStart(
              m_store.value(Keys::playFilesFromStart, false).toBool()),
      m_uiFontFamily(m_store.value(Keys::uiFontFamily).toString()),
      m_uiFontSize(m_store.value(Keys::uiFontSize, 0).toInt()),
      m_urlHistory(m_store.value(Keys::urlHistory).toStringList()),
      m_iconSet(m_store.value(Keys::iconSet, QStringLiteral("Default")).toString()),
      m_gui(m_store.value(Keys::gui, QStringLiteral("Basic")).toString()),
      m_eqBrightness(qBound(-100, m_store.value(Keys::eqBrightness, 0).toInt(), 100)),
      m_eqContrast(qBound(-100, m_store.value(Keys::eqContrast, 0).toInt(), 100)),
      m_eqHue(qBound(-100, m_store.value(Keys::eqHue, 0).toInt(), 100)),
      m_eqSaturation(qBound(-100, m_store.value(Keys::eqSaturation, 0).toInt(), 100)),
      m_eqGamma(qBound(-100, m_store.value(Keys::eqGamma, 0).toInt(), 100)),
      m_eqDefBrightness(qBound(-100, m_store.value(Keys::eqDefBrightness, 0).toInt(), 100)),
      m_eqDefContrast(qBound(-100, m_store.value(Keys::eqDefContrast, 0).toInt(), 100)),
      m_eqDefHue(qBound(-100, m_store.value(Keys::eqDefHue, 0).toInt(), 100)),
      m_eqDefSaturation(qBound(-100, m_store.value(Keys::eqDefSaturation, 0).toInt(), 100)),
      m_eqDefGamma(qBound(-100, m_store.value(Keys::eqDefGamma, 0).toInt(), 100)),
      m_toolbarGradient(m_store.value(Keys::toolbarGradient, true).toBool()),
      m_mainToolbarItems(m_store.value(Keys::mainToolbarItems).toStringList()),
      m_controlBarItems(m_store.value(Keys::controlBarItems).toStringList()),
      m_mainToolbarIconSize(
              qBound(16, m_store.value(Keys::mainToolbarIconSize, 24).toInt(), 48)),
      m_controlBarIconSize(
              qBound(16, m_store.value(Keys::controlBarIconSize, 24).toInt(), 48)),
      m_showTrayIcon(m_store.value(Keys::showTrayIcon, false).toBool()),
      m_showToolbar(m_store.value(Keys::showToolbar, true).toBool()),
      m_showControlBar(m_store.value(Keys::showControlBar, true).toBool()),
      m_showStatusBar(m_store.value(Keys::showStatusBar, true).toBool()),
      m_statusVideoInfo(m_store.value(Keys::statusVideoInfo, false).toBool()),
      m_statusAudioInfo(m_store.value(Keys::statusAudioInfo, false).toBool()),
      m_statusFormatInfo(m_store.value(Keys::statusFormatInfo, false).toBool()),
      m_statusBitrateInfo(m_store.value(Keys::statusBitrateInfo, false).toBool()),
      m_statusFrameCounter(m_store.value(Keys::statusFrameCounter, false).toBool()),
      m_timeDisplayRemaining(
              m_store.value(Keys::timeDisplayRemaining, false).toBool()),
      m_showMilliseconds(m_store.value(Keys::showMilliseconds, false).toBool()),
      m_touchMode(m_store.value(Keys::touchMode, false).toBool()),
      m_gestureSeek(m_store.value(Keys::gestureSeek, true).toBool())
{
    // Load the per-device audio-delay store (JSON array of {id, ms}, MRU first).
    const QJsonArray arr =
        QJsonDocument::fromJson(m_store.value(Keys::audioDeviceDelays)
                                        .toByteArray())
            .array();
    for (const QJsonValue &v : arr) {
        const QJsonObject o = v.toObject();
        const QString id = o.value(QStringLiteral("id")).toString();
        if (!id.isEmpty())
            m_audioDeviceDelays.append({ id, o.value(QStringLiteral("ms")).toInt() });
    }

    // Relay every writable property's notify signal to changed(), so the
    // Preferences dialog can detect unsaved changes generically.
    const QMetaObject *mo = metaObject();
    const QMetaMethod changedSignal = mo->method(mo->indexOfSignal("changed()"));
    for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
        const QMetaProperty prop = mo->property(i);
        if (prop.isWritable() && prop.hasNotifySignal())
            connect(this, prop.notifySignal(), this, changedSignal);
    }
}

int Settings::audioDelayForDevice(const QString &deviceId) const
{
    for (const auto &entry : m_audioDeviceDelays)
        if (entry.first == deviceId)
            return entry.second;
    return 0;
}

void Settings::setAudioDelayForDevice(const QString &deviceId, int ms)
{
    ms = qBound(-10000, ms, 0);
    if (deviceId.isEmpty())
        return;
    // Remove any existing entry for this device.
    for (int i = 0; i < m_audioDeviceDelays.size(); ++i) {
        if (m_audioDeviceDelays.at(i).first == deviceId) {
            if (m_audioDeviceDelays.at(i).second == ms)
                return; // unchanged
            m_audioDeviceDelays.removeAt(i);
            break;
        }
    }
    // Only non-zero delays are worth remembering; 0 = just drop the entry.
    if (ms != 0) {
        m_audioDeviceDelays.prepend({ deviceId, ms });
        constexpr int kMaxEntries = 20;
        while (m_audioDeviceDelays.size() > kMaxEntries)
            m_audioDeviceDelays.removeLast();
    }

    QJsonArray arr;
    for (const auto &entry : m_audioDeviceDelays) {
        QJsonObject o;
        o.insert(QStringLiteral("id"), entry.first);
        o.insert(QStringLiteral("ms"), entry.second);
        arr.append(o);
    }
    m_store.setValue(Keys::audioDeviceDelays,
                     QJsonDocument(arr).toJson(QJsonDocument::Compact));
    ++m_audioDeviceDelaysRevision;
    emit audioDeviceDelaysChanged();
}

void Settings::setToolbarGradient(bool enabled)
{
    if (enabled == m_toolbarGradient)
        return;
    m_toolbarGradient = enabled;
    m_store.setValue(Keys::toolbarGradient, enabled);
    emit toolbarGradientChanged();
}

void Settings::setMainToolbarItems(const QStringList &items)
{
    if (items == m_mainToolbarItems)
        return;
    m_mainToolbarItems = items;
    m_store.setValue(Keys::mainToolbarItems, items);
    emit mainToolbarItemsChanged();
}

void Settings::setControlBarItems(const QStringList &items)
{
    if (items == m_controlBarItems)
        return;
    m_controlBarItems = items;
    m_store.setValue(Keys::controlBarItems, items);
    emit controlBarItemsChanged();
}

void Settings::setMainToolbarIconSize(int size)
{
    size = qBound(16, size, 48);
    if (size == m_mainToolbarIconSize)
        return;
    m_mainToolbarIconSize = size;
    m_store.setValue(Keys::mainToolbarIconSize, size);
    emit mainToolbarIconSizeChanged();
}

void Settings::setControlBarIconSize(int size)
{
    size = qBound(16, size, 48);
    if (size == m_controlBarIconSize)
        return;
    m_controlBarIconSize = size;
    m_store.setValue(Keys::controlBarIconSize, size);
    emit controlBarIconSizeChanged();
}

void Settings::setShowTrayIcon(bool show)
{
    if (show == m_showTrayIcon)
        return;
    m_showTrayIcon = show;
    m_store.setValue(Keys::showTrayIcon, show);
    emit showTrayIconChanged();
}

void Settings::setShowToolbar(bool show)
{
    if (show == m_showToolbar)
        return;
    m_showToolbar = show;
    m_store.setValue(Keys::showToolbar, show);
    emit showToolbarChanged();
}

void Settings::setShowControlBar(bool show)
{
    if (show == m_showControlBar)
        return;
    m_showControlBar = show;
    m_store.setValue(Keys::showControlBar, show);
    emit showControlBarChanged();
}

void Settings::setShowStatusBar(bool show)
{
    if (show == m_showStatusBar)
        return;
    m_showStatusBar = show;
    m_store.setValue(Keys::showStatusBar, show);
    emit showStatusBarChanged();
}

void Settings::setStatusVideoInfo(bool show)
{
    if (show == m_statusVideoInfo)
        return;
    m_statusVideoInfo = show;
    m_store.setValue(Keys::statusVideoInfo, show);
    emit statusVideoInfoChanged();
}

void Settings::setStatusAudioInfo(bool show)
{
    if (show == m_statusAudioInfo)
        return;
    m_statusAudioInfo = show;
    m_store.setValue(Keys::statusAudioInfo, show);
    emit statusAudioInfoChanged();
}

void Settings::setStatusFormatInfo(bool show)
{
    if (show == m_statusFormatInfo)
        return;
    m_statusFormatInfo = show;
    m_store.setValue(Keys::statusFormatInfo, show);
    emit statusFormatInfoChanged();
}

void Settings::setStatusBitrateInfo(bool show)
{
    if (show == m_statusBitrateInfo)
        return;
    m_statusBitrateInfo = show;
    m_store.setValue(Keys::statusBitrateInfo, show);
    emit statusBitrateInfoChanged();
}

void Settings::setStatusFrameCounter(bool show)
{
    if (show == m_statusFrameCounter)
        return;
    m_statusFrameCounter = show;
    m_store.setValue(Keys::statusFrameCounter, show);
    emit statusFrameCounterChanged();
}

void Settings::setTimeDisplayRemaining(bool remaining)
{
    if (remaining == m_timeDisplayRemaining)
        return;
    m_timeDisplayRemaining = remaining;
    m_store.setValue(Keys::timeDisplayRemaining, remaining);
    emit timeDisplayRemainingChanged();
}

void Settings::setShowMilliseconds(bool show)
{
    if (show == m_showMilliseconds)
        return;
    m_showMilliseconds = show;
    m_store.setValue(Keys::showMilliseconds, show);
    emit showMillisecondsChanged();
}

void Settings::setTouchMode(bool enabled)
{
    if (enabled == m_touchMode)
        return;
    m_touchMode = enabled;
    m_store.setValue(Keys::touchMode, enabled);
    emit touchModeChanged();
}

void Settings::setGestureSeek(bool enabled)
{
    if (enabled == m_gestureSeek)
        return;
    m_gestureSeek = enabled;
    m_store.setValue(Keys::gestureSeek, enabled);
    emit gestureSeekChanged();
}

void Settings::setIconSet(const QString &iconSet)
{
    if (iconSet == m_iconSet)
        return;
    m_iconSet = iconSet;
    m_store.setValue(Keys::iconSet, iconSet);
    emit iconSetChanged();
}

void Settings::setGui(const QString &gui)
{
    if (gui == m_gui)
        return;
    m_gui = gui;
    m_store.setValue(Keys::gui, gui);
    emit guiChanged();
}

void Settings::setEqBrightness(int v)
{
    v = qBound(-100, v, 100);
    if (v == m_eqBrightness)
        return;
    m_eqBrightness = v;
    m_store.setValue(Keys::eqBrightness, v);
    emit eqChanged();
}

void Settings::setEqContrast(int v)
{
    v = qBound(-100, v, 100);
    if (v == m_eqContrast)
        return;
    m_eqContrast = v;
    m_store.setValue(Keys::eqContrast, v);
    emit eqChanged();
}

void Settings::setEqHue(int v)
{
    v = qBound(-100, v, 100);
    if (v == m_eqHue)
        return;
    m_eqHue = v;
    m_store.setValue(Keys::eqHue, v);
    emit eqChanged();
}

void Settings::setEqSaturation(int v)
{
    v = qBound(-100, v, 100);
    if (v == m_eqSaturation)
        return;
    m_eqSaturation = v;
    m_store.setValue(Keys::eqSaturation, v);
    emit eqChanged();
}

void Settings::setEqGamma(int v)
{
    v = qBound(-100, v, 100);
    if (v == m_eqGamma)
        return;
    m_eqGamma = v;
    m_store.setValue(Keys::eqGamma, v);
    emit eqChanged();
}

void Settings::setEqDefBrightness(int v)
{
    v = qBound(-100, v, 100);
    if (v == m_eqDefBrightness)
        return;
    m_eqDefBrightness = v;
    m_store.setValue(Keys::eqDefBrightness, v);
    emit eqDefaultsChanged();
}

void Settings::setEqDefContrast(int v)
{
    v = qBound(-100, v, 100);
    if (v == m_eqDefContrast)
        return;
    m_eqDefContrast = v;
    m_store.setValue(Keys::eqDefContrast, v);
    emit eqDefaultsChanged();
}

void Settings::setEqDefHue(int v)
{
    v = qBound(-100, v, 100);
    if (v == m_eqDefHue)
        return;
    m_eqDefHue = v;
    m_store.setValue(Keys::eqDefHue, v);
    emit eqDefaultsChanged();
}

void Settings::setEqDefSaturation(int v)
{
    v = qBound(-100, v, 100);
    if (v == m_eqDefSaturation)
        return;
    m_eqDefSaturation = v;
    m_store.setValue(Keys::eqDefSaturation, v);
    emit eqDefaultsChanged();
}

void Settings::setEqDefGamma(int v)
{
    v = qBound(-100, v, 100);
    if (v == m_eqDefGamma)
        return;
    m_eqDefGamma = v;
    m_store.setValue(Keys::eqDefGamma, v);
    emit eqDefaultsChanged();
}

void Settings::addUrlToHistory(const QString &url)
{
    const QString trimmed = url.trimmed();
    if (trimmed.isEmpty())
        return;
    m_urlHistory.removeAll(trimmed);
    m_urlHistory.prepend(trimmed);
    while (m_urlHistory.size() > m_urlHistoryMax)
        m_urlHistory.removeLast();
    m_store.setValue(Keys::urlHistory, m_urlHistory);
    emit urlHistoryChanged();
}

void Settings::setVolume(qreal volume)
{
    volume = qBound(0.0, volume, 1.0);
    if (qFuzzyCompare(volume, m_volume))
        return;
    m_volume = volume;
    m_store.setValue(Keys::volume, volume);
    emit volumeChanged();
}

void Settings::setMuted(bool muted)
{
    if (muted == m_muted)
        return;
    m_muted = muted;
    m_store.setValue(Keys::muted, muted);
    emit mutedChanged();
}

void Settings::setPlaybackRate(qreal rate)
{
    if (rate <= 0 || qFuzzyCompare(rate, m_playbackRate))
        return;
    m_playbackRate = rate;
    m_store.setValue(Keys::playbackRate, rate);
    emit playbackRateChanged();
}

void Settings::setPitchCompensation(bool enabled)
{
    if (enabled == m_pitchCompensation)
        return;
    m_pitchCompensation = enabled;
    m_store.setValue(Keys::pitchCompensation, enabled);
    emit pitchCompensationChanged();
}

void Settings::setLastOpenFolder(const QUrl &folder)
{
    if (folder == m_lastOpenFolder)
        return;
    m_lastOpenFolder = folder;
    m_store.setValue(Keys::lastOpenFolder, folder);
    emit lastOpenFolderChanged();
}

void Settings::setPlaylistShuffle(bool shuffle)
{
    if (shuffle == m_playlistShuffle)
        return;
    m_playlistShuffle = shuffle;
    m_store.setValue(Keys::playlistShuffle, shuffle);
    emit playlistShuffleChanged();
}

void Settings::setPlaylistRepeat(bool repeat)
{
    if (repeat == m_playlistRepeat)
        return;
    m_playlistRepeat = repeat;
    m_store.setValue(Keys::playlistRepeat, repeat);
    emit playlistRepeatChanged();
}

void Settings::setPlaylistAsWindow(bool asWindow)
{
    if (asWindow == m_playlistAsWindow)
        return;
    m_playlistAsWindow = asWindow;
    m_store.setValue(Keys::playlistAsWindow, asWindow);
    emit playlistAsWindowChanged();
}

void Settings::setResumePlayback(bool resume)
{
    if (resume == m_resumePlayback)
        return;
    m_resumePlayback = resume;
    m_store.setValue(Keys::resumePlayback, resume);
    emit resumePlaybackChanged();
}

void Settings::setOsdEnabled(bool enabled)
{
    if (enabled == m_osdEnabled)
        return;
    m_osdEnabled = enabled;
    m_store.setValue(Keys::osdEnabled, enabled);
    emit osdEnabledChanged();
}

void Settings::setOsdDuration(int durationMs)
{
    durationMs = qBound(500, durationMs, 10000);
    if (durationMs == m_osdDuration)
        return;
    m_osdDuration = durationMs;
    m_store.setValue(Keys::osdDuration, durationMs);
    emit osdDurationChanged();
}

void Settings::setNetworkTimeout(int seconds)
{
    seconds = qBound(5, seconds, 300);
    if (seconds == m_networkTimeout)
        return;
    m_networkTimeout = seconds;
    m_store.setValue(Keys::networkTimeout, seconds);
    emit networkTimeoutChanged();
}

void Settings::setPreferredAudioLanguage(const QString &languages)
{
    if (languages == m_preferredAudioLanguage)
        return;
    m_preferredAudioLanguage = languages;
    m_store.setValue(Keys::preferredAudioLanguage, languages);
    emit preferredAudioLanguageChanged();
}

void Settings::setPreferredSubtitleLanguage(const QString &languages)
{
    if (languages == m_preferredSubtitleLanguage)
        return;
    m_preferredSubtitleLanguage = languages;
    m_store.setValue(Keys::preferredSubtitleLanguage, languages);
    emit preferredSubtitleLanguageChanged();
}

void Settings::setSubtitlesByDefault(bool enabled)
{
    if (enabled == m_subtitlesByDefault)
        return;
    m_subtitlesByDefault = enabled;
    m_store.setValue(Keys::subtitlesByDefault, enabled);
    emit subtitlesByDefaultChanged();
}

void Settings::setCloseOnFinish(bool close)
{
    if (close == m_closeOnFinish)
        return;
    m_closeOnFinish = close;
    m_store.setValue(Keys::closeOnFinish, close);
    emit closeOnFinishChanged();
}

void Settings::setDisableScreensaver(bool disable)
{
    if (disable == m_disableScreensaver)
        return;
    m_disableScreensaver = disable;
    m_store.setValue(Keys::disableScreensaver, disable);
    emit disableScreensaverChanged();
}

void Settings::setPauseWhenMinimized(bool pause)
{
    if (pause == m_pauseWhenMinimized)
        return;
    m_pauseWhenMinimized = pause;
    m_store.setValue(Keys::pauseWhenMinimized, pause);
    emit pauseWhenMinimizedChanged();
}

void Settings::setVolumeStep(int stepPercent)
{
    stepPercent = qBound(1, stepPercent, 25);
    if (stepPercent == m_volumeStep)
        return;
    m_volumeStep = stepPercent;
    m_store.setValue(Keys::volumeStep, stepPercent);
    emit volumeStepChanged();
}

void Settings::setSeekShortStep(int seconds)
{
    seconds = qBound(1, seconds, 60);
    if (seconds == m_seekShortStep)
        return;
    m_seekShortStep = seconds;
    m_store.setValue(Keys::seekShortStep, seconds);
    emit seekShortStepChanged();
}

void Settings::setSeekMediumStep(int seconds)
{
    seconds = qBound(5, seconds, 600);
    if (seconds == m_seekMediumStep)
        return;
    m_seekMediumStep = seconds;
    m_store.setValue(Keys::seekMediumStep, seconds);
    emit seekMediumStepChanged();
}

void Settings::setSeekLongStep(int seconds)
{
    seconds = qBound(30, seconds, 3600);
    if (seconds == m_seekLongStep)
        return;
    m_seekLongStep = seconds;
    m_store.setValue(Keys::seekLongStep, seconds);
    emit seekLongStepChanged();
}

void Settings::setSeekWheelStep(int seconds)
{
    seconds = qBound(1, seconds, 600);
    if (seconds == m_seekWheelStep)
        return;
    m_seekWheelStep = seconds;
    m_store.setValue(Keys::seekWheelStep, seconds);
    emit seekWheelStepChanged();
}

void Settings::setRememberGeometry(bool remember)
{
    if (remember == m_rememberGeometry)
        return;
    m_rememberGeometry = remember;
    m_store.setValue(Keys::rememberGeometry, remember);
    emit rememberGeometryChanged();
}

void Settings::setWindowGeometry(const QRect &geometry)
{
    if (geometry == m_windowGeometry)
        return;
    m_windowGeometry = geometry;
    m_store.setValue(Keys::windowGeometry, geometry);
    emit windowGeometryChanged();
}

void Settings::setRestorePlaylist(bool restore)
{
    if (restore == m_restorePlaylist)
        return;
    m_restorePlaylist = restore;
    m_store.setValue(Keys::restorePlaylist, restore);
    emit restorePlaylistChanged();
}

void Settings::setPlayOnLoadPlaylist(bool play)
{
    if (play == m_playOnLoadPlaylist)
        return;
    m_playOnLoadPlaylist = play;
    m_store.setValue(Keys::playOnLoadPlaylist, play);
    emit playOnLoadPlaylistChanged();
}

void Settings::setIgnorePlaybackErrors(bool ignore)
{
    if (ignore == m_ignorePlaybackErrors)
        return;
    m_ignorePlaybackErrors = ignore;
    m_store.setValue(Keys::ignorePlaybackErrors, ignore);
    emit ignorePlaybackErrorsChanged();
}

void Settings::setMediaToAdd(int mode)
{
    mode = qBound(0, mode, 4);
    if (mode == m_mediaToAdd)
        return;
    m_mediaToAdd = mode;
    m_store.setValue(Keys::mediaToAdd, mode);
    emit mediaToAddChanged();
}

void Settings::setDisplayTitleName(bool display)
{
    if (display == m_displayTitleName)
        return;
    m_displayTitleName = display;
    m_store.setValue(Keys::displayTitleName, display);
    emit displayTitleNameChanged();
}

void Settings::setPlaylistAutoSort(bool autoSort)
{
    if (autoSort == m_playlistAutoSort)
        return;
    m_playlistAutoSort = autoSort;
    m_store.setValue(Keys::playlistAutoSort, autoSort);
    emit playlistAutoSortChanged();
}

void Settings::setCaseSensitiveSearch(bool caseSensitive)
{
    if (caseSensitive == m_caseSensitiveSearch)
        return;
    m_caseSensitiveSearch = caseSensitive;
    m_store.setValue(Keys::caseSensitiveSearch, caseSensitive);
    emit caseSensitiveSearchChanged();
}

void Settings::setAutosavePlaylistOnExit(bool autosave)
{
    if (autosave == m_autosavePlaylistOnExit)
        return;
    m_autosavePlaylistOnExit = autosave;
    m_store.setValue(Keys::autosavePlaylistOnExit, autosave);
    emit autosavePlaylistOnExitChanged();
}

void Settings::setScreenshotFolder(const QString &folder)
{
    if (folder == m_screenshotFolder)
        return;
    m_screenshotFolder = folder;
    m_store.setValue(Keys::screenshotFolder, folder);
    emit screenshotFolderChanged();
}

void Settings::setOpensubtitlesApiKey(const QString &key)
{
    if (key == m_opensubtitlesApiKey)
        return;
    m_opensubtitlesApiKey = key;
    m_store.setValue(Keys::opensubtitlesApiKey, key);
    emit opensubtitlesChanged();
}

void Settings::setOpensubtitlesUsername(const QString &user)
{
    if (user == m_opensubtitlesUsername)
        return;
    m_opensubtitlesUsername = user;
    m_store.setValue(Keys::opensubtitlesUsername, user);
    emit opensubtitlesChanged();
}

void Settings::setOpensubtitlesPassword(const QString &pass)
{
    if (pass == m_opensubtitlesPassword)
        return;
    m_opensubtitlesPassword = pass;
    SecureCredentials::write(Keys::opensubtitlesPassword, pass);
    emit opensubtitlesChanged();
}

void Settings::setProxyEnabled(bool enabled)
{
    if (enabled == m_proxyEnabled)
        return;
    m_proxyEnabled = enabled;
    m_store.setValue(Keys::proxyEnabled, enabled);
    emit proxyChanged();
}

void Settings::setProxyType(int type)
{
    type = qBound(0, type, 1);
    if (type == m_proxyType)
        return;
    m_proxyType = type;
    m_store.setValue(Keys::proxyType, type);
    emit proxyChanged();
}

void Settings::setProxyHost(const QString &host)
{
    if (host == m_proxyHost)
        return;
    m_proxyHost = host;
    m_store.setValue(Keys::proxyHost, host);
    emit proxyChanged();
}

void Settings::setProxyPort(int port)
{
    port = qBound(0, port, 65535);
    if (port == m_proxyPort)
        return;
    m_proxyPort = port;
    m_store.setValue(Keys::proxyPort, port);
    emit proxyChanged();
}

void Settings::setProxyUsername(const QString &user)
{
    if (user == m_proxyUsername)
        return;
    m_proxyUsername = user;
    m_store.setValue(Keys::proxyUsername, user);
    emit proxyChanged();
}

void Settings::setProxyPassword(const QString &pass)
{
    if (pass == m_proxyPassword)
        return;
    m_proxyPassword = pass;
    SecureCredentials::write(Keys::proxyPassword, pass);
    emit proxyChanged();
}

void Settings::setYoutubeEnabled(bool enabled)
{
    if (enabled == m_youtubeEnabled)
        return;
    m_youtubeEnabled = enabled;
    m_store.setValue(Keys::youtubeEnabled, enabled);
    emit youtubeChanged();
}

void Settings::setYtdlPath(const QString &path)
{
    if (path == m_ytdlPath)
        return;
    m_ytdlPath = path;
    m_store.setValue(Keys::ytdlPath, path);
    emit youtubeChanged();
}

void Settings::setYoutubeQuality(int height)
{
    if (height == m_youtubeQuality)
        return;
    m_youtubeQuality = height;
    m_store.setValue(Keys::youtubeQuality, height);
    emit youtubeChanged();
}

void Settings::setYoutubeMode(int mode)
{
    mode = qBound(0, mode, 2);
    if (mode == m_youtubeMode)
        return;
    m_youtubeMode = mode;
    m_store.setValue(Keys::youtubeMode, mode);
    emit youtubeChanged();
}

void Settings::setYoutubeCookiesFile(const QString &path)
{
    if (path == m_youtubeCookiesFile)
        return;
    m_youtubeCookiesFile = path;
    m_store.setValue(Keys::youtubeCookiesFile, path);
    emit youtubeChanged();
}

void Settings::setYoutubeFfmpegLocation(const QString &path)
{
    if (path == m_youtubeFfmpegLocation)
        return;
    m_youtubeFfmpegLocation = path;
    m_store.setValue(Keys::youtubeFfmpegLocation, path);
    emit youtubeChanged();
}

void Settings::setYoutubeCacheDir(const QString &path)
{
    // An empty value falls back to the default cache location.
    const QString dir = path.isEmpty() ? defaultYoutubeCacheDir() : path;
    if (dir == m_youtubeCacheDir)
        return;
    m_youtubeCacheDir = dir;
    m_store.setValue(Keys::youtubeCacheDir, dir);
    emit youtubeChanged();
}

void Settings::setYoutubeCacheSize(int maxFiles)
{
    maxFiles = qBound(1, maxFiles, 10000);
    if (maxFiles == m_youtubeCacheSize)
        return;
    m_youtubeCacheSize = maxFiles;
    m_store.setValue(Keys::youtubeCacheSize, maxFiles);
    emit youtubeChanged();
}

void Settings::setYoutubeThumbnailOffset(int seconds)
{
    seconds = qBound(0, seconds, 3600);
    if (seconds == m_youtubeThumbnailOffset)
        return;
    m_youtubeThumbnailOffset = seconds;
    m_store.setValue(Keys::youtubeThumbnailOffset, seconds);
    emit youtubeChanged();
}

void Settings::setYoutubeCacheThumbSize(int size)
{
    size = qBound(0, size, 2);
    if (size == m_youtubeCacheThumbSize)
        return;
    m_youtubeCacheThumbSize = size;
    m_store.setValue(Keys::youtubeCacheThumbSize, size);
    emit youtubeChanged();
}

void Settings::setYoutubeDownloaderCommand(const QString &command)
{
    if (command == m_youtubeDownloaderCommand)
        return;
    m_youtubeDownloaderCommand = command;
    m_store.setValue(Keys::youtubeDownloaderCommand, command);
    emit youtubeChanged();
}

void Settings::setYoutubeDownloaderArgs(const QString &args)
{
    if (args == m_youtubeDownloaderArgs)
        return;
    m_youtubeDownloaderArgs = args;
    m_store.setValue(Keys::youtubeDownloaderArgs, args);
    emit youtubeChanged();
}

void Settings::setYoutubeDownloadFolder(const QString &folder)
{
    if (folder == m_youtubeDownloadFolder)
        return;
    m_youtubeDownloadFolder = folder;
    m_store.setValue(Keys::youtubeDownloadFolder, folder);
    emit youtubeChanged();
}

void Settings::setUpdateCheckEnabled(bool enabled)
{
    if (enabled == m_updateCheckEnabled)
        return;
    m_updateCheckEnabled = enabled;
    m_store.setValue(Keys::updateCheckEnabled, enabled);
    emit updateSettingsChanged();
}

void Settings::setUpdateCheckIntervalDays(int days)
{
    if (days == m_updateCheckIntervalDays)
        return;
    m_updateCheckIntervalDays = days;
    m_store.setValue(Keys::updateCheckIntervalDays, days);
    emit updateSettingsChanged();
}

void Settings::setUpdateLastCheck(const QString &isoDate)
{
    if (isoDate == m_updateLastCheck)
        return;
    m_updateLastCheck = isoDate;
    m_store.setValue(Keys::updateLastCheck, isoDate);
    emit updateSettingsChanged();
}

void Settings::setUpdateLastKnownVersion(const QString &version)
{
    if (version == m_updateLastKnownVersion)
        return;
    m_updateLastKnownVersion = version;
    m_store.setValue(Keys::updateLastKnownVersion, version);
    emit updateSettingsChanged();
}

void Settings::setScreenshotFormat(const QString &format)
{
    if (format == m_screenshotFormat)
        return;
    m_screenshotFormat = format;
    m_store.setValue(Keys::screenshotFormat, format);
    emit screenshotFormatChanged();
}

void Settings::setRecentsMaxItems(int maxItems)
{
    maxItems = qBound(1, maxItems, 50);
    if (maxItems == m_recentsMaxItems)
        return;
    m_recentsMaxItems = maxItems;
    m_store.setValue(Keys::recentsMaxItems, maxItems);
    emit recentsMaxItemsChanged();
}

void Settings::setMainwindowResizeMode(int mode)
{
    mode = qBound(0, mode, 2);
    if (mode == m_mainwindowResizeMode)
        return;
    m_mainwindowResizeMode = mode;
    m_store.setValue(Keys::mainwindowResizeMode, mode);
    emit mainwindowResizeModeChanged();
}

void Settings::setPreventOutsideScreen(bool prevent)
{
    if (prevent == m_preventOutsideScreen)
        return;
    m_preventOutsideScreen = prevent;
    m_store.setValue(Keys::preventOutsideScreen, prevent);
    emit preventOutsideScreenChanged();
}

void Settings::setCenterWindow(bool center)
{
    if (center == m_centerWindow)
        return;
    m_centerWindow = center;
    m_store.setValue(Keys::centerWindow, center);
    emit centerWindowChanged();
}

void Settings::setHideVideoOnAudio(bool hide)
{
    if (hide == m_hideVideoOnAudio)
        return;
    m_hideVideoOnAudio = hide;
    m_store.setValue(Keys::hideVideoOnAudio, hide);
    emit hideVideoOnAudioChanged();
}

void Settings::setUseNativeFileDialog(bool useNative)
{
    if (useNative == m_useNativeFileDialog)
        return;
    m_useNativeFileDialog = useNative;
    m_store.setValue(Keys::useNativeFileDialog, useNative);
    emit useNativeFileDialogChanged();
}

void Settings::setSeekOnDrag(bool onDrag)
{
    if (onDrag == m_seekOnDrag)
        return;
    m_seekOnDrag = onDrag;
    m_store.setValue(Keys::seekOnDrag, onDrag);
    emit seekOnDragChanged();
}

void Settings::setSingleInstance(bool single)
{
    if (single == m_singleInstance)
        return;
    m_singleInstance = single;
    m_store.setValue(Keys::singleInstance, single);
    emit singleInstanceChanged();
}

void Settings::setUrlHistoryMax(int max)
{
    max = qBound(1, max, 100);
    if (max == m_urlHistoryMax)
        return;
    m_urlHistoryMax = max;
    m_store.setValue(Keys::urlHistoryMax, max);
    // Trim existing history to the new cap.
    bool trimmed = false;
    while (m_urlHistory.size() > m_urlHistoryMax) {
        m_urlHistory.removeLast();
        trimmed = true;
    }
    if (trimmed) {
        m_store.setValue(Keys::urlHistory, m_urlHistory);
        emit urlHistoryChanged();
    }
    emit urlHistoryMaxChanged();
}

void Settings::setCastPort(int port)
{
    port = qBound(1024, port, 65535);
    if (port == m_castPort)
        return;
    m_castPort = port;
    m_store.setValue(Keys::castPort, port);
    emit castPortChanged();
}

void Settings::setRememberLastDir(bool remember)
{
    if (remember == m_rememberLastDir)
        return;
    m_rememberLastDir = remember;
    m_store.setValue(Keys::rememberLastDir, remember);
    emit rememberLastDirChanged();
}

void Settings::setQtStyle(const QString &style)
{
    if (style == m_qtStyle)
        return;
    m_qtStyle = style;
    m_store.setValue(Keys::qtStyle, style);
    emit qtStyleChanged();
}

void Settings::setUiLanguage(const QString &language)
{
    if (language == m_uiLanguage)
        return;
    m_uiLanguage = language;
    m_store.setValue(Keys::uiLanguage, language);
    emit uiLanguageChanged();
}

void Settings::setInterfaceScaleFactor(qreal factor)
{
    if (qFuzzyCompare(factor, m_interfaceScaleFactor))
        return;
    m_interfaceScaleFactor = factor;
    m_store.setValue(Keys::interfaceScaleFactor, factor);
    emit interfaceScaleFactorChanged();
}

QVariantMap Settings::snapshot() const
{
    QVariantMap state;
    const QMetaObject *mo = metaObject();
    for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
        const QMetaProperty prop = mo->property(i);
        if (prop.isReadable() && prop.isWritable())
            state.insert(QString::fromLatin1(prop.name()), prop.read(this));
    }
    return state;
}

void Settings::restore(const QVariantMap &state)
{
    const QMetaObject *mo = metaObject();
    for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
        const QMetaProperty prop = mo->property(i);
        if (!prop.isWritable())
            continue;
        const auto it = state.constFind(QString::fromLatin1(prop.name()));
        if (it != state.constEnd())
            prop.write(this, *it); // setters guard equality + emit changed
    }
}

QStringList Settings::availableQtStyles() const
{
    // Qt 6 has no runtime style enumeration (QQuickStyle exposes only
    // name/setStyle), so offer the cross-platform styles that always ship
    // with Qt Quick Controls. Fusion is the recommended default.
    QStringList styles { QStringLiteral("Fusion"), QStringLiteral("Basic"),
                         QStringLiteral("Material"), QStringLiteral("Universal"),
                         QStringLiteral("Imagine") };
    // Keep whatever is stored selectable even if it is a platform style.
    if (!m_qtStyle.isEmpty() && !styles.contains(m_qtStyle))
        styles.prepend(m_qtStyle);
    return styles;
}

void Settings::setWheelFunction(int function)
{
    function = qBound(0, function, 2);
    if (function == m_wheelFunction)
        return;
    m_wheelFunction = function;
    m_store.setValue(Keys::wheelFunction, function);
    emit wheelFunctionChanged();
}

void Settings::setDoubleClickFunction(int function)
{
    function = qBound(0, function, 2);
    if (function == m_doubleClickFunction)
        return;
    m_doubleClickFunction = function;
    m_store.setValue(Keys::doubleClickFunction, function);
    emit doubleClickFunctionChanged();
}

void Settings::setMiddleClickFunction(int function)
{
    function = qBound(0, function, 3);
    if (function == m_middleClickFunction)
        return;
    m_middleClickFunction = function;
    m_store.setValue(Keys::middleClickFunction, function);
    emit middleClickFunctionChanged();
}

void Settings::setAutoAddFolderFiles(bool autoAdd)
{
    if (autoAdd == m_autoAddFolderFiles)
        return;
    m_autoAddFolderFiles = autoAdd;
    m_store.setValue(Keys::autoAddFolderFiles, autoAdd);
    emit autoAddFolderFilesChanged();
}

void Settings::setAudioDevice(const QString &deviceId)
{
    if (deviceId == m_audioDevice)
        return;
    m_audioDevice = deviceId;
    m_store.setValue(Keys::audioDevice, deviceId);
    emit audioDeviceChanged();
}

void Settings::setRememberFileSettings(bool remember)
{
    if (remember == m_rememberFileSettings)
        return;
    m_rememberFileSettings = remember;
    m_store.setValue(Keys::rememberFileSettings, remember);
    emit rememberFileSettingsChanged();
}

void Settings::setRememberVolume(bool remember)
{
    if (remember == m_rememberVolume)
        return;
    m_rememberVolume = remember;
    m_store.setValue(Keys::rememberVolume, remember);
    emit rememberVolumeChanged();
}

void Settings::setInitialVolume(int volumePercent)
{
    volumePercent = qBound(0, volumePercent, 100);
    if (volumePercent == m_initialVolume)
        return;
    m_initialVolume = volumePercent;
    m_store.setValue(Keys::initialVolume, volumePercent);
    emit initialVolumeChanged();
}

void Settings::setOsdFontSize(int pixelSize)
{
    pixelSize = qBound(10, pixelSize, 60);
    if (pixelSize == m_osdFontSize)
        return;
    m_osdFontSize = pixelSize;
    m_store.setValue(Keys::osdFontSize, pixelSize);
    emit osdFontSizeChanged();
}

void Settings::setHideMouseInFullscreen(bool hide)
{
    if (hide == m_hideMouseInFullscreen)
        return;
    m_hideMouseInFullscreen = hide;
    m_store.setValue(Keys::hideMouseInFullscreen, hide);
    emit hideMouseInFullscreenChanged();
}

void Settings::setMouseHideDelay(int delayMs)
{
    delayMs = qBound(500, delayMs, 10000);
    if (delayMs == m_mouseHideDelay)
        return;
    m_mouseHideDelay = delayMs;
    m_store.setValue(Keys::mouseHideDelay, delayMs);
    emit mouseHideDelayChanged();
}

void Settings::setDvdUseMenus(bool use)
{
    if (use == m_dvdUseMenus)
        return;
    m_dvdUseMenus = use;
    m_store.setValue(Keys::dvdUseMenus, use);
    emit dvdUseMenusChanged();
}

void Settings::setDvdUseFirstPlay(bool use)
{
    if (use == m_dvdUseFirstPlay)
        return;
    m_dvdUseFirstPlay = use;
    m_store.setValue(Keys::dvdUseFirstPlay, use);
    emit dvdUseFirstPlayChanged();
}

void Settings::setDvdMenuTimeout(int seconds)
{
    seconds = qBound(0, seconds, 600);
    if (seconds == m_dvdMenuTimeout)
        return;
    m_dvdMenuTimeout = seconds;
    m_store.setValue(Keys::dvdMenuTimeout, seconds);
    emit dvdMenuTimeoutChanged();
}

void Settings::setLeftClickFunction(int function)
{
    function = qBound(0, function, 2);
    if (function == m_leftClickFunction)
        return;
    m_leftClickFunction = function;
    m_store.setValue(Keys::leftClickFunction, function);
    emit leftClickFunctionChanged();
}

void Settings::setAutoPlayNext(bool autoPlay)
{
    if (autoPlay == m_autoPlayNext)
        return;
    m_autoPlayNext = autoPlay;
    m_store.setValue(Keys::autoPlayNext, autoPlay);
    emit autoPlayNextChanged();
}

void Settings::setAddDirectoriesRecursively(bool recursive)
{
    if (recursive == m_addDirectoriesRecursively)
        return;
    m_addDirectoriesRecursively = recursive;
    m_store.setValue(Keys::addDirectoriesRecursively, recursive);
    emit addDirectoriesRecursivelyChanged();
}

void Settings::setStartInFullscreen(bool fullscreen)
{
    if (fullscreen == m_startInFullscreen)
        return;
    m_startInFullscreen = fullscreen;
    m_store.setValue(Keys::startInFullscreen, fullscreen);
    emit startInFullscreenChanged();
}

void Settings::setSubtitlesAutoload(bool autoload)
{
    if (autoload == m_subtitlesAutoload)
        return;
    m_subtitlesAutoload = autoload;
    m_store.setValue(Keys::subtitlesAutoload, autoload);
    emit subtitlesAutoloadChanged();
}

void Settings::setSubFontFamily(const QString &family)
{
    if (family == m_subFontFamily)
        return;
    m_subFontFamily = family;
    m_store.setValue(Keys::subFontFamily, family);
    emit subFontFamilyChanged();
}

void Settings::setSubFontSize(int pixelSize)
{
    pixelSize = qBound(10, pixelSize, 80);
    if (pixelSize == m_subFontSize)
        return;
    m_subFontSize = pixelSize;
    m_store.setValue(Keys::subFontSize, pixelSize);
    emit subFontSizeChanged();
}

void Settings::setSubPosition(int percentFromTop)
{
    percentFromTop = qBound(0, percentFromTop, 100);
    if (percentFromTop == m_subPosition)
        return;
    m_subPosition = percentFromTop;
    m_store.setValue(Keys::subPosition, percentFromTop);
    emit subPositionChanged();
}

void Settings::setPlayFilesFromStart(bool fromStart)
{
    if (fromStart == m_playFilesFromStart)
        return;
    m_playFilesFromStart = fromStart;
    m_store.setValue(Keys::playFilesFromStart, fromStart);
    emit playFilesFromStartChanged();
}

void Settings::setUiFontFamily(const QString &family)
{
    if (family == m_uiFontFamily)
        return;
    m_uiFontFamily = family;
    m_store.setValue(Keys::uiFontFamily, family);
    emit uiFontFamilyChanged();
}

void Settings::setUiFontSize(int pointSize)
{
    pointSize = pointSize <= 0 ? 0 : qBound(6, pointSize, 32);
    if (pointSize == m_uiFontSize)
        return;
    m_uiFontSize = pointSize;
    m_store.setValue(Keys::uiFontSize, pointSize);
    emit uiFontSizeChanged();
}
