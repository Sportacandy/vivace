/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Chapter extraction for local media files. Qt Multimedia exposes no chapter
    API (QMediaMetaData has no Chapters key), so — as with the DVD IFOs — we
    parse the container ourselves: Matroska/WebM EBML Chapters and MP4/MOV Nero
    'chpl' boxes. Feeds the Browse > Chapters menu and the seek-bar tick marks.
*/

#ifndef CHAPTERPARSER_H
#define CHAPTERPARSER_H

#include <QList>
#include <QString>

namespace ChapterParse {

struct Chapter {
    qint64 startMs = 0;
    QString title; // may be empty; the UI falls back to a number
};

// Chapters of a local media file, ordered by start time. Empty when the file
// has none, is not a supported container, or cannot be read.
QList<Chapter> chapters(const QString &filePath);

} // namespace ChapterParse

#endif // CHAPTERPARSER_H
