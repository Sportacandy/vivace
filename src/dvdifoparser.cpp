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
