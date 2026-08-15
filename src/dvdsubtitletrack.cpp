/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "dvdsubtitletrack.h"

#include "dvdtitledevice.h"

#include <QScopedPointer>

namespace DvdSubtitle {

namespace {

constexpr qint64 sectorSize = 2048;

quint16 be16(const QByteArray &d, qint64 o)
{
    if (o < 0 || o + 2 > d.size())
        return 0;
    return quint16((quint8(d.at(o)) << 8) | quint8(d.at(o + 1)));
}

quint8 u8(const QByteArray &d, qint64 o)
{
    return (o >= 0 && o < d.size()) ? quint8(d.at(o)) : 0;
}

// 90kHz PES PTS, standard 5-byte packed format ('0010'/'0011' PTS[32:30]
// marker PTS[29:15] marker PTS[14:0] marker), starting right after the
// mandatory 3-byte PES header (flags/PES_header_data_length) at j+6..j+8.
// -1 when PTS_DTS_flags says no PTS is present.
qint64 ptsFromPesHeader(const QByteArray &d, qint64 j)
{
    if ((u8(d, j + 7) >> 6) < 2) // 00 = none, 01 = forbidden/reserved
        return -1;
    const quint8 b0 = u8(d, j + 9), b1 = u8(d, j + 10), b2 = u8(d, j + 11),
                 b3 = u8(d, j + 12), b4 = u8(d, j + 13);
    return (qint64((b0 >> 1) & 0x7) << 30) | (qint64(b1) << 22)
            | (qint64(b2 >> 1) << 15) | (qint64(b3) << 7) | qint64(b4 >> 1);
}

// Walk the SPU's SP_DCSQ chain (bounded, malformed-data safe) looking for a
// control sequence containing STP_DSP (stop display); returns its DELAY --
// units of 1024/90000 sec, per the DVD-Video spec -- or -1 if none is found
// (the event then runs until the next one starts, a reasonable fallback).
qint64 spuStopDelayTicks(const QByteArray &spu)
{
    if (spu.size() < 4)
        return -1;
    qint64 ctrl = be16(spu, 2);
    for (int hop = 0; hop < 8 && ctrl + 4 <= spu.size(); ++hop) {
        const qint64 delay = be16(spu, ctrl);
        const qint64 next = be16(spu, ctrl + 2);
        qint64 p = ctrl + 4;
        bool sawStop = false;
        while (p < spu.size()) {
            const int cmd = u8(spu, p);
            if (cmd == 0xFF)
                break;
            switch (cmd) {
            case 0x00: case 0x01: ++p; break;
            case 0x02: sawStop = true; ++p; break;
            case 0x03: case 0x04: p += 3; break;
            case 0x05: p += 7; break;
            case 0x06: p += 5; break;
            default: p = spu.size(); break;
            }
        }
        if (sawStop)
            return delay;
        if (next == ctrl || next <= 0)
            break;
        ctrl = next;
    }
    return -1;
}

// Accurate sector -> ms lookup using the disc's own VTS_TMAPT time map
// (entry i = the VOBU sector at real time (i+1)*timeMapUnitSec, masked to
// drop the discontinuity flag in bit 31 -- same convention
// PlayerController::seekDvd() already uses). `absSector` is in the disc's
// own absolute numbering (matching Cell::firstSector/lastSector), NOT
// run-relative. Returns -1 if no time map is available or the entries
// don't bracket usefully, so the caller can fall back to the cruder
// proportional-within-cell estimate.
qint64 msFromTimeMap(quint32 absSector, int timeMapUnitSec,
                     const QList<quint32> &timeMapSectors)
{
    if (timeMapUnitSec <= 0 || timeMapSectors.isEmpty())
        return -1;
    const qint64 unitMs = qint64(timeMapUnitSec) * 1000;

    // Binary search for the rightmost entry whose sector is <= absSector
    // (timeMapSectors is sorted ascending by construction -- consecutive
    // VOBU sectors through playback order).
    int lo = 0, hi = timeMapSectors.size() - 1, found = -1;
    while (lo <= hi) {
        const int mid = lo + (hi - lo) / 2;
        const quint32 s = timeMapSectors.at(mid) & 0x7FFFFFFF;
        if (s <= absSector) {
            found = mid;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    if (found < 0) {
        // Before the first sampled point: interpolate from the implicit
        // (sector 0, time 0) origin to entry 0.
        const quint32 s0 = timeMapSectors.at(0) & 0x7FFFFFFF;
        return s0 > 0 ? qint64(absSector) * unitMs / s0 : 0;
    }
    const qint64 t0 = qint64(found + 1) * unitMs;
    if (found + 1 >= timeMapSectors.size())
        return t0; // beyond the last sampled point -- rare tail case
    const quint32 s0 = timeMapSectors.at(found) & 0x7FFFFFFF;
    const quint32 s1 = timeMapSectors.at(found + 1) & 0x7FFFFFFF;
    if (s1 <= s0)
        return t0;
    const qint64 t1 = qint64(found + 2) * unitMs;
    return t0 + (t1 - t0) * qint64(absSector - s0) / qint64(s1 - s0);
}

} // namespace

Track Track::build(const QString &videoTsDir, int vtsNumber,
                   const QList<DvdIfo::Cell> &cells,
                   const QList<qint64> &cellStartsMs, qint64 runDurationMs,
                   int streamId, int timeMapUnitSec,
                   const QList<quint32> &timeMapSectors, qint64 runBaseMs)
{
    Track track;
    if (cells.isEmpty() || cells.size() != cellStartsMs.size())
        return track;
    const int targetSubId = 0x20 + streamId;

    QScopedPointer<DvdTitleDevice> device(
            DvdTitleDevice::createFromCells(videoTsDir, vtsNumber, cells));
    if (!device || !device->open(QIODevice::ReadOnly))
        return track;

    // Cumulative sector count at the start of each cell, to interpolate a
    // stream sector index back to "ms since this run's own start" using the
    // same cell-duration math the rest of the DVD engine already trusts
    // (dvdifoparser.cpp) -- not the SPU's own raw PTS, which would need its
    // own per-run zero-point that isn't otherwise needed here.
    QList<qint64> cellStartSector(cells.size());
    qint64 acc = 0;
    for (int i = 0; i < cells.size(); ++i) {
        cellStartSector[i] = acc;
        acc += cells.at(i).lastSector - cells.at(i).firstSector + 1;
    }
    auto msAtSector = [&](qint64 sectorIndex) -> qint64 {
        int k = 0;
        while (k + 1 < cells.size() && sectorIndex >= cellStartSector.at(k + 1))
            ++k;
        const qint64 into = sectorIndex - cellStartSector.at(k);
        // Prefer the disc's own time map -- accurate to its sampling
        // granularity (typically 5 real seconds) regardless of how the
        // bitrate varies scene to scene within this cell.
        const quint32 absSector = quint32(cells.at(k).firstSector + into);
        const qint64 mapped =
                msFromTimeMap(absSector, timeMapUnitSec, timeMapSectors);
        if (mapped >= 0)
            return mapped - runBaseMs; // absolute -> run-relative

        // Fallback: assume constant bitrate across the whole cell (only
        // reached for a disc/PGC with no time map at all).
        const qint64 cellSectors =
                cells.at(k).lastSector - cells.at(k).firstSector + 1;
        const qint64 cellStart = cellStartsMs.at(k);
        const qint64 cellEnd =
                (k + 1 < cells.size()) ? cellStartsMs.at(k + 1) : runDurationMs;
        if (cellSectors <= 0)
            return cellStart;
        return cellStart
                + (cellEnd - cellStart) * into / qMax<qint64>(1, cellSectors);
    };

    QByteArray spu;
    qint64 declared = -1;
    qint64 unitStartSector = -1;

    qint64 sectorIndex = 0;
    QByteArray sector(int(sectorSize), Qt::Uninitialized);
    while (device->read(sector.data(), sectorSize) == sectorSize) {
        int i = 0;
        while (true) {
            const int j = sector.indexOf(QByteArrayLiteral("\x00\x00\x01\xBD"), i);
            if (j < 0)
                break;
            const int pesLen = be16(sector, j + 4);
            const int hdrLen = u8(sector, j + 8);
            const int payloadStart = j + 9 + hdrLen;
            const int payloadEnd = j + 6 + pesLen;
            if (payloadStart < payloadEnd && payloadEnd <= sector.size()
                && u8(sector, payloadStart) == targetSubId) {
                if (spu.isEmpty())
                    unitStartSector = sectorIndex;
                spu.append(sector.constData() + payloadStart + 1,
                          payloadEnd - payloadStart - 1);
                if (declared < 0 && spu.size() >= 2)
                    declared = be16(spu, 0);
                if (declared > 0 && spu.size() >= declared) {
                    Event ev;
                    ev.startMs = msAtSector(unitStartSector);
                    ev.spu = spu.left(declared);
                    track.m_events.append(ev);
                    spu.clear();
                    declared = -1;
                }
            }
            i = j + 4;
        }
        ++sectorIndex;
    }

    for (int i = 0; i < track.m_events.size(); ++i) {
        Event &ev = track.m_events[i];
        const qint64 stopDelay = spuStopDelayTicks(ev.spu);
        if (stopDelay >= 0)
            ev.stopMs = ev.startMs + stopDelay * 1024 / 90;
        else if (i + 1 < track.m_events.size())
            ev.stopMs = track.m_events.at(i + 1).startMs;
    }

    return track;
}

DvdMenu::Subpicture Track::activeAt(qint64 positionMs)
{
    int found = -1;
    for (int i = m_events.size() - 1; i >= 0; --i) {
        if (m_events.at(i).startMs <= positionMs) {
            if (m_events.at(i).stopMs < 0 || positionMs < m_events.at(i).stopMs)
                found = i;
            break; // events are in ascending startMs order
        }
    }
    if (found == m_lastIndex)
        return m_lastDecoded;
    m_lastIndex = found;
    m_lastDecoded = (found >= 0) ? DvdMenu::decodeSpuBytes(m_events.at(found).spu)
                                 : DvdMenu::Subpicture();
    return m_lastDecoded;
}

} // namespace DvdSubtitle
