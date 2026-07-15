/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "subtitleparser.h"

#include <algorithm>

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStringConverter>
#include <QTextStream>

namespace SubtitleParser {

namespace {

// "HH:MM:SS,mmm" / "HH:MM:SS.mmm" / "MM:SS.mmm" (SRT and VTT).
qint64 parseSrtVttTime(const QString &s)
{
    const QString t = s.trimmed().replace(u',', u'.');
    const QStringList hms = t.split(u':');
    if (hms.isEmpty())
        return -1;
    double h = 0, m = 0, sec = 0;
    bool ok = true;
    if (hms.size() == 3) {
        h = hms.at(0).toDouble(&ok);
        m = ok ? hms.at(1).toDouble(&ok) : 0;
        sec = ok ? hms.at(2).toDouble(&ok) : 0;
    } else if (hms.size() == 2) {
        m = hms.at(0).toDouble(&ok);
        sec = ok ? hms.at(1).toDouble(&ok) : 0;
    } else {
        return -1;
    }
    if (!ok)
        return -1;
    return qint64((h * 3600 + m * 60 + sec) * 1000);
}

// "H:MM:SS.cc" (ASS/SSA, centiseconds).
qint64 parseAssTime(const QString &s)
{
    const QStringList hms = s.trimmed().split(u':');
    if (hms.size() != 3)
        return -1;
    bool ok = true;
    const double h = hms.at(0).toDouble(&ok);
    const double m = ok ? hms.at(1).toDouble(&ok) : 0;
    const double sec = ok ? hms.at(2).toDouble(&ok) : 0;
    return ok ? qint64((h * 3600 + m * 60 + sec) * 1000) : -1;
}

QString openText(const QString &filePath, bool utf8)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    QTextStream stream(&file);
    stream.setEncoding(utf8 ? QStringConverter::Utf8 : QStringConverter::System);
    stream.setAutoDetectUnicode(true); // honour a BOM if present
    return stream.readAll();
}

// Splits a text body into blank-line-separated blocks (LF/CRLF tolerant).
QStringList blocks(const QString &body)
{
    return body.split(QRegularExpression(QStringLiteral("\\r?\\n\\r?\\n")),
                      Qt::SkipEmptyParts);
}

QList<SubtitleCue> parseSrtVtt(const QString &body)
{
    QList<SubtitleCue> cues;
    static const QRegularExpression arrow(QStringLiteral("-->"));
    const QStringList parts = blocks(body);
    for (const QString &block : parts) {
        QStringList lines = block.split(QRegularExpression(QStringLiteral("\\r?\\n")));
        // Find the timing line ("start --> end").
        int timingLine = -1;
        for (int i = 0; i < lines.size(); ++i) {
            if (lines.at(i).contains(arrow)) {
                timingLine = i;
                break;
            }
        }
        if (timingLine < 0)
            continue; // header (WEBVTT), NOTE, STYLE, etc.

        const QStringList ab = lines.at(timingLine).split(arrow);
        if (ab.size() < 2)
            continue;
        SubtitleCue cue;
        cue.startMs = parseSrtVttTime(ab.at(0));
        // The end field may carry VTT cue settings after the timestamp;
        // trim first so a leading space doesn't yield an empty section.
        cue.endMs = parseSrtVttTime(ab.at(1).trimmed().section(u' ', 0, 0));
        if (cue.startMs < 0 || cue.endMs < 0 || cue.endMs < cue.startMs)
            continue;

        const QStringList textLines = lines.mid(timingLine + 1);
        cue.text = textLines.join(QStringLiteral("<br>")).trimmed();
        if (!cue.text.isEmpty())
            cues.append(cue);
    }
    return cues;
}

QList<SubtitleCue> parseAss(const QString &body)
{
    QList<SubtitleCue> cues;
    const QStringList lines = body.split(QRegularExpression(QStringLiteral("\\r?\\n")));
    int startCol = 1, endCol = 2, textCol = 9, fields = 10;
    bool inEvents = false;

    static const QRegularExpression overrides(QStringLiteral("\\{[^}]*\\}"));

    for (const QString &raw : lines) {
        const QString line = raw.trimmed();
        if (line.startsWith(u'[')) {
            inEvents = line.compare(QStringLiteral("[Events]"),
                                    Qt::CaseInsensitive) == 0;
            continue;
        }
        if (!inEvents)
            continue;

        if (line.startsWith(QStringLiteral("Format:"), Qt::CaseInsensitive)) {
            const QStringList cols =
                    line.mid(7).split(u',');
            fields = cols.size();
            for (int i = 0; i < cols.size(); ++i) {
                const QString c = cols.at(i).trimmed();
                if (c.compare(QStringLiteral("Start"), Qt::CaseInsensitive) == 0)
                    startCol = i;
                else if (c.compare(QStringLiteral("End"), Qt::CaseInsensitive) == 0)
                    endCol = i;
                else if (c.compare(QStringLiteral("Text"), Qt::CaseInsensitive) == 0)
                    textCol = i;
            }
            continue;
        }
        if (!line.startsWith(QStringLiteral("Dialogue:"), Qt::CaseInsensitive))
            continue;

        // Text is the last field and may contain commas, so keep the tail.
        const QStringList cols = line.mid(9).split(u',');
        if (cols.size() < fields || textCol >= fields)
            continue;
        SubtitleCue cue;
        cue.startMs = parseAssTime(cols.value(startCol));
        cue.endMs = parseAssTime(cols.value(endCol));
        if (cue.startMs < 0 || cue.endMs < 0 || cue.endMs < cue.startMs)
            continue;
        QString text = cols.mid(textCol).join(u','); // rejoin the tail
        text.remove(overrides);                       // drop {\...} codes
        text.replace(QStringLiteral("\\N"), QStringLiteral("<br>"));
        text.replace(QStringLiteral("\\n"), QStringLiteral("<br>"));
        text.replace(QStringLiteral("\\h"), QStringLiteral(" "));
        cue.text = text.trimmed();
        if (!cue.text.isEmpty())
            cues.append(cue);
    }
    return cues;
}

} // namespace

const QStringList &supportedExtensions()
{
    static const QStringList exts = {
        QStringLiteral("srt"), QStringLiteral("vtt"),
        QStringLiteral("ass"), QStringLiteral("ssa"),
    };
    return exts;
}

bool isSubtitleFile(const QUrl &url)
{
    if (!url.isLocalFile())
        return false;
    return supportedExtensions().contains(
            QFileInfo(url.toLocalFile()).suffix().toLower());
}

QList<SubtitleCue> load(const QUrl &url)
{
    if (!isSubtitleFile(url))
        return {};
    const QString path = url.toLocalFile();
    const QString suffix = QFileInfo(path).suffix().toLower();
    const QString body = openText(path, true); // UTF-8 with BOM autodetect
    if (body.isEmpty())
        return {};

    QList<SubtitleCue> cues = (suffix == QLatin1String("ass")
                               || suffix == QLatin1String("ssa"))
            ? parseAss(body)
            : parseSrtVtt(body);
    std::stable_sort(cues.begin(), cues.end(),
                     [](const SubtitleCue &a, const SubtitleCue &b) {
                         return a.startMs < b.startMs;
                     });
    return cues;
}

} // namespace SubtitleParser
