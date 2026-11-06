/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "playlistparser.h"

#include "androidcontentpath.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringConverter>
#include <QTextStream>

namespace PlaylistParser {

namespace {

QUrl entryUrl(const QString &line, const QDir &baseDir)
{
    // Lines can be URLs (http://…), absolute paths (also with backslashes
    // or drive letters) or paths relative to the playlist's folder.
    const QUrl asUrl(line);
    if (!asUrl.scheme().isEmpty() && asUrl.scheme().size() > 1)
        return asUrl; // real scheme (length 1 would be a drive letter)

    const QString path = QDir::fromNativeSeparators(line);
    if (QDir::isAbsolutePath(path))
        return QUrl::fromLocalFile(QDir::cleanPath(path));
    return QUrl::fromLocalFile(QDir::cleanPath(baseDir.absoluteFilePath(path)));
}

// True when entryUrl() doesn't need the playlist's folder for `line`.
bool isAbsoluteEntry(const QString &line)
{
    const QUrl asUrl(line);
    return (!asUrl.scheme().isEmpty() && asUrl.scheme().size() > 1)
            || QDir::isAbsolutePath(QDir::fromNativeSeparators(line));
}

// An Android file-picker URI (content://). Its file name is the document's
// display name, which Qt's Android content file engine resolves for
// QFileInfo; QFile reads the document through the same engine.
bool isContentUrl(const QUrl &url)
{
    return url.scheme() == QLatin1String("content");
}

QString playlistSuffix(const QUrl &url)
{
    if (url.isLocalFile())
        return QFileInfo(url.toLocalFile()).suffix().toLower();
    // QFileInfo::suffix() parses the raw path string, which for a content://
    // URI ends in a document id ("msf%3A7333"), not a file name. Only
    // fileName() asks the content engine for the real display name.
    if (isContentUrl(url))
        return QFileInfo(QFileInfo(url.toString()).fileName()).suffix().toLower();
    return {};
}

// Same folder as the playlist being saved: write just the filename, so the
// playlist stays valid if the folder (playlist + media together) is moved
// or copied elsewhere, with no absolute path to go stale. canonicalPath()
// (not absolutePath()) drives the comparison so it isn't fooled by case
// differences (harmless on a case-insensitive filesystem, but on a
// case-sensitive one a false "same folder" match would write a filename
// that resolves to the wrong place on load) or a relative/symlinked
// argument; it falls back to absolutePath() when the entry doesn't exist
// on disk (a dangling playlist entry) since canonicalPath() then returns
// empty.
QString entryLine(const QUrl &url, const QString &playlistDir)
{
    if (!url.isLocalFile())
        return url.toString();

    const QFileInfo entryInfo(url.toLocalFile());
    QString entryDir = entryInfo.canonicalPath();
    if (entryDir.isEmpty())
        entryDir = entryInfo.absolutePath();

    if (!playlistDir.isEmpty() && entryDir == playlistDir)
        return entryInfo.fileName();
    return QDir::toNativeSeparators(entryInfo.absoluteFilePath());
}

// `baseDir` resolves relative entries; empty when the playlist's folder is
// unknown (an Android content:// playlist whose real path couldn't be
// found), in which case relative entries are skipped -- they can't be
// opened from the one document the file picker granted access to.
QList<PlaylistEntry> loadM3u(const QString &filePath, bool utf8, const QString &baseDirPath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    QTextStream stream(&file);
    stream.setEncoding(utf8 ? QStringConverter::Utf8 : QStringConverter::System);
    // BOM (if any) still wins over the encoding set above.
    stream.setAutoDetectUnicode(true);

    const QDir baseDir(baseDirPath);
    QList<PlaylistEntry> entries;
    QString pendingTitle;
    qint64 pendingDurationMs = -1;

    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty())
            continue;
        if (line.startsWith(QLatin1String("#EXTINF:"))) {
            // #EXTINF:<seconds>,<title>  (seconds may be -1 or fractional)
            const qsizetype comma = line.indexOf(u',');
            const QString head = line.mid(8, (comma >= 0 ? comma : line.size()) - 8);
            bool ok = false;
            const double secs = head.trimmed().toDouble(&ok);
            pendingDurationMs = (ok && secs > 0) ? qint64(secs * 1000) : -1;
            pendingTitle = comma >= 0 ? line.mid(comma + 1).trimmed() : QString();
            continue;
        }
        if (line.startsWith(u'#'))
            continue;

        if (!baseDirPath.isEmpty() || isAbsoluteEntry(line))
            entries.append({ entryUrl(line, baseDir), pendingTitle, pendingDurationMs });
        pendingTitle.clear();
        pendingDurationMs = -1;
    }
    return entries;
}

} // namespace

bool isPlaylistFile(const QUrl &url)
{
    const QString suffix = playlistSuffix(url);
    return suffix == QLatin1String("m3u") || suffix == QLatin1String("m3u8");
}

QList<PlaylistEntry> load(const QUrl &url)
{
    if (!isPlaylistFile(url))
        return {};
    const bool utf8 = playlistSuffix(url) == QLatin1String("m3u8");
    if (isContentUrl(url)) {
        const QString realPath = AndroidContentPath::contentFilePath(url);
        const QString baseDir = realPath.isEmpty()
                ? QString() : QFileInfo(realPath).absolutePath();
        return loadM3u(url.toString(), utf8, baseDir);
    }
    const QString filePath = url.toLocalFile();
    return loadM3u(filePath, utf8, QFileInfo(filePath).absolutePath());
}

bool save(const QUrl &url, const QList<PlaylistEntry> &entries)
{
    if (!url.isLocalFile())
        return false;

    QFile file(url.toLocalFile());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return false;

    QTextStream stream(&file);
    const bool utf8 = QFileInfo(file.fileName()).suffix().toLower()
                      != QLatin1String("m3u");
    stream.setEncoding(utf8 ? QStringConverter::Utf8 : QStringConverter::System);

    const QFileInfo playlistInfo(file.fileName());
    QString playlistDir = playlistInfo.canonicalPath();
    if (playlistDir.isEmpty())
        playlistDir = playlistInfo.absolutePath();

    stream << "#EXTM3U\n";
    for (const PlaylistEntry &entry : entries) {
        if (!entry.title.isEmpty() || entry.durationMs > 0) {
            const qint64 secs = entry.durationMs > 0 ? entry.durationMs / 1000 : -1;
            stream << "#EXTINF:" << secs << "," << entry.title << "\n";
        }
        stream << entryLine(entry.url, playlistDir) << "\n";
    }
    return stream.status() == QTextStream::Ok;
}

} // namespace PlaylistParser
