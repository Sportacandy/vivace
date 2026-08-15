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

// VTSI_MAT's subtitle (subpicture) stream attribute table: declared once per
// VTS, shared by every title in it. `id` is 0-based (0..31); the actual
// private_stream_1 sub-stream ID a decoder must match is 0x20 + id.
struct SubtitleStream {
    int id = 0;
    QString language; // 2-letter code (e.g. "en", "ja"); empty if unset
};

// VTSI_MAT's audio stream attribute table. `id` is 0-based (0..7); which PES
// stream ID it actually shows up as depends on `codingMode` (the attribute's
// own top 3 bits: 0 = AC3, 2/3 = MPEG audio, 4 = LPCM, 6 = DTS, 7 = SDDS) --
// not resolved here, since nothing in this codebase demuxes DVD audio
// independently of FFmpeg's own (already-working) track detection yet.
struct AudioStream {
    int id = 0;
    QString language;
    int codingMode = 0;
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

    // Declared streams for this title's VTS (see the structs above) -- read
    // from the IFO directly rather than relying on FFmpeg to discover them
    // empirically from the demuxed stream, which for DVD subpicture tracks
    // is unreliable (see PlayerController's DVD subtitle handling).
    QList<SubtitleStream> subtitleStreams;
    QList<AudioStream> audioStreams;

    // This title's PGC's own highlight/subtitle palette (16 YCrCb entries,
    // same pgc_t layout DvdMenu::Domain's menu PGCs use) -- needed to render
    // decoded subtitle subpictures, which are colour-index bitmaps until
    // mapped through this table.
    quint32 palette[16] = { 0 };
};

// Empty when the folder has no parseable IFO structure.
QList<Title> titles(const QString &videoTsDir);

} // namespace DvdIfo

#endif // DVDIFOPARSER_H
