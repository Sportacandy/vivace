/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SUBTITLEPARSER_H
#define SUBTITLEPARSER_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>

// One subtitle line with its on-screen interval. `text` is Qt StyledText:
// SRT/VTT inline tags (<i>, <b>, <u>) pass through and line breaks become
// <br>; ASS override codes are stripped.
struct SubtitleCue
{
    qint64 startMs = 0;
    qint64 endMs = 0;
    QString text;
};

/*  External subtitle loader (Qt Multimedia can't take external subs). Parses
    SubRip (.srt), WebVTT (.vtt) and basic SSA/ASS (.ass/.ssa) into cues that
    a QML overlay renders synced to the playback position. (SMPlayer delegates
    this to mplayer/mpv; here it is our own, so styling/delay/position are ours
    to control.)
*/
namespace SubtitleParser {

bool isSubtitleFile(const QUrl &url);
const QStringList &supportedExtensions(); // without the leading dot

// Cues sorted by start time; empty on failure/unknown format.
QList<SubtitleCue> load(const QUrl &url);

} // namespace SubtitleParser

#endif // SUBTITLEPARSER_H
