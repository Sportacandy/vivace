/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef DVDIFOPARSER_H
#define DVDIFOPARSER_H

#include <QList>
#include <QString>

/*  Minimal DVD-Video IFO parsing: enough of VIDEO_TS.IFO (title table)
    and VTS_xx_0.IFO (program chains) to know which cells — sector ranges
    within the title-set VOB domain — make up each title, and how long
    each title plays. No menus, no cell commands, no multi-PGC titles;
    interleaved angle blocks contribute their first angle only.
*/
namespace DvdIfo {

struct Cell {
    qint64 firstSector = 0;
    qint64 lastSector = 0; // inclusive, 2048-byte sectors in the VOB domain
};

struct Title {
    int titleNumber = 0; // 1-based, as on the disc
    int vtsNumber = 0;   // which VTS_xx the title lives in
    int vtsTitleNumber = 0; // 1-based title index within its VTS (for JumpVTS_TT)
    qint64 durationMs = 0;
    QList<Cell> cells;   // in playback order
    QList<qint64> cellStartsMs;    // start time of each entry in `cells`
    // True when the cell starts a new timestamp timeline (STC
    // discontinuity flag, or a different logical VOB unit): playback
    // must be split there, the decoder clock cannot cross it.
    QList<bool> cellNewTimeline;
    QList<qint64> chapterStartsMs; // one entry per chapter (program)
    QList<int> chapterCellIndexes; // index into `cells` per chapter

    // VTS_TMAPTI time map: sector of the VOBU playing at time
    // (i + 1) * timeMapUnitSec. Empty when the disc has no time map.
    int timeMapUnitSec = 0;
    QList<quint32> timeMapSectors;
};

// Empty when the folder has no parseable IFO structure.
QList<Title> titles(const QString &videoTsDir);

} // namespace DvdIfo

#endif // DVDIFOPARSER_H
