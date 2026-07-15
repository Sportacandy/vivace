/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <cstdio>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QGuiApplication>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QSettings>
#include <QTextStream>
#include <QTranslator>
#include <QUrl>
#include <QVariant>

#include "singleinstance.h"

namespace {

// Persistent troubleshooting log. A WIN32 GUI app has no usable stderr, so
// Qt/QML warnings and errors are otherwise invisible in a shipped build; this
// mirrors them to a file (next to the settings, i.e. the folder Preferences >
// Advanced opens as vivace.log). Kept in every build config on purpose.
QtMessageHandler g_previousHandler = nullptr;
QString g_logFilePath;

void fileMessageHandler(QtMsgType type, const QMessageLogContext &context,
                        const QString &message)
{
    // Chain to the default handler so nothing is lost in dev (IDE / console /
    // OutputDebugString still get every message).
    if (g_previousHandler)
        g_previousHandler(type, context, message);

    // Persist warnings and errors only — the signal for troubleshooting,
    // without the volume of qDebug/qInfo (e.g. DVD navigation traces).
    const char *level = nullptr;
    switch (type) {
    case QtWarningMsg:  level = "WARN"; break;
    case QtCriticalMsg: level = "CRIT"; break;
    case QtFatalMsg:    level = "FATAL"; break;
    default: return; // Debug / Info are not persisted
    }
    if (g_logFilePath.isEmpty())
        return;
    QFile file(g_logFilePath);
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return;
    QTextStream out(&file);
    out << QDateTime::currentDateTime().toString(Qt::ISODate)
        << " [" << level << "] ";
    if (context.category && qstrcmp(context.category, "default") != 0)
        out << context.category << ": ";
    out << message << '\n';
}

// Install the file logger. Rotates once when the log passes ~1 MB, so it never
// grows unbounded while the previous session (e.g. before a crash) survives as
// vivace.log.1.
void installFileLogger()
{
    // Same folder UiHelpers::configFolderUrl() exposes, so the existing
    // "config folder" link in Preferences > Advanced reveals the log too.
    const QSettings probe(QSettings::IniFormat, QSettings::UserScope,
                          QStringLiteral("vivace-player"),
                          QStringLiteral("vivace_files"));
    const QString dir = QFileInfo(probe.fileName()).absolutePath();
    QDir().mkpath(dir);
    g_logFilePath = dir + QStringLiteral("/vivace.log");

    const QFileInfo info(g_logFilePath);
    if (info.exists() && info.size() > 1024 * 1024) {
        QFile::remove(g_logFilePath + QStringLiteral(".1"));
        QFile::rename(g_logFilePath, g_logFilePath + QStringLiteral(".1"));
    }
    g_previousHandler = qInstallMessageHandler(fileMessageHandler);
}

// Parse a "start" time given as h:m:s, m:s or plain seconds into milliseconds.
qint64 parseTimeToMs(const QString &text)
{
    const QStringList parts = text.split(u':');
    qint64 seconds = 0;
    for (const QString &p : parts)
        seconds = seconds * 60 + p.toLongLong();
    return seconds * 1000;
}

// Vivace's command-line options (GNU long options, with short aliases where
// sensible). Defined once at file scope so the parse path and the --help text
// stay in sync.
const QCommandLineOption kCliFullscreen(
        { QStringLiteral("f"), QStringLiteral("fullscreen") },
        QStringLiteral("Start in fullscreen."));
const QCommandLineOption kCliOntop(
        { QStringLiteral("t"), QStringLiteral("ontop") },
        QStringLiteral("Keep the window above other windows."));
const QCommandLineOption kCliCloseAtEnd(
        { QStringLiteral("c"), QStringLiteral("close-at-end") },
        QStringLiteral("Close Vivace when playback ends."));
const QCommandLineOption kCliSub(
        { QStringLiteral("s"), QStringLiteral("sub") },
        QStringLiteral("Load the subtitle <file>."), QStringLiteral("file"));
const QCommandLineOption kCliPos(
        QStringLiteral("pos"),
        QStringLiteral("Window position as <x,y>."), QStringLiteral("x,y"));
const QCommandLineOption kCliSize(
        QStringLiteral("size"),
        QStringLiteral("Window size as <w,h>."), QStringLiteral("w,h"));
const QCommandLineOption kCliStart(
        { QStringLiteral("b"), QStringLiteral("start") },
        QStringLiteral("Start playback at <time> (h:m:s, m:s or seconds)."),
        QStringLiteral("time"));

// Configure `parser` with Vivace's description, --help/--version and the full
// option set. Shared by parseVivaceArgs() and the --help/--version handler.
void configureCliParser(QCommandLineParser &parser)
{
    // ASCII hyphen (not an em-dash) so the console codepage renders it cleanly.
    parser.setApplicationDescription(
            QStringLiteral("Vivace - a fast, pure-Qt media player."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOptions({ kCliFullscreen, kCliOntop, kCliCloseAtEnd, kCliSub,
                        kCliPos, kCliSize, kCliStart });
    parser.addPositionalArgument(QStringLiteral("media"),
                                 QStringLiteral("File or URL to play."),
                                 QStringLiteral("[media]"));
}

// Parse `userArgs` (program name excluded) into a map applied by Main.qml
// (applyStartupOptions). Unrecognised input is ignored (parse, not process) —
// a GUI app has no console for the auto --help/error output; --help/--version
// are handled separately, up front, by handleCliInfoRequest().
QVariantMap parseVivaceArgs(const QStringList &userArgs)
{
    QCommandLineParser parser;
    configureCliParser(parser);
    parser.parse(QStringList{ QStringLiteral("vivace") } + userArgs);

    QVariantMap m;
    m[QStringLiteral("fullscreen")] = parser.isSet(kCliFullscreen);
    m[QStringLiteral("ontop")] = parser.isSet(kCliOntop);
    m[QStringLiteral("closeAtEnd")] = parser.isSet(kCliCloseAtEnd);
    if (parser.isSet(kCliSub)) {
        const QFileInfo info(parser.value(kCliSub));
        if (info.exists())
            m[QStringLiteral("subtitle")] =
                    QUrl::fromLocalFile(info.absoluteFilePath()).toString();
    }
    if (parser.isSet(kCliStart))
        m[QStringLiteral("startMs")] = parseTimeToMs(parser.value(kCliStart));
    if (parser.isSet(kCliPos)) {
        const QStringList xy = parser.value(kCliPos).split(u',');
        bool okX = false, okY = false;
        if (xy.size() == 2) {
            const int x = xy[0].trimmed().toInt(&okX);
            const int y = xy[1].trimmed().toInt(&okY);
            if (okX && okY) {
                m[QStringLiteral("hasPos")] = true;
                m[QStringLiteral("x")] = x;
                m[QStringLiteral("y")] = y;
            }
        }
    }
    if (parser.isSet(kCliSize)) {
        const QStringList wh = parser.value(kCliSize).split(u',');
        bool okW = false, okH = false;
        if (wh.size() == 2) {
            const int w = wh[0].trimmed().toInt(&okW);
            const int h = wh[1].trimmed().toInt(&okH);
            if (okW && okH && w > 0 && h > 0) {
                m[QStringLiteral("hasSize")] = true;
                m[QStringLiteral("width")] = w;
                m[QStringLiteral("height")] = h;
            }
        }
    }
    const QStringList positional = parser.positionalArguments();
    if (!positional.isEmpty())
        m[QStringLiteral("source")] =
                QUrl::fromUserInput(positional.first(), QDir::currentPath())
                        .toString();
    return m;
}

} // namespace

#ifdef Q_OS_WIN
#include <windows.h>
#include <shobjidl.h>
#pragma comment(lib, "shell32.lib")

// Give the process a stable Application User Model ID (before any UI). This is
// the runtime half of the app's identity: it groups the taskbar button and
// tags the System Media Transport Controls session. The friendly name and icon
// the SMTC/media flyout shows for this AUMID are resolved by Windows from a
// Start Menu shortcut carrying System.AppUserModel.ID == this AUMID — an
// install-time artifact — so an uninstalled/dev build still shows "unknown
// app". Creating that shortcut is the installer's job (Phase 6 packaging).
static void registerAppIdentity()
{
    SetCurrentProcessExplicitAppUserModelID(L"VivacePlayer.Vivace");
}
#endif

namespace {

// Print CLI text (--help / --version) to the terminal. A WIN32 GUI app has no
// console of its own, so on Windows we attach to the launching terminal's
// console; when launched without one (e.g. from Explorer) the text is silently
// dropped, which is fine for command-line-only output.
void printCliMessage(const QString &text)
{
    const QByteArray bytes = (text + QLatin1Char('\n')).toLocal8Bit();
#ifdef Q_OS_WIN
    // Use the raw output handle so this works whether stdout is redirected to a
    // file/pipe (inherited handle) or connected to the launching terminal's
    // console (attached below). WriteFile handles both handle kinds.
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    bool attached = false;
    if (h == nullptr || h == INVALID_HANDLE_VALUE) {
        attached = AttachConsole(ATTACH_PARENT_PROCESS);
        if (attached) {
            h = GetStdHandle(STD_OUTPUT_HANDLE);
            if (h == nullptr || h == INVALID_HANDLE_VALUE)
                h = CreateFileW(L"CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE,
                                nullptr, OPEN_EXISTING, 0, nullptr);
        }
    }
    if (h && h != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(h, bytes.constData(), static_cast<DWORD>(bytes.size()),
                  &written, nullptr);
    }
    if (attached)
        FreeConsole();
#else
    std::fputs(bytes.constData(), stdout);
    std::fflush(stdout);
#endif
}

// Handle --help/-h and --version/-v up front (before single-instance
// forwarding, so a second `vivace --help` prints and exits without disturbing a
// running window). Returns true if such a request was handled and main() should
// exit. Requires the application name/version to already be set.
bool handleCliInfoRequest(const QStringList &userArgs)
{
    QCommandLineParser parser;
    configureCliParser(parser);
    parser.parse(QStringList{ QStringLiteral("vivace") } + userArgs);
    if (parser.isSet(QStringLiteral("help"))) {
        printCliMessage(parser.helpText());
        return true;
    }
    if (parser.isSet(QStringLiteral("version"))) {
        printCliMessage(QCoreApplication::applicationName()
                        + QLatin1Char(' ')
                        + QCoreApplication::applicationVersion());
        return true;
    }
    return false;
}

} // namespace

int main(int argc, char *argv[])
{
    // Force Qt Multimedia's FFmpeg backend. Vivace is designed and tested only
    // against it (subtitle delivery via the video sink, DVD device sourcing,
    // etc.); the platform backends are unsupported. On Windows in particular the
    // MediaFoundation backend intermittently fails with "Media session serious
    // error" on some files, and it is chosen automatically when the FFmpeg
    // plugin's DLLs aren't found. Setting this before the media stack
    // initialises pins the backend (respecting an explicit user override).
    if (!qEnvironmentVariableIsSet("QT_MEDIA_BACKEND"))
        qputenv("QT_MEDIA_BACKEND", "ffmpeg");

    // Settings read before QGuiApplication (explicit org/app: the app names
    // are not registered yet, and QT_SCALE_FACTOR must be set beforehand).
    const QSettings preStore(QSettings::NativeFormat, QSettings::UserScope,
                             QStringLiteral("vivace-player"),
                             QStringLiteral("vivace"));
    const qreal scaleFactor =
            preStore.value(QStringLiteral("ui/interfaceScaleFactor"), 0.0).toReal();
    if (scaleFactor > 0.0 && !qEnvironmentVariableIsSet("QT_SCALE_FACTOR"))
        qputenv("QT_SCALE_FACTOR", QByteArray::number(scaleFactor));

    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("vivace"));
    QCoreApplication::setOrganizationName(QStringLiteral("vivace-player"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("Vivace"));

    installFileLogger(); // capture Qt/QML warnings to a file for troubleshooting

    // UI language: the ui/language setting ("" = follow the system's *display*
    // language), applied here before the QML engine loads so qsTr()/tr()
    // resolve. The app catalog is embedded (qrc:/i18n/vivace_<lang>.qm); qtbase
    // provides the standard dialog-button text. Restart-required (like style).
    //
    // For "system default" we deliberately use only the FIRST entry of the
    // system's uiLanguages() list — the user's primary display language — rather
    // than passing QLocale::system() straight to QTranslator::load(). The system
    // list reflects ALL configured preferences and (per the Qt docs, more so
    // since 6.9) includes region/format-derived fallbacks: for a user in e.g.
    // Japan running an English display language it contains "ja" as a lower
    // priority entry, so load() would pick vivace_ja.qm when there is no English
    // catalog — showing Japanese to an English user. Restricting to the primary
    // display language makes an absent catalog fall back to the (English) source
    // strings, as expected.
    const QString uiLang =
            preStore.value(QStringLiteral("ui/language")).toString();
    QLocale uiLocale;
    if (uiLang.isEmpty()) {
        const QStringList sysUi = QLocale::system().uiLanguages();
        uiLocale = sysUi.isEmpty() ? QLocale::system()
                                   : QLocale(sysUi.constFirst());
    } else {
        uiLocale = QLocale(uiLang);
    }
    static QTranslator appTranslator;
    if (appTranslator.load(uiLocale, QStringLiteral("vivace"),
                           QStringLiteral("_"), QStringLiteral(":/i18n")))
        QCoreApplication::installTranslator(&appTranslator);
    static QTranslator qtTranslator;
    if (qtTranslator.load(uiLocale, QStringLiteral("qtbase"),
                          QStringLiteral("_"),
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        QCoreApplication::installTranslator(&qtTranslator);

#ifdef Q_OS_WIN
    registerAppIdentity(); // taskbar / SMTC app name (before any UI)
#endif

    // Qt Quick Controls style: Fusion by default (a fully customizable style;
    // the platform-native default only tolerates our custom menu/slider
    // delegates), overridable in Preferences > Interface (restart required).
    const QString style =
            preStore.value(QStringLiteral("ui/qtStyle"),
                           QStringLiteral("Fusion")).toString();
    QQuickStyle::setStyle(style.isEmpty() ? QStringLiteral("Fusion") : style);

    QIcon appIcon;
    for (const int size : { 16, 32, 64, 128, 256 }) {
        appIcon.addFile(QStringLiteral(":/qt/qml/Vivace/icons/app_%1.png")
                                .arg(size));
    }
    QGuiApplication::setWindowIcon(appIcon);

    // Optional application font from settings (read directly; the Settings
    // QML singleton is not created yet). Applied before any UI is built. Touch
    // mode enlarges the whole UI font here so it reaches every window uniformly
    // — including the plain-Window dialogs, which (unlike ApplicationWindow)
    // have no `font` property to bind in QML. Icon sizes scale live via
    // Theme.touchScale; the font part applies on the next launch.
    {
        const QSettings store;
        const QString family = store.value(QStringLiteral("ui/fontFamily"))
                                       .toString();
        const int size = store.value(QStringLiteral("ui/fontSize"), 0).toInt();
        const bool touch = store.value(QStringLiteral("ui/touchMode"), false)
                                   .toBool();
        if (!family.isEmpty() || size > 0 || touch) {
            QFont font = QGuiApplication::font();
            if (!family.isEmpty())
                font.setFamily(family);
            if (size > 0)
                font.setPointSize(size);
            if (touch) {
                if (font.pointSizeF() > 0)
                    font.setPointSizeF(font.pointSizeF() * 1.35);
                else if (font.pixelSize() > 0)
                    font.setPixelSize(qRound(font.pixelSize() * 1.35));
            }
            QGuiApplication::setFont(font);
        }
    }

    // Command-line options (media + --fullscreen/--ontop/--close-at-end/--sub/
    // --pos/--size/--start), applied by Main.qml's applyStartupOptions().
    const QStringList userArgs = QCoreApplication::arguments().mid(1);

    // --help/--version: print to the terminal and exit (before single-instance
    // forwarding, so it never disturbs a running window).
    if (handleCliInfoRequest(userArgs))
        return 0;

    const QVariantMap startupOptions = parseVivaceArgs(userArgs);

    // Single-instance mode (Preferences > Interface > Instances): hand the
    // arguments to a running instance and exit, or become the primary.
    SingleInstance instance(QStringLiteral("vivace-single-instance-")
                            + qEnvironmentVariable("USERNAME"));
    const bool singleInstance =
            preStore.value(QStringLiteral("ui/singleInstance"), false).toBool();
    if (singleInstance && instance.sendToRunning(userArgs))
        return 0;
    if (singleInstance)
        instance.startServer();

    QQmlApplicationEngine engine;
    engine.setInitialProperties({
        { QStringLiteral("startupOptions"), startupOptions }
    });

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Vivace", "Main");

    // Forward a later instance's arguments to the running window.
    QObject::connect(
        &instance, &SingleInstance::messageReceived, &app,
        [&engine](const QStringList &incoming) {
            if (engine.rootObjects().isEmpty())
                return;
            const QVariantMap opts = parseVivaceArgs(incoming);
            QMetaObject::invokeMethod(engine.rootObjects().first(),
                                      "handleSecondInstance",
                                      Q_ARG(QVariant, QVariant(opts)));
        });

    return app.exec();
}
