/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Python.h MUST be the very first include in this file, before
// pythonyoutuberesolver.h or any other Qt header. Real build failure
// (2026-09-14, real Android NDK build): Qt's <QObject> (pulled in
// transitively by this file's own .h) defines "slots" as a bare macro
// (expanding to nothing, unless QT_NO_KEYWORDS/Q_SLOTS is used) --
// CPython's own object.h uses "slots" as a plain C struct field name
// (PyType_Slot *slots;), which the preprocessor then mangled into
// "PyType_Slot *;", producing "error: expected member name or ';' after
// declaration specifiers". Python.h itself defines nothing that collides
// with Qt, so there's no corresponding risk the other way around --
// standard advice for any Qt+CPython-embedding translation unit.
#ifdef VIVACE_HAVE_PYTHON_YOUTUBE
// TEMPORARY desktop-test-build-only workaround (see CMakeLists.txt's own
// comment on VIVACE_CPYTHON_DESKTOP_HOME for why this whole branch exists):
// on MSVC, Python.h's own pyconfig.h auto-links against the DEBUG import
// lib (a `#pragma comment(lib, "python3XX_d.lib")`, triggered whenever
// _DEBUG is defined) -- but an ordinary python.org install only ships the
// release one, so a Debug build of Vivace itself failed with a real link
// error, "LNK1104: cannot open file 'python312_d.lib'". This is CPython's
// own documented gotcha for linking a Debug host app against a release
// Python; the standard workaround is undefining _DEBUG only around the
// Python.h include so its own linking pragma takes the release-lib path,
// then restoring it immediately after for the rest of this translation
// unit. Irrelevant on Android (no such Debug/Release CRT split there),
// hence scoped to _MSC_VER + VIVACE_CPYTHON_DESKTOP_HOME specifically.
#if defined(_MSC_VER) && defined(_DEBUG) && defined(VIVACE_CPYTHON_DESKTOP_HOME)
#define VIVACE_PY_UNDEF_DEBUG
#undef _DEBUG
#endif
#include <Python.h>
#ifdef VIVACE_PY_UNDEF_DEBUG
#define _DEBUG
#undef VIVACE_PY_UNDEF_DEBUG
#endif
#endif

#include "pythonyoutuberesolver.h"

#include <QDir>
#include <QDirIterator>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QThread>

#if defined(VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS) && defined(Q_OS_ANDROID)
#include <QCoreApplication> // QNativeInterface::QAndroidApplication
#include <QJniObject>
#endif

namespace {
Q_LOGGING_CATEGORY(lcPyYoutube, "vivace.pythonyoutube")

// Official yt-dlp release asset: a plain, platform-independent Python
// zipapp (a shebang line + real ZIP data -- Python's own zipimport reads
// straight past the shebang; confirmed directly against a real download of
// this exact file, "PK\x03\x04" starts right after "#!/usr/bin/env
// python3\n"). NOT the same asset YoutubeResolver's own installOrUpdate()
// downloads for other platforms (those are compiled per-platform binaries
// -- exactly what Android can't execute).
const char *const kYtdlpZipappUrl =
        "https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp";
} // namespace

PythonYoutubeResolver::PythonYoutubeResolver(QObject *parent)
    : QObject(parent)
{
}

PythonYoutubeResolver::~PythonYoutubeResolver() = default;

void PythonYoutubeResolver::setBusy(bool busy)
{
    if (busy == m_busy)
        return;
    m_busy = busy;
    emit busyChanged();
}

void PythonYoutubeResolver::setPreferredHeight(int height)
{
    if (height == m_preferredHeight)
        return;
    m_preferredHeight = height;
    emit preferredHeightChanged();
}

void PythonYoutubeResolver::setCookiesFile(const QString &path)
{
    if (path == m_cookiesFile)
        return;
    m_cookiesFile = path;
    emit cookiesFileChanged();
}

void PythonYoutubeResolver::setCacheDir(const QString &dir)
{
    if (dir == m_cacheDir)
        return;
    m_cacheDir = dir;
    emit cacheDirChanged();
}

bool PythonYoutubeResolver::isSupported()
{
//#ifdef VIVACE_HAVE_PYTHON_YOUTUBE
    return true;
// #else
//     return false;
// #endif
}

void PythonYoutubeResolver::cancel()
{
    ++m_generation; // any in-flight resolveOnWorkerThread() result is now stale
    setBusy(false);
}

QString PythonYoutubeResolver::plannedInstallPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            + QStringLiteral("/yt-dlp.pyz");
}

bool PythonYoutubeResolver::ensureYtdlpDownloaded(QString *pathOut, QString *errorOut)
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    const QString path = plannedInstallPath();

    if (QFileInfo::exists(path) && QFileInfo(path).size() > 0) {
        *pathOut = path;
        return true;
    }

    // One-time (or after a cache clear) blocking download on the calling
    // (GUI) thread -- a real UI-freeze risk on a slow connection, but a
    // pragmatic first-cut simplification: this only ever runs once per
    // install (every later resolve() hits the QFileInfo::exists() check
    // above and costs nothing). A nicer version would mirror
    // YoutubeResolver::installOrUpdate()'s own async progress-signal
    // design instead of a nested QEventLoop.
    QNetworkAccessManager net;
    QNetworkRequest req{QUrl(QString::fromLatin1(kYtdlpZipappUrl))};
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                      QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = net.get(req);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        *errorOut = reply->errorString();
        reply->deleteLater();
        return false;
    }

    const QByteArray data = reply->readAll();
    reply->deleteLater();
    if (data.isEmpty()) {
        *errorOut = QStringLiteral("Downloaded yt-dlp file was empty.");
        return false;
    }

    // Write to a temp name then rename, so a crash/interruption mid-download
    // never leaves a corrupt file at the real path that would pass the
    // exists()+size>0 check above on the next launch.
    const QString tmpPath = path + QStringLiteral(".part");
    {
        QFile f(tmpPath);
        if (!f.open(QIODevice::WriteOnly)) {
            *errorOut = QStringLiteral("Could not write yt-dlp to %1: %2")
                                .arg(tmpPath, f.errorString());
            return false;
        }
        f.write(data);
    }
    QFile::remove(path);
    if (!QFile::rename(tmpPath, path)) {
        *errorOut = QStringLiteral("Could not finalize the downloaded yt-dlp file.");
        return false;
    }

    *pathOut = path;
    return true;
}

void PythonYoutubeResolver::installOrUpdate()
{
    const QString path = plannedInstallPath();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QNetworkRequest req{QUrl(QString::fromLatin1(kYtdlpZipappUrl))};
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                      QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = m_installNet.get(req);
    QObject::connect(reply, &QNetworkReply::downloadProgress, this,
                      [this](qint64 received, qint64 total) {
                          emit installProgress(received, total);
                      });
    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, path]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit installFailed(reply->errorString());
            return;
        }
        const QByteArray data = reply->readAll();
        if (data.isEmpty()) {
            emit installFailed(QStringLiteral("Downloaded yt-dlp file was empty."));
            return;
        }
        // Write to a temp name then rename, same reasoning as
        // ensureYtdlpDownloaded(): a crash/interruption mid-write must
        // never leave a corrupt file at the real path.
        const QString tmpPath = path + QStringLiteral(".part");
        {
            QFile f(tmpPath);
            if (!f.open(QIODevice::WriteOnly)) {
                emit installFailed(QStringLiteral("Could not write yt-dlp to %1: %2")
                                            .arg(tmpPath, f.errorString()));
                return;
            }
            f.write(data);
        }
        QFile::remove(path);
        if (!QFile::rename(tmpPath, path)) {
            emit installFailed(QStringLiteral("Could not finalize the downloaded yt-dlp file."));
            return;
        }
        emit installFinished(path);
    });
}

void PythonYoutubeResolver::resolve(const QString &pageUrl)
{
    ++m_generation;
    const qint64 generation = m_generation;

#ifndef VIVACE_HAVE_PYTHON_YOUTUBE
    Q_UNUSED(pageUrl);
    Q_UNUSED(generation);
    emit failed(QStringLiteral("Embedded Python support is not available in this build."));
#else
    setBusy(true);

    QString ytdlpPath;
    QString error;
    if (!ensureYtdlpDownloaded(&ytdlpPath, &error)) {
        setBusy(false);
        if (generation == m_generation)
            emit failed(QStringLiteral("Could not get yt-dlp: %1").arg(error));
        return;
    }

    const QUrl originalPageUrl(pageUrl);
    const int height = m_preferredHeight;
    QThread *thread = QThread::create([this, pageUrl, ytdlpPath, height,
                                        originalPageUrl, generation]() {
        resolveOnWorkerThread(pageUrl, ytdlpPath, height, originalPageUrl, generation);
    });
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
#endif
}

bool PythonYoutubeResolver::downloadModeSupported()
{
#if defined(VIVACE_HAVE_PYTHON_YOUTUBE) && defined(VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS)
    return true;
#else
    return false;
#endif
}

void PythonYoutubeResolver::download(const QString &pageUrl)
{
    ++m_generation;
    const qint64 generation = m_generation;

#if !defined(VIVACE_HAVE_PYTHON_YOUTUBE) || !defined(VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS)
    Q_UNUSED(pageUrl);
    Q_UNUSED(generation);
    emit failed(QStringLiteral("Download & play is not available in this build."));
#else
    setBusy(true);

    QString ytdlpPath;
    QString error;
    if (!ensureYtdlpDownloaded(&ytdlpPath, &error)) {
        setBusy(false);
        if (generation == m_generation)
            emit failed(QStringLiteral("Could not get yt-dlp: %1").arg(error));
        return;
    }

    // Shares YoutubeResolver's own cacheDir (both bound from the SAME
    // Settings.youtubeCacheDir in Main.qml) and its exact
    // "<title> [<id>].<ext>" outtmpl naming -- so a video already there
    // (downloaded either by this class or by YoutubeResolver itself) is
    // detected by yt-dlp's own default "don't re-download an existing
    // destination file" behaviour, and YoutubeResolver's purely
    // filesystem-driven cacheEntries()/cacheCount/removeCacheEntry()/
    // copyOrMoveToFolder() already work correctly for files this class
    // writes -- see noteExternalDownload(), called from Main.qml's
    // onDownloaded handler, for the one piece (LRU eviction/cacheCount)
    // that needs an explicit nudge since that bookkeeping is private
    // state on YoutubeResolver.
    const QString outDir = m_cacheDir;
    QDir().mkpath(outDir);

    const QUrl originalPageUrl(pageUrl);
    const int height = m_preferredHeight;
    const QString cookiesFile = m_cookiesFile;
    QThread *thread = QThread::create([this, pageUrl, ytdlpPath, height, outDir, cookiesFile,
                                        originalPageUrl, generation]() {
        downloadOnWorkerThread(pageUrl, ytdlpPath, height, outDir, cookiesFile,
                                originalPageUrl, generation);
    });
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
#endif
}

#ifdef VIVACE_HAVE_PYTHON_YOUTUBE
namespace {

QMutex &pythonInitMutex()
{
    static QMutex m;
    return m;
}

// Extracts the whole android/assets/python-stdlib/ directory tree (a real
// recursive directory, bundled at build time by CMakeLists.txt) into a
// real, writable directory that the embedded interpreter's own
// (non-Qt-aware) file I/O can open directly -- Qt's "assets:/" virtual
// filesystem is a Qt-only abstraction, invisible to CPython's own
// fopen()-based module loader. Mirrors CPython's own official Android
// testbed's extractAssetDir() (MainActivity.kt, bundled inside this exact
// release tarball at Android/testbed/) almost exactly, just in C++/Qt
// instead of Kotlin -- Qt's Android asset file engine supports directory
// listing via QDirIterator, not just single-file access, so each file can
// be copied one at a time. Skipped entirely once a previous run's
// extraction is already present (a marker file, not re-scanning all
// ~2700 files every launch).
//
// Returns the directory ABOVE lib/python<ver>/, i.e. what PyConfig.home
// should point at (see ensurePythonInitialized() below) -- the same
// PYTHONHOME/lib/pythonX.Y/ layout a real CPython installation uses.
//
// The marker file's CONTENT is the exact bundled CPython release version
// (VIVACE_CPYTHON_VERSION), not just its existence -- a plain existence
// check would silently keep reusing a STALE extraction left over in
// app-private storage from an earlier install of a different build (app
// data survives an ordinary APK reinstall; only a full uninstall clears
// it) forever, since nothing else ever re-validates it. Real bug found
// 2026-09-15 this way: "ModuleNotFoundError: No module named
// 'zipfile._path'" on a device that had run an earlier build (from before
// this whole stdlib-extraction mechanism was finalized) -- the stdlib
// bundled in the CURRENT APK was correct the whole time, but the marker
// from that earlier, incomplete run made this function skip re-extracting
// it. A version bump doesn't guarantee the file SET changed, but it's a
// value that already changes exactly when the bundled stdlib might, with
// no extra bookkeeping needed.
// Copies one level of `assetDir` into `destDir`, recursing into
// subdirectories ITSELF (one QDir::entryInfoList() call per directory)
// rather than trusting QDirIterator::Subdirectories to walk the whole tree
// in one call. Real bug found 2026-09-15: a single QDirIterator(assetsRoot,
// QDir::Files, QDirIterator::Subdirectories) call correctly reached files
// ONE level deep under python-stdlib/ (encodings/*.py, lib-dynload/*.so,
// ssl.py -- everything needed to get as far as importing yt_dlp's own
// dependency on `ssl`) but silently never reached files TWO levels deep
// (zipfile/_path/__init__.py -- "ModuleNotFoundError: No module named
// 'zipfile._path'", reproducible even immediately after a version-tagged
// re-extraction, ruling out a stale-cache explanation) -- a known-flaky
// area of Qt's Android "assets:" file engine. Doing the recursion manually,
// one directory at a time, only ever relies on the SAME single-level
// listing primitive already proven to work (it's what produced the correct
// top-level split into encodings/, lib-dynload/, asyncio/, etc. in the
// first place), so it can't hit whatever depth limit Subdirectories itself
// has internally.
bool copyAssetTree(const QString &assetDir, const QString &destDir, int *countInOut,
                    QString *errorOut)
{
    const QFileInfoList entries = QDir(assetDir).entryInfoList(QDir::NoDotAndDotDot
                                                                | QDir::AllEntries);
    for (const QFileInfo &entry : entries) {
        const QString childAssetPath = assetDir + QLatin1Char('/') + entry.fileName();
        const QString outPath = destDir + QLatin1Char('/') + entry.fileName();
        if (entry.isDir()) {
            QDir().mkpath(outPath);
            if (!copyAssetTree(childAssetPath, outPath, countInOut, errorOut))
                return false;
            continue;
        }
        QFile::remove(outPath); // QFile::copy() refuses to overwrite an existing file
        if (!QFile::copy(childAssetPath, outPath)) {
            *errorOut = QStringLiteral("Could not extract %1 from APK assets")
                                .arg(childAssetPath);
            return false;
        }
        ++(*countInOut);
    }
    return true;
}

QString extractedStdlibHomeDir(QString *errorOut)
{
#ifdef VIVACE_CPYTHON_DESKTOP_HOME
    // TEMPORARY desktop-test build (see CMakeLists.txt's own comment on
    // this whole branch): a real, already-installed Python has its own
    // correct stdlib layout on disk already (Lib/ + DLLs/ on Windows,
    // lib/pythonX.Y/ + lib-dynload/ on POSIX) -- no Android-style asset
    // extraction needed at all. PyConfig.home (set from this same value in
    // ensurePythonInitialized() below) makes CPython's own normal
    // getpath.c calculation find it automatically, exactly like it would
    // for any ordinary desktop install.
    Q_UNUSED(errorOut);
    return QStringLiteral(VIVACE_CPYTHON_DESKTOP_HOME);
#else
    const QString destDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                             + QStringLiteral("/pyhome");
    const QString libDir = destDir + QStringLiteral("/lib/python" VIVACE_CPYTHON_ABI_VERSION);
    const QString markerPath = destDir + QStringLiteral("/.extracted");
    const QByteArray expectedMarker = QByteArrayLiteral(VIVACE_CPYTHON_VERSION);
    {
        QFile existingMarker(markerPath);
        if (existingMarker.open(QIODevice::ReadOnly)
            && existingMarker.readAll() == expectedMarker) {
            return destDir;
        }
    }

    QDir().mkpath(libDir);

    int count = 0;
    if (!copyAssetTree(QStringLiteral("assets:/python-stdlib"), libDir, &count, errorOut))
        return QString();
    if (count == 0) {
        *errorOut = QStringLiteral("python-stdlib assets missing from APK (0 files found)");
        return QString();
    }
    qWarning() << "PythonYoutubeResolver: extracted" << count
               << "stdlib files into" << libDir;

    QFile marker(markerPath);
    if (marker.open(QIODevice::WriteOnly))
        marker.write(expectedMarker);

    return destDir;
#endif // VIVACE_CPYTHON_DESKTOP_HOME
}

// Extracts android/assets/cacert.pem (bundled at build time, see
// CMakeLists.txt) to a real, writable file, so it can be pointed at via the
// SSL_CERT_FILE environment variable -- Android has no POSIX-visible system
// CA store at all (the trust store is exposed only through Java APIs), so
// without this the embedded interpreter's own OpenSSL fails every HTTPS
// connection with "unable to get local issuer certificate". Copied once;
// reused on every later call (a CA bundle changes far less often than the
// stdlib, so no version-tagged marker like extractedStdlibHomeDir()'s --
// just a plain existence check). Android-only -- unused (and unreferenced)
// entirely on the TEMPORARY desktop-test build, see its own call site.
#ifndef VIVACE_CPYTHON_DESKTOP_HOME
QString ensureCaCertBundle(QString *errorOut)
{
    const QString dest = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                          + QStringLiteral("/cacert.pem");
    if (QFileInfo::exists(dest) && QFileInfo(dest).size() > 0)
        return dest;

    QFile src(QStringLiteral("assets:/cacert.pem"));
    if (!src.exists()) {
        *errorOut = QStringLiteral("cacert.pem missing from APK assets");
        return QString();
    }
    QFile::remove(dest); // QFile::copy() refuses to overwrite an existing file
    if (!src.copy(dest)) {
        *errorOut = QStringLiteral("Could not extract the CA certificate bundle: %1")
                            .arg(src.errorString());
        return QString();
    }
    return dest;
}
#endif // VIVACE_CPYTHON_DESKTOP_HOME

#if defined(VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS)
#ifdef Q_OS_ANDROID
// Resolves ApplicationInfo.nativeLibraryDir via JNI -- the ONE directory an
// Android app is legally allowed to exec() a real native binary from
// (everything under it is part of the signed, installed package; app-
// PRIVATE storage like filesDir/cacheDir is not, which is the whole reason
// yt-dlp itself has to run through an embedded interpreter instead of a
// downloaded binary in the first place). libffmpeg.so/libvivace_node.so
// (see CMakeLists.txt's VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS block) are real
// executables bundled via QT_ANDROID_EXTRA_LIBS specifically so they land
// here. This is the first JNI call anywhere in Vivace's own code --
// QJniObject (Qt6::Core, since 6.1) and QNativeInterface::QAndroidApplication
// (since 6.2) are both real, documented, non-private Qt 6 APIs.
QString androidNativeLibraryDir()
{
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    if (!context.isValid())
        return QString();
    QJniObject appInfo = context.callObjectMethod(
            "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
    if (!appInfo.isValid())
        return QString();
    QJniObject dir = appInfo.getObjectField("nativeLibraryDir", "Ljava/lang/String;");
    return dir.toString();
}
#endif

// Resolves the bundled ffmpeg executable's real on-device path -- Android:
// ApplicationInfo.nativeLibraryDir + "/libffmpeg.so" (a real executable
// despite the name; see CMakeLists.txt's own comment on why it has to be
// named that way). TEMPORARY desktop-test build: the path baked in at
// compile time from VIVACE_YOUTUBE_TOOLS_DESKTOP_FFMPEG (a cache variable
// the developer sets, e.g. this project's own scripts/
// build-android-youtube-tools.sh output, or a system ffmpeg).
QString ffmpegToolPath(QString *errorOut)
{
#ifdef VIVACE_CPYTHON_DESKTOP_HOME
    return QStringLiteral(VIVACE_YOUTUBE_TOOLS_DESKTOP_FFMPEG);
#else
    const QString dir = androidNativeLibraryDir();
    if (dir.isEmpty()) {
        *errorOut = QStringLiteral("Could not resolve the app's native library directory");
        return QString();
    }
    return dir + QStringLiteral("/libffmpeg.so");
#endif
}

// Same as ffmpegToolPath(), for the bundled node wrapper
// (libvivace_node.so / VIVACE_YOUTUBE_TOOLS_DESKTOP_NODE).
QString nodeToolPath(QString *errorOut)
{
#ifdef VIVACE_CPYTHON_DESKTOP_HOME
    return QStringLiteral(VIVACE_YOUTUBE_TOOLS_DESKTOP_NODE);
#else
    const QString dir = androidNativeLibraryDir();
    if (dir.isEmpty()) {
        *errorOut = QStringLiteral("Could not resolve the app's native library directory");
        return QString();
    }
    return dir + QStringLiteral("/libvivace_node.so");
#endif
}
#endif // VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS

// Initializes the embedded interpreter exactly once per process.
//
// FIRST ATTEMPT (reverted 2026-09-15, a real bug found on a real device):
// PyConfig_InitIsolatedConfig + config.module_search_paths pointed directly
// at a single zipped stdlib file, no config.home at all -- consistently
// failed with "Py_InitializeFromConfig failed: Failed to import encodings
// module". Root-caused by reading CPython's own official Android CI
// harness (Android/testbed/, bundled inside this exact release tarball,
// see CMakeLists.txt's own comment for the full story): it uses
// PyConfig_InitPythonConfig (NOT isolated) and sets ONLY config.home,
// pointing at a real extracted directory tree, letting CPython's own
// normal getpath.c calculation find lib/python<ver>/ underneath -- exactly
// the PYTHONHOME convention a real desktop install uses. Since that's
// CPython's own tested, working recipe for this exact build, this now
// mirrors it instead of the zip/module_search_paths shortcut.
//
// use_environment/user_site_directory=0 and parse_argv=0 restore some of
// the isolation the earlier attempt had (no host env vars, no user
// site-packages, no attempt to parse the real OS argv, none of which makes
// sense for an interpreter embedded inside an Android app) without
// touching path CALCULATION itself, which is what needs to stay exactly
// like the official recipe for the encodings bootstrap to succeed.
//
// Per CPython's own documented embedding pattern ("Non-Python-created
// threads" in the C API docs), the calling thread holds the GIL right
// after Py_InitializeFromConfig() returns; PyEval_SaveThread() releases it
// immediately so ANY thread (including ones that didn't call this
// function) can later acquire it correctly via PyGILState_Ensure().
bool ensurePythonInitialized(QString *errorOut)
{
    static bool initialized = false;
    QMutexLocker locker(&pythonInitMutex());
    if (initialized)
        return true;

    // Python needs this to find its own temp directory; Android only sets
    // it automatically on API level 33+ (same reasoning/fix as CPython's
    // own official Android testbed, MainActivity.kt).
    if (!qEnvironmentVariableIsSet("TMPDIR")) {
        const QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        if (!tmp.isEmpty())
            qputenv("TMPDIR", tmp.toUtf8());
    }

#ifndef VIVACE_CPYTHON_DESKTOP_HOME
    // Read directly by OpenSSL's own X509_STORE_set_default_paths() --
    // independent of Python's config.use_environment=0 below, which only
    // governs env vars the INTERPRETER itself consults (PYTHONPATH etc.),
    // not third-party C libraries linked into the same process. Best-
    // effort: a missing/unextractable bundle only means HTTPS requests
    // will fail with a certificate-verify error, not that startup itself
    // should fail (some future network use might not even need TLS).
    // Skipped entirely on the TEMPORARY desktop-test build -- there's no
    // assets:/cacert.pem there at all (Android-only asset), and a real
    // desktop Python install already has working TLS via the OS's own
    // certificate store.
    if (!qEnvironmentVariableIsSet("SSL_CERT_FILE")) {
        QString certErr;
        const QString certPath = ensureCaCertBundle(&certErr);
        if (!certPath.isEmpty())
            qputenv("SSL_CERT_FILE", certPath.toUtf8());
        else
            qWarning() << "PythonYoutubeResolver: CA cert bundle unavailable:" << certErr;
    }
#endif

    const QString homeDir = extractedStdlibHomeDir(errorOut);
    if (homeDir.isEmpty())
        return false;

    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    config.site_import = 0;
    config.user_site_directory = 0;
    config.use_environment = 0;
    config.parse_argv = 0;
    config.write_bytecode = 0;

    // QString::toStdWString(), NOT a raw reinterpret_cast of utf16() data:
    // wchar_t is 2 bytes on Windows but 4 bytes on Android/Linux/macOS --
    // treating a UTF-16 (2-byte-unit) buffer as an array of 4-byte wchar_t
    // would silently corrupt the path on the very platform this code
    // actually targets. toStdWString() does the correct, platform-aware
    // conversion.
    const std::wstring homeDirW = homeDir.toStdWString();
    PyStatus status = PyConfig_SetString(&config, &config.home, homeDirW.c_str());
    if (PyStatus_Exception(status)) {
        *errorOut = QStringLiteral("PyConfig_SetString(home) failed");
        PyConfig_Clear(&config);
        return false;
    }

    status = Py_InitializeFromConfig(&config);
    PyConfig_Clear(&config);
    if (PyStatus_Exception(status)) {
        *errorOut = QStringLiteral("Py_InitializeFromConfig failed: %1")
                            .arg(status.err_msg ? QString::fromUtf8(status.err_msg)
                                                 : QStringLiteral("(no message)"));
        return false;
    }

#ifndef VIVACE_CPYTHON_DESKTOP_HOME
    // Redirect Python-level sys.stdout/sys.stderr to logcat. CPython's own
    // Android build ships _android_support.py specifically for this
    // (confirmed present in the extracted stdlib -- see
    // extractedStdlibHomeDir() above) -- normally wired up automatically by
    // site.py, which config.site_import=0 above disables, so it has to be
    // done by hand here instead. Without this, PyErr_Print()'s own
    // traceback output (runYtdlp()'s only diagnostic on every Python-side
    // failure) goes to the process's raw stderr fd, which is NOT reliably
    // captured by logcat for an embedded (non-testbed) app -- exactly the
    // gap that made a real "Could not import yt_dlp" failure impossible to
    // diagnose further without this. Best-effort: PyErr_Clear() below means
    // a failure here (e.g. a future stdlib without this module) only ever
    // costs debug visibility, never interpreter startup itself. Skipped on
    // the TEMPORARY desktop-test build -- _android_support/liblog.so are
    // Android-only, and formatCurrentPythonError() (used by runYtdlp() for
    // every Python-side failure) already gets the real traceback text
    // directly via PyErr_Fetch regardless of stdout/stderr redirection, so
    // nothing is lost by skipping this on desktop.
    //
    // _write is a thin Python wrapper around the raw ctypes function, NOT
    // the raw function passed directly to init_streams() -- real bug found
    // 2026-09-15 ("ctypes.ArgumentError: argument 2: TypeError: 'str'
    // object cannot be interpreted as ctypes.c_char_p", surfacing as a
    // seemingly unrelated "yt-dlp could not resolve this URL" failure,
    // since it happened the first time yt-dlp printed anything at all):
    // _android_support.py's own Logcat.write(prio, tag, message) calls
    // android_log_write(prio, tag, message) with `tag` still a plain
    // Python str (set verbatim in init_streams() as "python.stdout"/
    // "python.stderr") while `message` is already bytes (encoded upstream
    // in TextLogStream/BinaryLogStream) -- but argtypes=(c_int, c_char_p,
    // c_char_p) requires BOTH to already be bytes; ctypes' c_char_p never
    // auto-encodes a str. The wrapper encodes whichever argument is still
    // a str before forwarding to the real, argtypes-declared function.
    PyRun_SimpleString(
            "import ctypes as _ctypes, _android_support as _andsup\n"
            "_raw_write = _ctypes.CDLL('liblog.so').__android_log_write\n"
            "_raw_write.argtypes = (_ctypes.c_int, _ctypes.c_char_p, _ctypes.c_char_p)\n"
            "def _write(prio, tag, msg):\n"
            "    if isinstance(tag, str):\n"
            "        tag = tag.encode('utf-8')\n"
            "    if isinstance(msg, str):\n"
            "        msg = msg.encode('utf-8')\n"
            "    _raw_write(prio, tag, msg)\n"
            "_andsup.init_streams(_write, 4, 5)\n" // 4=ANDROID_LOG_INFO, 5=ANDROID_LOG_WARN
    );
    PyErr_Clear();
#endif

    PyEval_SaveThread(); // release the GIL; callers use PyGILState_Ensure() from here on
    initialized = true;
    return true;
}

// Formats the CURRENTLY-SET Python exception (caller must have already
// confirmed PyErr_Occurred()) as full traceback text and clears it --
// same information PyErr_Print() would write to Python's own sys.stderr,
// but returned directly instead, so callers can both log it through Qt's
// OWN logging (qWarning() -- routed by main.cpp's qInstallMessageHandler
// to vivace.log AND, via Qt's Android platform plugin, reliably to logcat
// regardless of whatever state Python-level stdout/stderr redirection is
// in) and surface it directly in the user-facing OSD error message. Falls
// back to a plain str(exception) if the `traceback` module import/call
// itself fails for any reason (never fails outright -- always returns
// SOME text, worst case a fixed placeholder).
QString formatCurrentPythonError()
{
    PyObject *excType = nullptr, *excValue = nullptr, *excTraceback = nullptr;
    PyErr_Fetch(&excType, &excValue, &excTraceback);
    PyErr_NormalizeException(&excType, &excValue, &excTraceback);

    QString result;
    if (PyObject *tbModule = PyImport_ImportModule("traceback")) {
        if (PyObject *formatFn = PyObject_GetAttrString(tbModule, "format_exception")) {
            PyObject *args = PyTuple_Pack(3, excType, excValue,
                                           excTraceback ? excTraceback : Py_None);
            if (PyObject *lines = PyObject_CallObject(formatFn, args)) {
                PyObject *sep = PyUnicode_FromString("");
                if (PyObject *joined = PyUnicode_Join(sep, lines)) {
                    result = QString::fromUtf8(PyUnicode_AsUTF8(joined)).trimmed();
                    Py_DECREF(joined);
                }
                Py_DECREF(sep);
                Py_DECREF(lines);
            } else {
                PyErr_Clear(); // formatting itself failed -- fall through to str() below
            }
            Py_DECREF(args);
            Py_DECREF(formatFn);
        }
        Py_DECREF(tbModule);
    }
    if (result.isEmpty() && excValue) {
        if (PyObject *str = PyObject_Str(excValue)) {
            result = QString::fromUtf8(PyUnicode_AsUTF8(str));
            Py_DECREF(str);
        }
    }

    Py_XDECREF(excType);
    Py_XDECREF(excValue);
    Py_XDECREF(excTraceback);
    return result.isEmpty() ? QStringLiteral("(unknown Python error)") : result;
}

// Shared by runYtdlp()/runYtdlpDownload(): puts ytdlpPath (the zipapp's own
// file path -- zipimport recognizes it as a zip archive purely from being
// on sys.path, per the module's own header comment on kYtdlpZipappUrl) on
// sys.path and returns a NEW reference to the yt_dlp.YoutubeDL class, or
// nullptr on failure (logs the full traceback via qWarning() and fills
// errorOut with a short message -- see formatCurrentPythonError()'s own
// doc comment for why that's more reliable here than PyErr_Print()).
PyObject *importYoutubeDlClass(const QString &ytdlpPath, QString *errorOut)
{
    PyObject *sysPath = PySys_GetObject("path"); // borrowed reference
    if (!sysPath) {
        *errorOut = QStringLiteral("sys.path unavailable");
        return nullptr;
    }
    PyObject *ytdlpPathObj = PyUnicode_FromString(ytdlpPath.toUtf8().constData());
    PyList_Insert(sysPath, 0, ytdlpPathObj); // does not steal the reference
    Py_DECREF(ytdlpPathObj);

    PyObject *ytDlpModule = PyImport_ImportModule("yt_dlp");
    if (!ytDlpModule) {
        const QString tb = formatCurrentPythonError();
        qWarning().noquote() << "PythonYoutubeResolver: could not import yt_dlp:\n" << tb;
        *errorOut = QStringLiteral("Could not import yt_dlp: %1")
                            .arg(tb.section(QLatin1Char('\n'), -1, -1));
        return nullptr;
    }

    PyObject *ytDlpClass = PyObject_GetAttrString(ytDlpModule, "YoutubeDL");
    Py_DECREF(ytDlpModule);
    if (!ytDlpClass) {
        const QString tb = formatCurrentPythonError();
        qWarning().noquote() << "PythonYoutubeResolver: yt_dlp.YoutubeDL not found:\n" << tb;
        *errorOut = QStringLiteral("yt_dlp.YoutubeDL not found: %1")
                            .arg(tb.section(QLatin1Char('\n'), -1, -1));
        return nullptr;
    }
    return ytDlpClass;
}

// Shared by runYtdlp()/runYtdlpDownload(): constructs yt_dlp.YoutubeDL(opts)
// (stealing/consuming ytDlpClass's own reference either way) and returns a
// NEW reference to the instance, or nullptr on failure.
PyObject *constructYoutubeDl(PyObject *ytDlpClass, PyObject *opts, QString *errorOut)
{
    PyObject *ctorArgs = PyTuple_Pack(1, opts);
    PyObject *ydl = PyObject_CallObject(ytDlpClass, ctorArgs);
    Py_DECREF(ctorArgs);
    Py_DECREF(opts);
    Py_DECREF(ytDlpClass);
    if (!ydl) {
        const QString tb = formatCurrentPythonError();
        qWarning().noquote() << "PythonYoutubeResolver: could not construct YoutubeDL:\n" << tb;
        *errorOut = QStringLiteral("Could not construct YoutubeDL: %1")
                            .arg(tb.section(QLatin1Char('\n'), -1, -1));
        return nullptr;
    }
    return ydl;
}

// Runs entirely while holding the GIL (caller's responsibility). Returns
// true and fills mediaUrl/title on success; on failure, logs the full
// Python traceback via qWarning() and fills errorOut with a short,
// user-facing message that also includes the traceback's own final line
// (the actual exception type/message) directly, so the OSD toast alone is
// informative without needing a log at all.
bool runYtdlp(const QString &ytdlpPath, const QString &pageUrl, int preferredHeight,
              QString *mediaUrlOut, QString *titleOut, QString *errorOut)
{
    PyObject *ytDlpClass = importYoutubeDlClass(ytdlpPath, errorOut);
    if (!ytDlpClass)
        return false;

    // Options mirror YoutubeResolver::startResolve()'s own yt-dlp CLI
    // invocation: a single muxed/progressive format capped at
    // preferredHeight (QMediaPlayer can't merge separate DASH streams), no
    // playlist expansion, quiet (the result is read back programmatically,
    // not parsed from stdout the way the subprocess-based resolver does).
    PyObject *opts = PyDict_New();
    const QString format = preferredHeight > 0
            ? QStringLiteral("best[height<=?%1]/best").arg(preferredHeight)
            : QStringLiteral("best");
    PyObject *formatVal = PyUnicode_FromString(format.toUtf8().constData());
    PyDict_SetItemString(opts, "format", formatVal); // does not steal the reference
    Py_DECREF(formatVal);
    PyDict_SetItemString(opts, "noplaylist", Py_True);
    PyDict_SetItemString(opts, "quiet", Py_True);
    PyDict_SetItemString(opts, "no_warnings", Py_True);

    PyObject *ydl = constructYoutubeDl(ytDlpClass, opts, errorOut);
    if (!ydl)
        return false;

    PyObject *extractInfo = PyObject_GetAttrString(ydl, "extract_info");
    Py_DECREF(ydl);
    if (!extractInfo) {
        const QString tb = formatCurrentPythonError();
        qWarning().noquote() << "PythonYoutubeResolver: YoutubeDL.extract_info not found:\n" << tb;
        *errorOut = QStringLiteral("YoutubeDL.extract_info not found: %1")
                            .arg(tb.section(QLatin1Char('\n'), -1, -1));
        return false;
    }

    PyObject *urlArg = PyUnicode_FromString(pageUrl.toUtf8().constData());
    PyObject *extractArgs = PyTuple_Pack(1, urlArg);
    Py_DECREF(urlArg);
    PyObject *kwargs = PyDict_New();
    PyDict_SetItemString(kwargs, "download", Py_False);

    PyObject *info = PyObject_Call(extractInfo, extractArgs, kwargs);
    Py_DECREF(extractInfo);
    Py_DECREF(extractArgs);
    Py_DECREF(kwargs);

    if (!info) {
        const QString tb = formatCurrentPythonError();
        qWarning().noquote() << "PythonYoutubeResolver: extract_info failed:\n" << tb;
        *errorOut = QStringLiteral("yt-dlp could not resolve this URL: %1")
                            .arg(tb.section(QLatin1Char('\n'), -1, -1));
        return false;
    }

    PyObject *urlObj = PyDict_GetItemString(info, "url"); // borrowed
    PyObject *titleObj = PyDict_GetItemString(info, "title"); // borrowed
    if (urlObj && PyUnicode_Check(urlObj))
        *mediaUrlOut = QString::fromUtf8(PyUnicode_AsUTF8(urlObj));
    if (titleObj && PyUnicode_Check(titleObj))
        *titleOut = QString::fromUtf8(PyUnicode_AsUTF8(titleObj));
    Py_DECREF(info);

    if (mediaUrlOut->isEmpty()) {
        *errorOut = QStringLiteral(
                "yt-dlp returned no direct URL for this video (it may need "
                "a separate video+audio merge, which isn't supported here)");
        return false;
    }
    return true;
}

#if defined(VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS)
// Runs entirely while holding the GIL (caller's responsibility). Downloads
// pageUrl's video to outDir, merging separate HD video+audio streams via
// the bundled ffmpeg (ffmpegPath) -- the SAME format-selector strategy as
// desktop's YoutubeResolver::startDownload() (prefer avc1+m4a, a clean
// remux any ffmpeg handles; fall back to best+best for >1080p/AV1-only
// content). nodePath enables yt-dlp's own --js-runtimes mechanism
// (js_runtimes dict, confirmed against yt_dlp.YoutubeDL's own docstring --
// NOT the same shape as the CLI's list-of-"runtime:path"-strings form) for
// YouTube's signature/PoToken challenges, since Deno isn't available on
// Android (see this class's own header comment for the full story).
// cookiesFile (optional) unlocks HD/members-only/age-restricted videos,
// same as desktop's own cookies field -- safe here for the same reason
// desktop's HelpMark gives: cookies only affect the download, not a
// stream a player must open.
bool runYtdlpDownload(const QString &ytdlpPath, const QString &pageUrl, int preferredHeight,
                      const QString &ffmpegPath, const QString &nodePath, const QString &outDir,
                      const QString &cookiesFile, QString *filePathOut, QString *titleOut,
                      QString *errorOut)
{
    PyObject *ytDlpClass = importYoutubeDlClass(ytdlpPath, errorOut);
    if (!ytDlpClass)
        return false;

    PyObject *opts = PyDict_New();
    QString format = QStringLiteral(
            "bestvideo[vcodec^=avc1]+bestaudio[ext=m4a]/bestvideo+bestaudio/best");
    if (preferredHeight > 0)
        format = QStringLiteral(
                "bestvideo[height<=?%1][vcodec^=avc1]+bestaudio[ext=m4a]/"
                "bestvideo[height<=?%1]+bestaudio/best[height<=?%1]/best")
                .arg(preferredHeight);
    PyObject *formatVal = PyUnicode_FromString(format.toUtf8().constData());
    PyDict_SetItemString(opts, "format", formatVal);
    Py_DECREF(formatVal);
    PyObject *mergeFmtVal = PyUnicode_FromString("mp4");
    PyDict_SetItemString(opts, "merge_output_format", mergeFmtVal);
    Py_DECREF(mergeFmtVal);
    PyObject *outtmplVal = PyUnicode_FromString(
            (outDir + QStringLiteral("/%(title).100B [%(id)s].%(ext)s")).toUtf8().constData());
    PyDict_SetItemString(opts, "outtmpl", outtmplVal);
    Py_DECREF(outtmplVal);
    PyObject *ffmpegLocVal = PyUnicode_FromString(ffmpegPath.toUtf8().constData());
    PyDict_SetItemString(opts, "ffmpeg_location", ffmpegLocVal);
    Py_DECREF(ffmpegLocVal);
    // js_runtimes = {'node': {'path': nodePath}} -- a dict of dicts, per
    // YoutubeDL.py's own docstring (NOT the CLI's "runtime:path" string
    // list form -- that parsing happens in options.py, only relevant to
    // the yt-dlp CLI entry point, which this embedded-interpreter path
    // never goes through). Naming ONLY "node" here (no "deno" key at all)
    // is what disables yt-dlp's own default-enabled "deno" runtime --
    // Deno has no Android build, so it would just fail to launch on every
    // attempt if left enabled instead of replaced.
    PyObject *nodeRuntimeOpts = PyDict_New();
    PyObject *nodePathVal = PyUnicode_FromString(nodePath.toUtf8().constData());
    PyDict_SetItemString(nodeRuntimeOpts, "path", nodePathVal);
    Py_DECREF(nodePathVal);
    PyObject *jsRuntimes = PyDict_New();
    PyDict_SetItemString(jsRuntimes, "node", nodeRuntimeOpts);
    Py_DECREF(nodeRuntimeOpts);
    PyDict_SetItemString(opts, "js_runtimes", jsRuntimes);
    Py_DECREF(jsRuntimes);
    PyDict_SetItemString(opts, "noplaylist", Py_True);
    PyDict_SetItemString(opts, "quiet", Py_True);
    PyDict_SetItemString(opts, "no_warnings", Py_True);
    // Same key names as yt-dlp's own CLI options.py (dest='cookiefile',
    // dest='writethumbnail'/'convertthumbnails') -- confirmed against the
    // real installed yt_dlp source, not guessed. The poster thumbnail lets
    // YoutubeResolver::cacheEntries() (which this class's downloads are
    // now visible to -- see the cacheDir property's own doc comment) show
    // a real preview in the cache browser instead of a blank cell; no
    // black-frame-detection fallback is done here (that's YoutubeResolver-
    // internal, private, C++-side logic) -- an occasional black poster is
    // an accepted, minor gap versus desktop's own fuller mechanism.
    if (!cookiesFile.isEmpty()) {
        PyObject *cookiesVal = PyUnicode_FromString(cookiesFile.toUtf8().constData());
        PyDict_SetItemString(opts, "cookiefile", cookiesVal);
        Py_DECREF(cookiesVal);
    }
    PyDict_SetItemString(opts, "writethumbnail", Py_True);
    PyObject *convertThumbsVal = PyUnicode_FromString("jpg");
    PyDict_SetItemString(opts, "convertthumbnails", convertThumbsVal);
    Py_DECREF(convertThumbsVal);

    PyObject *ydl = constructYoutubeDl(ytDlpClass, opts, errorOut);
    if (!ydl)
        return false;

    PyObject *extractInfo = PyObject_GetAttrString(ydl, "extract_info");
    Py_DECREF(ydl);
    if (!extractInfo) {
        const QString tb = formatCurrentPythonError();
        qWarning().noquote() << "PythonYoutubeResolver: YoutubeDL.extract_info not found:\n" << tb;
        *errorOut = QStringLiteral("YoutubeDL.extract_info not found: %1")
                            .arg(tb.section(QLatin1Char('\n'), -1, -1));
        return false;
    }

    PyObject *urlArg = PyUnicode_FromString(pageUrl.toUtf8().constData());
    PyObject *extractArgs = PyTuple_Pack(1, urlArg);
    Py_DECREF(urlArg);
    PyObject *kwargs = PyDict_New();
    PyDict_SetItemString(kwargs, "download", Py_True);

    PyObject *info = PyObject_Call(extractInfo, extractArgs, kwargs);
    Py_DECREF(extractInfo);
    Py_DECREF(extractArgs);
    Py_DECREF(kwargs);

    if (!info) {
        const QString tb = formatCurrentPythonError();
        qWarning().noquote() << "PythonYoutubeResolver: extract_info(download=True) failed:\n" << tb;
        *errorOut = QStringLiteral("yt-dlp could not download this video: %1")
                            .arg(tb.section(QLatin1Char('\n'), -1, -1));
        return false;
    }

    // CORRECTED (2026-09-19, real on-device evidence): the top-level dict
    // 'filepath' key this used to read here is NEVER actually set for a
    // real download -- confirmed directly against YoutubeDL.py's own
    // process_video_result(): its download loop always calls
    // process_info() on a COPY of info_dict (new_info), never on
    // info_dict itself, and the loop's only write-back to the top-level
    // dict at the end (info_dict.update(best_format)) copies plain format
    // metadata, not any per-format copy's 'filepath'. This was wrong for
    // EVERY download, not just the video+audio-merge case -- it happened
    // to go unnoticed until a real merge (two separate downloads, seen in
    // logcat) exposed it as "yt-dlp did not report a downloaded file
    // path" despite both streams downloading to 100%.
    //
    // The real final (post-merge) path lives on each entry of
    // 'requested_downloads' instead -- confirmed against
    // FFmpegMergerPP.run() itself: it renames its merged output onto
    // exactly info['filepath'] on the SAME per-format dict this list
    // holds (info_dict['requested_downloads'] = downloaded_formats, each
    // element one of those new_info copies) -- so this is the correct
    // key regardless of whether a merge happened at all (a plain
    // single-format download also produces exactly one entry here, with
    // its own already-correct 'filepath').
    PyObject *titleObj = PyDict_GetItemString(info, "title"); // borrowed
    if (titleObj && PyUnicode_Check(titleObj))
        *titleOut = QString::fromUtf8(PyUnicode_AsUTF8(titleObj));

    PyObject *requestedDownloads = PyDict_GetItemString(info, "requested_downloads"); // borrowed
    if (requestedDownloads && PyList_Check(requestedDownloads) && PyList_Size(requestedDownloads) > 0) {
        PyObject *lastEntry = PyList_GetItem( // borrowed
                requestedDownloads, PyList_Size(requestedDownloads) - 1);
        if (lastEntry && PyDict_Check(lastEntry)) {
            PyObject *entryPath = PyDict_GetItemString(lastEntry, "filepath"); // borrowed
            if (entryPath && PyUnicode_Check(entryPath))
                *filePathOut = QString::fromUtf8(PyUnicode_AsUTF8(entryPath));
        }
    }
    Py_DECREF(info);

    if (filePathOut->isEmpty()) {
        *errorOut = QStringLiteral("yt-dlp did not report a downloaded file path");
        return false;
    }
    // A merge that never actually ran (e.g. ffmpeg not recognized as
    // usable by yt-dlp's own probe -- it silently skips merging rather
    // than raising an error in that case) still reports the WOULD-BE
    // merged path here without ever creating it, since that rename only
    // happens inside a successful FFmpegMergerPP.run(); catch that
    // explicitly with a message that actually points at the cause,
    // rather than the player failing to open a nonexistent file with no
    // context.
    if (!QFile::exists(*filePathOut)) {
        *errorOut = QStringLiteral(
                "yt-dlp reported a downloaded file that does not exist "
                "(the video and audio likely downloaded separately but could "
                "not be merged -- check that the bundled ffmpeg is usable)");
        filePathOut->clear();
        return false;
    }
    return true;
}
#endif // VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS

} // namespace
#endif // VIVACE_HAVE_PYTHON_YOUTUBE

void PythonYoutubeResolver::resolveOnWorkerThread(QString pageUrl, QString ytdlpPath,
                                                   int preferredHeight,
                                                   QUrl originalPageUrl,
                                                   qint64 generation)
{
#ifdef VIVACE_HAVE_PYTHON_YOUTUBE
    QString error;
    QString mediaUrl;
    QString title;
    bool ok;

    if (!ensurePythonInitialized(&error)) {
        ok = false;
    } else {
        PyGILState_STATE gstate = PyGILState_Ensure();
        ok = runYtdlp(ytdlpPath, pageUrl, preferredHeight, &mediaUrl, &title, &error);
        PyGILState_Release(gstate);
    }

    if (!ok)
        qCWarning(lcPyYoutube) << "resolve failed:" << error;

    // Marshal back onto this object's own (GUI) thread -- 'this' lives
    // there, not on this worker thread, so touching m_generation/m_busy or
    // emitting signals directly here would be a cross-thread race.
    QMetaObject::invokeMethod(this, [this, generation, ok, mediaUrl, title,
                                      originalPageUrl, error]() {
        if (generation != m_generation)
            return; // superseded by a newer resolve()/cancel() -- drop silently
        setBusy(false);
        if (ok)
            emit resolved(QUrl(mediaUrl), title, originalPageUrl);
        else
            emit failed(error);
    }, Qt::QueuedConnection);
#else
    Q_UNUSED(pageUrl);
    Q_UNUSED(ytdlpPath);
    Q_UNUSED(preferredHeight);
    Q_UNUSED(originalPageUrl);
    Q_UNUSED(generation);
#endif
}

void PythonYoutubeResolver::downloadOnWorkerThread(QString pageUrl, QString ytdlpPath,
                                                    int preferredHeight, QString outDir,
                                                    QString cookiesFile, QUrl originalPageUrl,
                                                    qint64 generation)
{
#if defined(VIVACE_HAVE_PYTHON_YOUTUBE) && defined(VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS)
    QString error;
    QString filePath;
    QString title;
    bool ok;

    const QString ffmpegPath = ffmpegToolPath(&error);
    const QString nodePath = ffmpegPath.isEmpty() ? QString() : nodeToolPath(&error);
    if (ffmpegPath.isEmpty() || nodePath.isEmpty()) {
        ok = false;
    } else if (!ensurePythonInitialized(&error)) {
        ok = false;
    } else {
        PyGILState_STATE gstate = PyGILState_Ensure();
        ok = runYtdlpDownload(ytdlpPath, pageUrl, preferredHeight, ffmpegPath, nodePath, outDir,
                              cookiesFile, &filePath, &title, &error);
        PyGILState_Release(gstate);
    }

    if (!ok)
        qCWarning(lcPyYoutube) << "download failed:" << error;

    QMetaObject::invokeMethod(this, [this, generation, ok, filePath, title,
                                      originalPageUrl, error]() {
        if (generation != m_generation)
            return; // superseded by a newer resolve()/download()/cancel() -- drop silently
        setBusy(false);
        if (ok)
            emit downloaded(QUrl::fromLocalFile(filePath), title, originalPageUrl);
        else
            emit failed(error);
    }, Qt::QueuedConnection);
#else
    Q_UNUSED(pageUrl);
    Q_UNUSED(ytdlpPath);
    Q_UNUSED(preferredHeight);
    Q_UNUSED(outDir);
    Q_UNUSED(cookiesFile);
    Q_UNUSED(originalPageUrl);
    Q_UNUSED(generation);
#endif
}
