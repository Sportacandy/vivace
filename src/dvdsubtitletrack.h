/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef DVDSUBTITLETRACK_H
#define DVDSUBTITLETRACK_H

#include <QByteArray>
#include <QList>
#include <QString>

#include "dvdifoparser.h"
#include "dvdspu.h"

/*  Real DVD movie-subtitle (subpicture) playback.

    Unlike a menu's single still SPU, a title's subtitle stream is many SPU
    units scattered through playback, each shown for its own time span. Qt
    Multimedia's FFmpeg backend does not reliably surface DVD subpicture
    streams as playable tracks when fed Vivace's assembled cell/sector VOB
    data (confirmed against a real disc: 0 subtitle tracks detected despite
    the IFO declaring 2) -- so this demuxes and decodes them directly from
    the same IFO-described cell data, the same approach mpv/libdvdnav take,
    rather than depending on FFmpeg's track model at all.
*/
namespace DvdSubtitle {

struct Event {
    qint64 startMs = 0;
    qint64 stopMs = -1; // -1 = until the next event (or end of title)
    QByteArray spu;      // raw, already size-trimmed SPU unit bytes
};

class Track {
public:
    // Scans the title-set VOBs (VTS_<vtsNumber>_1..9.VOB) across `cells` --
    // the specific cell run actually being played, i.e. the same slice
    // PlayerController::applyDvdTitle() feeds to DvdTitleDevice, not
    // necessarily the whole title -- for private_stream_1 packets whose
    // sub-stream ID is 0x20 + streamId. Each SPU unit's event time is
    // interpolated from its sector position using the disc's own VTS_TMAPT
    // time map (timeMapUnitSec/timeMapSectors, the SAME per-PGC data
    // PlayerController::seekDvd() already trusts for accurate seeking) when
    // one is available; falling back to a crude proportional-within-cell
    // estimate otherwise. The time map is real, densely-sampled (typically
    // every 5 real seconds) data reflecting the disc's actual, non-constant
    // bitrate -- the fallback assumes a CONSTANT bitrate across an entire
    // cell, which can be many minutes long and is frequently very wrong
    // (found 2026-08-15: a whole cluster of real subtitle events computed
    // 35+ seconds later than their true position with the fallback method,
    // making them appear to not show at all during the real dialogue and
    // then show stale/wrong text later). Not parsed from the stream's own
    // raw PTS, which would need its own per-run zero-point and isn't needed
    // once real disc timing is available. `cellStartsMs`/`runDurationMs`
    // must already be relative to this run's own start (ms 0), matching
    // what QMediaPlayer::position() reports for the assembled stream;
    // `timeMapSectors` stays in the disc's own ABSOLUTE sector numbering
    // (matching `cells`' firstSector/lastSector), not run-relative;
    // `runBaseMs` is the run's own true start in that SAME absolute
    // timeline (title->cellStartsMs.at(fromCellIndex), before the caller's
    // own run-relative adjustment to `cellStartsMs`), used to convert a
    // time-map-derived absolute time back into the run-relative coordinate
    // every other event time here uses.
    static Track build(const QString &videoTsDir, int vtsNumber,
                       const QList<DvdIfo::Cell> &cells,
                       const QList<qint64> &cellStartsMs, qint64 runDurationMs,
                       int streamId, int timeMapUnitSec = 0,
                       const QList<quint32> &timeMapSectors = {},
                       qint64 runBaseMs = 0);

    bool isEmpty() const { return m_events.isEmpty(); }
    int eventCount() const { return m_events.size(); }

    // The subpicture active at `positionMs` (ms since this run's own start),
    // or an invalid Subpicture if none is showing right now. Decodes lazily
    // and caches the last hit, so repeated calls at nearby positions (e.g.
    // a UI timer polling every few hundred ms) don't redecode every time.
    DvdMenu::Subpicture activeAt(qint64 positionMs);

    // The event index resolved by the most recent activeAt() call (-2 before
    // any call, -1 = "no event active"). Diagnostic-only accessor.
    int lastResolvedIndex() const { return m_lastIndex; }

    // Diagnostic-only: the full built event list, for logging/inspection.
    const QList<Event> &events() const { return m_events; }

private:
    QList<Event> m_events;
    int m_lastIndex = -2; // -2 = nothing decoded yet; -1 = "no event" cached
    DvdMenu::Subpicture m_lastDecoded;
};

} // namespace DvdSubtitle

#endif // DVDSUBTITLETRACK_H
