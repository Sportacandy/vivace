/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "playlistparser.h"

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

QList<PlaylistEntry> loadM3u(const QString &filePath, bool utf8)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    QTextStream stream(&file);
    stream.setEncoding(utf8 ? QStringConverter::Utf8 : QStringConverter::System);
    // BOM (if any) still wins over the encoding set above.
    stream.setAutoDetectUnicode(true);

    const QDir baseDir = QFileInfo(filePath).dir();
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

        entries.append({ entryUrl(line, baseDir), pendingTitle, pendingDurationMs });
        pendingTitle.clear();
        pendingDurationMs = -1;
    }
    return entries;
}

} // namespace

bool isPlaylistFile(const QUrl &url)
{
    if (!url.isLocalFile())
        return false;
    const QString suffix = QFileInfo(url.toLocalFile()).suffix().toLower();
    return suffix == QLatin1String("m3u") || suffix == QLatin1String("m3u8");
}

QList<PlaylistEntry> load(const QUrl &url)
{
    if (!isPlaylistFile(url))
        return {};
    const QString filePath = url.toLocalFile();
    const bool utf8 = QFileInfo(filePath).suffix().toLower() == QLatin1String("m3u8");
    return loadM3u(filePath, utf8);
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

    stream << "#EXTM3U\n";
    for (const PlaylistEntry &entry : entries) {
        if (!entry.title.isEmpty() || entry.durationMs > 0) {
            const qint64 secs = entry.durationMs > 0 ? entry.durationMs / 1000 : -1;
            stream << "#EXTINF:" << secs << "," << entry.title << "\n";
        }
        stream << (entry.url.isLocalFile()
                           ? QDir::toNativeSeparators(entry.url.toLocalFile())
                           : entry.url.toString())
               << "\n";
    }
    return stream.status() == QTextStream::Ok;
}

} // namespace PlaylistParser
