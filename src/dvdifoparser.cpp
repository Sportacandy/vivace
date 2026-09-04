/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Offsets follow the DVD-Video specification as commonly documented
    (dvd.sourceforge.net "DVD-Video Information"); every read is bounds-
    checked and any anomaly abandons parsing (callers fall back to the
    size heuristic).
*/

#include "dvdifoparser.h"

#include <QDir>
#include <QFile>
#include <QHash>
#include <QtEndian>

namespace DvdIfo {

namespace {

constexpr qint64 sectorSize = 2048;

quint8 u8(const QByteArray &data, qint64 offset)
{
    return offset >= 0 && offset < data.size()
                   ? quint8(data.at(offset)) : 0;
}

quint16 be16(const QByteArray &data, qint64 offset)
{
    if (offset < 0 || offset + 2 > data.size())
        return 0;
    return qFromBigEndian<quint16>(data.constData() + offset);
}

quint32 be32(const QByteArray &data, qint64 offset)
{
    if (offset < 0 || offset + 4 > data.size())
        return 0;
    return qFromBigEndian<quint32>(data.constData() + offset);
}

int fromBcd(quint8 value)
{
    return (value >> 4) * 10 + (value & 0x0F);
}

// PGC playback time: BCD hh:mm:ss:ff with the frame rate in the top
// two bits of the frame byte (01 = 25 fps, 11 = 30 fps).
qint64 bcdTimeMs(quint32 time)
{
    const quint8 hours = quint8(time >> 24);
    const quint8 minutes = quint8(time >> 16);
    const quint8 seconds = quint8(time >> 8);
    const quint8 frameByte = quint8(time);

    const int rateBits = frameByte >> 6;
    const qreal fps = rateBits == 1 ? 25.0 : 29.97;
    const int frames = fromBcd(frameByte & 0x3F);

    return qint64(fromBcd(hours)) * 3600000
            + qint64(fromBcd(minutes)) * 60000
            + qint64(fromBcd(seconds)) * 1000
            + qint64(frames * 1000.0 / fps);
}

QByteArray readIfo(const QDir &dir, const QString &name)
{
    QFile file(dir.filePath(name));
    if (!file.open(QIODevice::ReadOnly)) {
        // Rips sometimes carry lowercase names.
        file.setFileName(dir.filePath(name.toLower()));
        if (!file.open(QIODevice::ReadOnly))
            return {};
    }
    return file.readAll();
}

// A language code is two ASCII letters when set; some rips leave it as
// 0x00 0x00 or spaces for an unused slot -- treat anything else as "unset"
// rather than surfacing garbage as a language name.
QString langCode(const QByteArray &data, qint64 offset)
{
    const quint8 a = u8(data, offset);
    const quint8 b = u8(data, offset + 1);
    if (a >= 'a' && a <= 'z' && b >= 'a' && b <= 'z')
        return QString(QLatin1Char(char(a))) + QLatin1Char(char(b));
    return {};
}

// VTSI_MAT's audio/subtitle attribute tables (offsets per the DVD-Video
// spec as commonly documented, dvd.sourceforge.net "DVD-Video Information"
// -- verified directly against a real disc's IFO bytes, not just the docs):
// audio count at 0x202 (2 bytes), 8-byte entries from 0x204, language at
// entry+2; subtitle count at 0x254 (2 bytes), 6-byte entries from 0x256,
// language at entry+2.
//
// `pgc` (the SAME entry-PGC offset the caller already resolved for
// duration/palette/cells) additionally resolves each logical subtitle
// stream's REAL physical sub-stream id via the PGC's own subp_control[32]
// table (pgc_t, 4-byte big-endian entries at pgc + 0x1C: one byte per
// display mode, each byte = 1 available-bit + 7-bit physical id). Byte1
// (bits 23-16) is the slot to trust -- confirmed against two real discs
// (see SubtitleStream::physicalId's own doc comment for the full
// investigation): it's the identity mapping on a disc with no real
// remapping, and the genuinely-different real id on one that does remap.
void parseStreamTables(const QByteArray &vts, qint64 pgc,
                       QList<AudioStream> &audio,
                       QList<SubtitleStream> &subtitle)
{
    const int audioCount = qBound(0, int(be16(vts, 0x202)), 8);
    for (int i = 0; i < audioCount; ++i) {
        const qint64 entry = 0x204 + qint64(i) * 8;
        AudioStream a;
        a.id = i;
        a.codingMode = (u8(vts, entry) >> 5) & 0x7;
        a.language = langCode(vts, entry + 2);
        audio.append(a);
    }

    const int subpCount = qBound(0, int(be16(vts, 0x254)), 32);
    for (int i = 0; i < subpCount; ++i) {
        const qint64 entry = 0x256 + qint64(i) * 6;
        SubtitleStream s;
        s.id = i;
        s.language = langCode(vts, entry + 2);
        // subp_control[i]'s own byte0 top bit says whether this ENTRY was
        // authored at all -- found necessary 2026-09-04, regression-testing
        // against real discs whose subtitle selection already worked
        // correctly before this fix: a disc can leave one stream's whole
        // 4-byte entry all-zero (e.g. "ルパン三世 カリオストロの城"'s English
        // track), and byte1 being 0 there is NOT "physical id 0" (which
        // would collide with byte1's real, populated id-0 for a DIFFERENT
        // stream) -- it means "no override was authored, use the plain
        // logical index" (SubtitleStream::physicalId's own -1 sentinel,
        // which the caller already falls back on). Only trust byte1 when
        // byte0's own avail bit is set, matching every disc's authored
        // entries (both identity-mapped and genuinely remapped ones).
        const quint8 byte0 = u8(vts, pgc + 0x1C + qint64(i) * 4);
        if (byte0 & 0x80)
            s.physicalId = u8(vts, pgc + 0x1C + qint64(i) * 4 + 1) & 0x7F;
        subtitle.append(s);
    }
}

} // namespace

QList<Title> titles(const QString &videoTsDir)
{
    const QDir dir(videoTsDir);

    const QByteArray vmg = readIfo(dir, QStringLiteral("VIDEO_TS.IFO"));
    if (!vmg.startsWith(QByteArrayLiteral("DVDVIDEO-VMG")))
        return {};

    // TT_SRPT: table of titles.
    const qint64 ttSrpt = qint64(be32(vmg, 0xC4)) * sectorSize;
    const int titleCount = be16(vmg, ttSrpt);
    if (titleCount <= 0 || titleCount > 99)
        return {};

    QHash<int, QByteArray> vtsCache;
    QList<Title> out;

    for (int i = 0; i < titleCount; ++i) {
        const qint64 entry = ttSrpt + 8 + qint64(i) * 12;
        const int vtsNumber = u8(vmg, entry + 6);
        const int vtsTitleNumber = u8(vmg, entry + 7);
        if (vtsNumber < 1 || vtsNumber > 99 || vtsTitleNumber < 1)
            continue;

        if (!vtsCache.contains(vtsNumber)) {
            vtsCache.insert(vtsNumber,
                            readIfo(dir, QStringLiteral("VTS_%1_0.IFO")
                                    .arg(vtsNumber, 2, 10, u'0')));
        }
        const QByteArray &vts = vtsCache.value(vtsNumber);
        if (!vts.startsWith(QByteArrayLiteral("DVDVIDEO-VTS")))
            continue;

        // VTS_PTT_SRPT: title -> entry PGC.
        const qint64 ptt = qint64(be32(vts, 0xC8)) * sectorSize;
        const int pttTitles = be16(vts, ptt);
        if (vtsTitleNumber > pttTitles)
            continue;
        const qint64 titleOffset =
                ptt + qint64(be32(vts, ptt + 8 + qint64(vtsTitleNumber - 1) * 4));
        const int pgcNumber = be16(vts, titleOffset);
        if (pgcNumber < 1)
            continue;

        // VTS_PGCIT: the program chain itself.
        const qint64 pgcit = qint64(be32(vts, 0xCC)) * sectorSize;
        const int pgcCount = be16(vts, pgcit);
        if (pgcNumber > pgcCount)
            continue;
        const qint64 pgc = pgcit
                + qint64(be32(vts, pgcit + 8 + qint64(pgcNumber - 1) * 8 + 4));

        Title title;
        title.titleNumber = i + 1;
        title.vtsNumber = vtsNumber;
        title.vtsTitleNumber = vtsTitleNumber;
        title.durationMs = bcdTimeMs(be32(vts, pgc + 4));
        parseStreamTables(vts, pgc, title.audioStreams, title.subtitleStreams);
        // Highlight/subtitle palette: 16 entries of (reserved, Y, Cr, Cb) at
        // PGC + 0xA4 (same pgc_t layout dvdmenuparser.cpp uses for menus).
        for (int p = 0; p < 16; ++p)
            title.palette[p] = be32(vts, pgc + 0xA4 + qint64(p) * 4) & 0xFFFFFF;

        // Command table (PGC + 0xE4): pre/post/cell command counts, then
        // 8-byte VM commands each -- same layout dvdmenuparser.cpp already
        // reads for menu PGCs. Only post-commands are needed here (see
        // Title::postCommands' own doc comment).
        if (be16(vts, pgc + 0xE4) != 0) {
            const qint64 cmdTable = pgc + be16(vts, pgc + 0xE4);
            const int nPre = be16(vts, cmdTable);
            const int nPost = be16(vts, cmdTable + 2);
            qint64 cmdPos = cmdTable + 8 + qint64(nPre) * 8;
            for (int c = 0; c < nPost && c < 128; ++c, cmdPos += 8) {
                if (cmdPos + 8 > vts.size())
                    break;
                title.postCommands.append(vts.mid(cmdPos, 8));
            }
        }

        // PGC offsets: 0xE4 commands, 0xE6 program map,
        // 0xE8 cell playback info table, 0xEA cell position info table.
        const int cellCount = u8(vts, pgc + 3);
        const qint64 cellTable = pgc + be16(vts, pgc + 0xE8);
        const qint64 positionTable = pgc + be16(vts, pgc + 0xEA);
        QList<qint64> cellStartMs(cellCount + 2, 0); // 1-based entry cells
        QList<int> cellIncludedIndex(cellCount + 2, 0);
        qint64 accumulatedMs = 0;
        int previousVobId = -1;
        for (int c = 0; c < cellCount; ++c) {
            const qint64 cell = cellTable + qint64(c) * 24;
            cellStartMs[c + 1] = accumulatedMs;
            cellIncludedIndex[c + 1] = int(title.cells.size());

            const quint8 category = u8(vts, cell);
            const int blockMode = (category >> 6) & 0x3;
            const int blockType = (category >> 4) & 0x3;
            // Angle blocks: keep the first angle only.
            if (blockType == 1 && blockMode != 1)
                continue;

            Cell range;
            range.firstSector = be32(vts, cell + 0x08);
            range.lastSector = be32(vts, cell + 0x14);
            if (range.lastSector < range.firstSector)
                continue;

            // New timestamp timeline? (libdvdread cell_playback_t: byte 0
            // bit 1 = stc_discontinuity; position table gives the logical
            // VOB unit, whose change also restarts the clock.)
            const bool stcDiscontinuity = (category & 0x02) != 0;
            const int vobId = be16(vts, positionTable + qint64(c) * 4);
            const bool newTimeline = !title.cells.isEmpty()
                    && (stcDiscontinuity || vobId != previousVobId);
            previousVobId = vobId;

            title.cells.append(range);
            title.cellStartsMs.append(accumulatedMs);
            title.cellNewTimeline.append(newTimeline);
            accumulatedMs += bcdTimeMs(be32(vts, cell + 0x04));
        }

        // Chapters: the PGC program map holds each program's entry cell.
        const int programCount = u8(vts, pgc + 2);
        const qint64 programMap = pgc + be16(vts, pgc + 0xE6);
        for (int p = 0; p < programCount; ++p) {
            const int entryCell = u8(vts, programMap + p);
            if (entryCell >= 1 && entryCell <= cellCount) {
                title.chapterStartsMs.append(cellStartMs.at(entryCell));
                title.chapterCellIndexes.append(
                        qMin(cellIncludedIndex.at(entryCell),
                             int(title.cells.size()) - 1));
            }
        }

        // VTS_TMAPT (sector pointer at 0xD4, per libdvdread's vts_mat_t):
        // per-PGC time map for byte-accurate seeking. Entry i = VOBU
        // sector at (i+1)*tmu sec; bit 31 flags a discontinuity and is
        // masked off by the user.
        const qint64 tmapti = qint64(be32(vts, 0xD4)) * sectorSize;
        if (tmapti > 0) {
            const int tmapCount = be16(vts, tmapti);
            if (pgcNumber <= tmapCount) {
                const qint64 tmap = tmapti
                        + qint64(be32(vts, tmapti + 8
                                              + qint64(pgcNumber - 1) * 4));
                const int unit = u8(vts, tmap);
                const int entryCount = be16(vts, tmap + 2);
                if (unit > 0 && entryCount > 0) {
                    title.timeMapUnitSec = unit;
                    for (int e = 0; e < entryCount; ++e)
                        title.timeMapSectors.append(
                                be32(vts, tmap + 4 + qint64(e) * 4));
                }
            }
        }

        if (!title.cells.isEmpty() && title.durationMs > 0)
            out.append(title);
    }

    return out;
}

} // namespace DvdIfo
