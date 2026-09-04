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
    // cell_playback_t's own still_time (offset +0x02 within the 24-byte
    // cell entry -- a SEPARATE field from the whole-PGC still_time at
    // PGC + 0xA2): 0 = play straight through into the next cell/PGC end;
    // 1-254 = hold this cell's last frame that many seconds; 0xFF = hold
    // forever until the user acts. Verified against two real discs
    // (2026-08-15): this per-cell field, not the PGC-level one, is what
    // actually distinguishes "plays once then auto-advances" from "must
    // wait for the user" -- a real interactive multi-button menu and a
    // skippable logo-animation-with-a-dummy-button can both have PGC-level
    // still_time == 0, but the menu's own cell has still_time == 0xFF.
    int stillTime = 0;
    // This cell's own playback duration (BCD hh:mm:ss:ff at cell + 0x04,
    // same encoding as PGC/title durations elsewhere in this project).
    // Only populated by dvdmenuparser.cpp so far (menu cells can need to
    // freeze partway through a multi-cell PGC -- see the 2026-08-15 fix
    // notes on why a single whole-PGC "last cell" assumption isn't
    // enough); title cells still get their timing from
    // DvdIfo::Title::cellStartsMs instead.
    qint64 durationMs = 0;
};

// VTSI_MAT's subtitle (subpicture) stream attribute table: declared once per
// VTS, shared by every title in it. `id` is this stream's 0-based LOGICAL
// index (0..31) -- the number a menu/VM SPRM2 selection or this struct's own
// array position refers to. It is NOT necessarily the physical private_
// stream_1 sub-stream ID a decoder must pattern-match (0x20 + physicalId):
// the PGC's own subp_control[32] table (pgc_t, offset +0x1C, 4 bytes per
// logical entry) can remap a logical subtitle stream onto a DIFFERENT
// physical stream per display mode -- found 2026-09-04 on a real disc
// ("リトル・ロマンス") whose 8 logical streams (id 0..7) map onto 16 real
// physical streams (physicalId 0..15, two per language: a "wide" and a
// "letterbox" copy, byte1/byte2 of the 4-byte entry respectively -- Japanese
// on that disc even had DIFFERENT content per copy, horizontal vs vertical
// text). Confirmed against real SPU bitmap data (decoded and read the actual
// glyphs) that byte1 (bits 23-16 of the big-endian subp_control value) is
// the slot to use: on a "simple" disc whose subp_control happens to be the
// identity mapping (byte1 == logical id, e.g. "あなただけ今晩は"/Irma la
// Douce, already verified working), it's a no-op; on the doubled disc above
// it resolves to the REAL id (byte1 == 2 * logical id) instead of the wrong,
// unrelated stream the raw logical index would hit. `physicalId` holds this
// resolved value once a title's PGC has been parsed (see parseStreamTables()
// in dvdifoparser.cpp), but ONLY when the disc actually authored an override
// for this logical stream (subp_control[i]'s own byte0 top bit set) -- a
// disc can legitimately leave one stream's whole 4-byte entry all-zero
// (found on "ルパン三世 カリオストロの城"'s English track), which means "no
// override, use the plain logical index" and must NOT be read as "physical
// id 0" (that would collide with whichever OTHER stream's real, authored id
// 0 happens to be). `physicalId` stays -1 in that case, same as before any
// title has loaded, meaning "fall back to `id`" (also still true for the
// menu-domain parsing paths, which don't resolve subp_control at all).
struct SubtitleStream {
    int id = 0;
    int physicalId = -1; // resolved PES sub-stream id (0x20 + physicalId); -1 = unresolved/unauthored, use `id`
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

    // This title's PGC's own post-commands (each 8 bytes, same command
    // table layout dvdmenuparser.cpp already reads for menu PGCs) -- run
    // through the DVD VM when the title naturally finishes playing (its
    // last cell/run ends), instead of always just stopping. Needed for
    // discs where a short mandatory clip (an anti-piracy warning, a studio
    // logo, ...) is itself an ordinary TITLE (not a menu PGC) whose own
    // post-commands are what actually chains onward into the disc's real
    // menu system or the main feature -- found 2026-08-16 on a disc (Cars)
    // whose interactive main menu is reachable only after such a chain of
    // plain, non-menu titles. Most titles have none (empty), in which case
    // playback simply stops at the end exactly as before.
    QList<QByteArray> postCommands;
};

// Empty when the folder has no parseable IFO structure.
QList<Title> titles(const QString &videoTsDir);

} // namespace DvdIfo

#endif // DVDIFOPARSER_H
