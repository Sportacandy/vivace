/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "uihelpers.h"

#include <algorithm>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QKeySequence>
#include <QLocale>
#include <QMediaFormat>
#include <QCursor>
#include <QGuiApplication>
#include <QSettings>
#include <QTextStream>
#include <QUrl>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shlobj.h>
#ifdef _MSC_VER
#pragma comment(lib, "shell32.lib")
#endif
#endif

namespace {

// Alphabetical {name, description, value} rows from a list of enums.
template<typename Enum, typename NameFn, typename DescFn>
QVariantList formatRows(const QList<Enum> &values, NameFn name, DescFn description)
{
    QList<QVariantMap> rows;
    rows.reserve(values.size());
    for (const Enum value : values) {
        rows.append({ { QStringLiteral("name"), name(value) },
                      { QStringLiteral("description"), description(value) },
                      { QStringLiteral("value"), int(value) } });
    }
    std::sort(rows.begin(), rows.end(),
              [](const QVariantMap &a, const QVariantMap &b) {
                  return a.value(QStringLiteral("name")).toString()
                                 .compare(b.value(QStringLiteral("name"))
                                                  .toString(),
                                          Qt::CaseInsensitive) < 0;
              });

    QVariantList out;
    out.reserve(rows.size());
    for (const QVariantMap &row : rows)
        out << row;
    return out;
}

} // namespace

UiHelpers::UiHelpers(QObject *parent)
    : QObject(parent)
{
}

QString UiHelpers::shortcutText(const QVariant &shortcut) const
{
    if (!shortcut.isValid())
        return {};

    QKeySequence sequence;
    if (shortcut.userType() == QMetaType::QKeySequence)
        sequence = shortcut.value<QKeySequence>();
    else if (shortcut.userType() == QMetaType::QString)
        sequence = QKeySequence::fromString(shortcut.toString(),
                                            QKeySequence::PortableText);
    else if (shortcut.canConvert<int>())
        sequence = QKeySequence(
                static_cast<QKeySequence::StandardKey>(shortcut.toInt()));

    return sequence.toString(QKeySequence::NativeText);
}

QString UiHelpers::mnemonicLabel(const QString &text) const
{
    QString out;
    out.reserve(text.size() + 8);

    bool underlineNext = false;
    for (qsizetype i = 0; i < text.size(); ++i) {
        const QChar c = text.at(i);

        if (c == u'&' && !underlineNext) {
            if (i + 1 < text.size() && text.at(i + 1) == u'&') {
                out += QStringLiteral("&amp;");
                ++i;
            } else {
                underlineNext = true;
            }
            continue;
        }

        QString escaped;
        if (c == u'<')
            escaped = QStringLiteral("&lt;");
        else if (c == u'>')
            escaped = QStringLiteral("&gt;");
        else if (c == u'&')
            escaped = QStringLiteral("&amp;");
        else
            escaped = c;

        if (underlineNext) {
            out += QStringLiteral("<u>") + escaped + QStringLiteral("</u>");
            underlineNext = false;
        } else {
            out += escaped;
        }
    }
    return out;
}

QString UiHelpers::toLocalPath(const QUrl &url) const
{
    return url.isLocalFile() ? QDir::toNativeSeparators(url.toLocalFile())
                             : url.toString();
}

void UiHelpers::setCursorHidden(bool hidden)
{
    // Hide/show the pointer at once via the application override cursor. A
    // QML MouseArea.cursorShape change doesn't re-apply to a stationary
    // pointer, so it can't hide on inactivity; the override does. The override
    // is application-wide, so the caller must scope it — only activate it while
    // the pointer is idle over the video, and clear it the instant the pointer
    // moves, leaves the video, or the window is deactivated. Balanced via the
    // guard so set/restore stay paired.
    if (hidden == m_cursorHidden)
        return;
    m_cursorHidden = hidden;
    if (hidden)
        QGuiApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
    else
        QGuiApplication::restoreOverrideCursor();
}

QString UiHelpers::readInternetShortcut(const QUrl &fileUrl) const
{
    const QString path = fileUrl.isLocalFile() ? fileUrl.toLocalFile()
                                               : fileUrl.toString();
    if (QFileInfo(path).suffix().compare(QStringLiteral("url"),
                                         Qt::CaseInsensitive) != 0)
        return {};
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    // Parse manually rather than via QSettings: the URL value can contain '='
    // and other characters QSettings would mangle. Return the first URL= value.
    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.startsWith(QStringLiteral("URL="), Qt::CaseInsensitive))
            return line.mid(4).trimmed();
    }
    return {};
}

QString UiHelpers::readTextFile(const QUrl &url) const
{
    // Accept a qrc URL (qrc:/...), a file URL, or a bare path/resource string.
    QString path;
    if (url.scheme() == QLatin1String("qrc"))
        path = QLatin1Char(':') + url.path();
    else if (url.isLocalFile())
        path = url.toLocalFile();
    else
        path = url.toString();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    return QString::fromUtf8(file.readAll());
}

QUrl UiHelpers::configFolderUrl() const
{
    const QSettings probe(QSettings::IniFormat, QSettings::UserScope,
                          QStringLiteral("vivace-player"),
                          QStringLiteral("vivace_files"));
    return QUrl::fromLocalFile(QFileInfo(probe.fileName()).absolutePath());
}

QUrl UiHelpers::logFileUrl() const
{
    const QSettings probe(QSettings::IniFormat, QSettings::UserScope,
                          QStringLiteral("vivace-player"),
                          QStringLiteral("vivace_files"));
    const QString path = QFileInfo(probe.fileName()).absolutePath()
            + QStringLiteral("/vivace.log");
    return QFileInfo::exists(path) ? QUrl::fromLocalFile(path) : QUrl();
}

QString UiHelpers::qtRuntimeVersion() const
{
    return QString::fromLatin1(qVersion());
}

QString UiHelpers::qtBuildVersion() const
{
    return QStringLiteral(QT_VERSION_STR);
}

QVariantList UiHelpers::availableUiLanguages() const
{
    // Enumerate the embedded catalogs (qrc:/i18n/vivace_<lang>.qm) so the
    // selector always matches what is actually shipped.
    QVariantList out;
    const QDir dir(QStringLiteral(":/i18n"));
    const QStringList files =
            dir.entryList({ QStringLiteral("vivace_*.qm") }, QDir::Files);
    for (const QString &file : files) {
        // "vivace_pt_BR.qm" -> "pt_BR"
        QString code = file;
        code.remove(0, QStringLiteral("vivace_").size());
        code.chop(QStringLiteral(".qm").size());
        const QLocale locale(code);
        QString name = locale.nativeLanguageName();
        if (name.isEmpty())
            name = code;
        else
            name[0] = name[0].toUpper();
        // Disambiguate territory variants (pt_BR, zh_CN/zh_TW) by native
        // territory so they don't collapse to the same language name.
        if (code.contains(QLatin1Char('_'))) {
            const QString territory = locale.nativeTerritoryName();
            if (!territory.isEmpty())
                name += QStringLiteral(" (%1)").arg(territory);
        }
        QVariantMap row;
        row[QStringLiteral("code")] = code;
        row[QStringLiteral("label")] = name;
        out.append(row);
    }
    std::sort(out.begin(), out.end(), [](const QVariant &a, const QVariant &b) {
        return a.toMap().value(QStringLiteral("label")).toString().localeAwareCompare(
                       b.toMap().value(QStringLiteral("label")).toString()) < 0;
    });
    return out;
}

QVariantList UiHelpers::supportedFileFormats() const
{
    return formatRows(QMediaFormat().supportedFileFormats(QMediaFormat::Decode),
                      &QMediaFormat::fileFormatName,
                      &QMediaFormat::fileFormatDescription);
}

QVariantList UiHelpers::supportedVideoCodecs() const
{
    return formatRows(QMediaFormat().supportedVideoCodecs(QMediaFormat::Decode),
                      &QMediaFormat::videoCodecName,
                      &QMediaFormat::videoCodecDescription);
}

QVariantList UiHelpers::supportedAudioCodecs() const
{
    return formatRows(QMediaFormat().supportedAudioCodecs(QMediaFormat::Decode),
                      &QMediaFormat::audioCodecName,
                      &QMediaFormat::audioCodecDescription);
}

// ---- Windows file associations (user scope) --------------------------------

#ifdef Q_OS_WIN
namespace {
// Our ProgID under HKCU\Software\Classes.
const QString kProgId = QStringLiteral("Vivace.MediaFile");

QSettings classesRoot()
{
    return QSettings(QStringLiteral("HKEY_CURRENT_USER\\Software\\Classes"),
                     QSettings::NativeFormat);
}

// (Re)writes the ProgID so its open command points at the current exe.
void ensureProgId(QSettings &reg)
{
    const QString exe =
            QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    reg.setValue(kProgId + QStringLiteral("/."),
                 QStringLiteral("Vivace media file"));
    reg.setValue(kProgId + QStringLiteral("/DefaultIcon/."),
                 QStringLiteral("\"%1\",0").arg(exe));
    reg.setValue(kProgId + QStringLiteral("/shell/open/command/."),
                 QStringLiteral("\"%1\" \"%2\"").arg(exe, QStringLiteral("%1")));
}
} // namespace
#endif

bool UiHelpers::fileAssociationsSupported() const
{
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}

QStringList UiHelpers::associatedExtensions(const QStringList &extensions) const
{
    QStringList out;
#ifdef Q_OS_WIN
    QSettings reg = classesRoot();
    for (const QString &ext : extensions) {
        const QString dotExt = QStringLiteral(".") + ext;
        const bool inOpenWith = reg.contains(
                dotExt + QStringLiteral("/OpenWithProgids/") + kProgId);
        const bool isDefault =
                reg.value(dotExt + QStringLiteral("/.")).toString() == kProgId;
        if (inOpenWith || isDefault)
            out << ext;
    }
#else
    Q_UNUSED(extensions);
#endif
    return out;
}

bool UiHelpers::setFileAssociations(const QStringList &toAssociate,
                                    const QStringList &toRemove) const
{
#ifdef Q_OS_WIN
    QSettings reg = classesRoot();
    if (!toAssociate.isEmpty())
        ensureProgId(reg);

    for (const QString &ext : toAssociate) {
        const QString dotExt = QStringLiteral(".") + ext;
        // Advertise in "Open with"; claim the classic default only when the
        // extension has none (never clobber another app's association —
        // Windows' UserChoice governs the real default anyway).
        reg.setValue(dotExt + QStringLiteral("/OpenWithProgids/") + kProgId,
                     QString());
        if (reg.value(dotExt + QStringLiteral("/.")).toString().isEmpty())
            reg.setValue(dotExt + QStringLiteral("/."), kProgId);
    }
    for (const QString &ext : toRemove) {
        const QString dotExt = QStringLiteral(".") + ext;
        reg.remove(dotExt + QStringLiteral("/OpenWithProgids/") + kProgId);
        if (reg.value(dotExt + QStringLiteral("/.")).toString() == kProgId)
            reg.remove(dotExt + QStringLiteral("/."));
    }
    reg.sync();
    const bool ok = reg.status() == QSettings::NoError;
    if (ok)
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    return ok;
#else
    Q_UNUSED(toAssociate);
    Q_UNUSED(toRemove);
    return false;
#endif
}
