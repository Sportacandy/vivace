/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Resolves a YouTube page URL to a directly-playable media URL on Android,
    where YoutubeResolver's own mechanism (spawn a downloaded yt-dlp/ffmpeg
    BINARY as a subprocess) cannot work at all -- Android forbids executing a
    native binary that wasn't shipped inside the signed APK. This class runs
    yt-dlp's own official pure-Python release (a plain zipapp -- data, not an
    executable) through an embedded CPython interpreter instead: the
    interpreter itself ships inside the APK (see CMakeLists.txt,
    VIVACE_HAVE_PYTHON_YOUTUBE, and https://github.com/Sportacandy/
    cpython-android-prebuilt), and it only ever *interprets* downloaded
    .py/.pyz bytes, never executes them as machine code -- Google's own
    Device and Network Abuse policy explicitly allows this ("code that runs
    in a virtual machine or an interpreter... Python, Lua, etc. loaded at
    run time must not allow potential violations of Google Play policies").

    Streaming mode (resolve()) works the same way on every platform this
    class is compiled for. Download & play mode (download()) additionally
    needs a real ffmpeg (to merge separate HD video+audio streams -- yt-dlp's
    own Python API, not a CLI subprocess, but ffmpeg itself is still a
    genuine external binary) and a real JS runtime for yt-dlp's own
    --js-runtimes mechanism (YouTube's signature/PoToken challenges). On
    Android, External tool mode still isn't offered (it fundamentally needs
    an arbitrary USER-supplied binary, which can never be bundled the way
    ffmpeg/node can) -- see PrefNetworkPage.qml's own Android-only mode
    restriction. VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS (CMakeLists.txt) gates
    whether download() is usable at all; see downloadModeSupported().

    PO (proof-of-origin) token support (VIVACE_HAVE_POT_PROVIDER,
    CMakeLists.txt): both resolve() and download() transparently pass a
    bundled, pre-built copy of the community "BgUtils POT Provider"
    project to yt-dlp (script_path extractor-arg + js_runtimes, resolved
    in importYoutubeDlClass()/addPotProviderExtractorArgs() in the .cpp)
    when available -- no user setup, unlike desktop's own YoutubeResolver::
    installOrUpdatePotProvider() button. See
    scripts/build-android-pot-provider.sh for how those assets are built
    at APK-build time (a real, empirically-necessary departure from
    upstream's own dependency versions, to run on this project's Node
    18-based Android JS runtime) and ensurePotProviderAssets() in the .cpp
    for how they're extracted/located at runtime.

    UNVERIFIED IN THIS SESSION beyond careful reading of CPython's own
    documented embedding API (Py_Initialize/PyGILState/zipimport), real,
    local compilation/ELF-header verification of the bundled ffmpeg/node
    tools (scripts/build-android-youtube-tools.sh), a REAL Android NDK
    build of this whole file (including every PO-token-provider addition)
    via the project's own Android_Qt_6_11_1_aarch64_v8a_Debug CMake
    preset, and -- for the PO token provider generation script
    specifically -- a real end-to-end run of the exact bundle scripts/
    build-android-pot-provider.sh produces, under a real Node v18.20.4
    binary (matching nodejs-mobile's own exact bundled version) on a
    desktop machine, generating a real, correctly-shaped PO token. What
    remains genuinely unverified: this environment has no Android device
    or emulator to confirm the SAME script actually runs correctly when
    spawned as a real subprocess by nodejs-mobile specifically (as opposed
    to a same-version desktop Node binary), or that the plugin/sys.path
    wiring behaves identically inside the real embedded-CPython-on-Android
    environment. Needs the user's own build+device test cycle, same
    other Android-native change in this project.
*/

#ifndef PYTHONYOUTUBERESOLVER_H
#define PYTHONYOUTUBERESOLVER_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

class PythonYoutubeResolver : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    // Preferred maximum video height (e.g. 720); 0 = no cap. Mirrors
    // YoutubeResolver's own property of the same name/meaning.
    Q_PROPERTY(int preferredHeight READ preferredHeight WRITE setPreferredHeight
                       NOTIFY preferredHeightChanged)
    // Optional cookies.txt path, forwarded to yt-dlp's own 'cookiefile'
    // option in download()'s Python call. Mirrors YoutubeResolver::
    // cookiesFile in name/meaning; bound from the SAME Settings.
    // youtubeCookiesFile value in Main.qml, since Preferences > Network >
    // YouTube's "Cookies file:" field is shared by both resolver types.
    Q_PROPERTY(QString cookiesFile READ cookiesFile WRITE setCookiesFile
                       NOTIFY cookiesFileChanged)
    // download()'s own output/cache folder. Bound from the SAME Settings.
    // youtubeCacheDir value YoutubeResolver's own cacheDir uses -- both
    // resolvers write into ONE shared folder using the identical
    // "<title> [<id>].<ext>" naming convention, so YoutubeResolver's
    // existing (purely filesystem-driven) cacheEntries()/cacheCount/
    // removeCacheEntry()/copyOrMoveToFolder() already work correctly for
    // files this class downloads too -- see noteExternalDownload() (called
    // from Main.qml's onDownloaded handler) for the one piece that can't
    // come for free (LRU eviction/cacheCount bookkeeping, which live on
    // YoutubeResolver as private state).
    Q_PROPERTY(QString cacheDir READ cacheDir WRITE setCacheDir NOTIFY cacheDirChanged)

public:
    explicit PythonYoutubeResolver(QObject *parent = nullptr);
    ~PythonYoutubeResolver() override;

    bool busy() const { return m_busy; }
    int preferredHeight() const { return m_preferredHeight; }
    void setPreferredHeight(int height);
    QString cookiesFile() const { return m_cookiesFile; }
    void setCookiesFile(const QString &path);
    QString cacheDir() const { return m_cacheDir; }
    void setCacheDir(const QString &dir);

    // True only on a build that both targets Android AND actually links the
    // embedded CPython interpreter (VIVACE_HAVE_PYTHON_YOUTUBE -- false for
    // an ABI cpython-android-prebuilt doesn't publish, e.g. armeabi-v7a/x86,
    // and for every non-Android platform, where this class exists but does
    // nothing). QML uses this to decide whether YouTube playback can be
    // offered on Android at all, rather than hard-coding "Android == yes".
    Q_INVOKABLE static bool isSupported();

    // Resolve pageUrl to a directly-playable stream URL. Runs the actual
    // Python call on a background QThread (network I/O + interpreter work
    // would otherwise block the GUI thread); emits resolved() or failed()
    // back on this object's own thread. A second resolve() call before the
    // first finishes supersedes it -- the first call's result is silently
    // dropped when it arrives (see m_generation), not literally cancelled
    // (an in-flight blocking urllib call inside the interpreter can't be
    // interrupted from outside it).
    Q_INVOKABLE void resolve(const QString &pageUrl);

    // Marks any in-flight resolve() as superseded, so its eventual result
    // (success or failure) is silently dropped instead of emitted. Does NOT
    // stop the underlying worker thread/Python call already in progress --
    // see resolve()'s own doc comment for why that can't be done cleanly.
    Q_INVOKABLE void cancel();

    // True only when this build bundles a real ffmpeg + JS runtime
    // (VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS) AND, on Android, they can
    // actually be located via ApplicationInfo.nativeLibraryDir at runtime
    // -- QML uses this to decide whether Download & play mode can be
    // offered at all, the same way isSupported() gates Streaming mode.
    Q_INVOKABLE static bool downloadModeSupported();

    // Downloads pageUrl's video (merging separate HD video+audio streams
    // via the bundled ffmpeg, same format-selector strategy as desktop's
    // YoutubeResolver::startDownload()) into cacheDir and emits downloaded()
    // with the final file's path, or failed() -- same threading/supersession
    // model as resolve() (see its own doc comment).
    Q_INVOKABLE void download(const QString &pageUrl);

    // Path installOrUpdate() writes to (also what resolve()'s own
    // ensureYtdlpDownloaded() reads from). Same name/shape as
    // YoutubeResolver::plannedInstallPath() so YoutubeSupportDialog.qml can
    // host either resolver type interchangeably (its own `resolver`
    // property is duck-typed as a plain QtObject for exactly this reason --
    // neither resolver class inherits from the other).
    Q_INVOKABLE QString plannedInstallPath() const;

    // Downloads yt-dlp's latest pure-Python release, OVERWRITING whatever
    // is already cached -- unlike resolve()'s own ensureYtdlpDownloaded(),
    // which downloads once and reuses the file forever after, this is the
    // explicit "Install / Update yt-dlp…" button's mechanism (the button
    // Vivace already has on desktop; Android needs its own working version
    // of it, since the desktop one downloads a native binary Android can't
    // execute at all). Async (QNetworkAccessManager on this object's own/
    // GUI thread, no nested QEventLoop) so installProgress reports real
    // progress instead of just spinning. Needs no VIVACE_HAVE_PYTHON_YOUTUBE
    // guard -- it's a plain HTTP download, unrelated to the embedded
    // interpreter itself.
    Q_INVOKABLE void installOrUpdate();

signals:
    void busyChanged();
    void preferredHeightChanged();
    void cookiesFileChanged();
    void cacheDirChanged();
    // mediaUrl is directly playable; title is the video title; pageUrl
    // echoes the original request. Same shape as YoutubeResolver::resolved,
    // so both can feed the same Main.qml onResolved handler.
    void resolved(const QUrl &mediaUrl, const QString &title, const QUrl &pageUrl);
    void failed(const QString &message);
    // Same shape as YoutubeResolver::downloaded, for the same QML-reuse
    // reason as resolved()'s own shape matching.
    void downloaded(const QUrl &fileUrl, const QString &title, const QUrl &pageUrl);

    // Same shapes as YoutubeResolver's own installProgress/installFinished/
    // installFailed, for the same QML-reuse reason as plannedInstallPath().
    void installProgress(qint64 received, qint64 total);
    void installFinished(const QString &path);
    void installFailed(const QString &message);

private:
    void setBusy(bool busy);
    // Runs on a background QThread (QThread::create), NOT this object's own
    // thread -- must not touch Qt properties directly; posts its result
    // back via a queued invokeMethod instead.
    void resolveOnWorkerThread(QString pageUrl, QString ytdlpPath, int preferredHeight,
                               QUrl originalPageUrl, qint64 generation);

    // Same threading contract as resolveOnWorkerThread(), for download().
    // A separate function (not an overload/extra params on
    // resolveOnWorkerThread()) since the two genuinely diverge -- an output
    // directory has no equivalent in the plain Streaming path. Resolves the
    // bundled ffmpeg/node tool paths ITSELF (rather than being handed them
    // by download()) -- purely a source-layout consequence, not a
    // correctness requirement: those helpers live in the same anonymous
    // namespace as runYtdlpDownload(), textually AFTER resolve()/download()
    // in the .cpp file, so download() itself can't call them directly. JNI
    // calls (androidNativeLibraryDir(), which these resolve through on
    // Android) are safe from any thread -- QJniObject/
    // QNativeInterface::QAndroidApplication handle JNIEnv attachment
    // automatically.
    void downloadOnWorkerThread(QString pageUrl, QString ytdlpPath, int preferredHeight,
                                 QString outDir, QString cookiesFile, QUrl originalPageUrl,
                                 qint64 generation);

    // Ensures yt-dlp's own pure-Python zipapp release is present in
    // app-private storage, downloading it (blocking) if not. Called from
    // resolve() on the CALLING (GUI) thread, before the worker thread is
    // spawned, so the one QNetworkAccessManager instance involved is only
    // ever touched from its own owning thread.
    bool ensureYtdlpDownloaded(QString *pathOut, QString *errorOut);

    bool m_busy = false;
    int m_preferredHeight = 720;
    QString m_cookiesFile;
    QString m_cacheDir;
    qint64 m_generation = 0;
    // Only used by installOrUpdate() -- resolve()'s own ensureYtdlpDownloaded()
    // creates its own short-lived manager per call instead (a blocking nested
    // QEventLoop can't share a manager across calls the way the async
    // installOrUpdate() flow naturally does).
    QNetworkAccessManager m_installNet;
};

#endif // PYTHONYOUTUBERESOLVER_H
