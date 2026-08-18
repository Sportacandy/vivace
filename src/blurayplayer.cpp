/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "blurayplayer.h"

#include <QDir>
#include <QFileInfo>
#include <QDebug>

#ifdef VIVACE_HAVE_BLURAY
#include <libbluray/bluray.h>
#include <climits>
#include <cstring>
#endif

bool BlurayDisc::isBlurayFolder(const QString &path)
{
    // A pure filesystem check -- always available, even in a build without
    // libbluray, so callers can still tell "this looks like a BD disc, but
    // this copy of Vivace has no Blu-ray support" apart from "not a disc at
    // all". BDMV/index.bdmv is the standard BD-ROM root marker (same role
    // VIDEO_TS.IFO plays for DVD).
    const QDir dir(path);
    return dir.exists(QStringLiteral("BDMV/index.bdmv"))
        || dir.exists(QStringLiteral("BDMV/INDEX.BDMV"));
}

BlurayDisc::BlurayDisc() = default;

BlurayDisc::~BlurayDisc()
{
    close();
}

#ifdef VIVACE_HAVE_BLURAY

bool BlurayDisc::open(const QString &path)
{
    close();

    // libbluray's public API takes a plain const char*; UTF-8 is what its
    // own Windows file backend expects a non-ASCII path to be encoded as
    // (matching how VLC/mpv pass paths to it) -- NOT verified against real
    // non-ASCII hardware in this environment, flagged as a follow-up.
    BLURAY *bd = bd_open(path.toUtf8().constData(), nullptr);
    if (!bd)
        return false;

    const uint32_t count = bd_get_titles(bd, TITLES_RELEVANT, 0);
    if (count == 0) {
        bd_close(bd);
        return false;
    }

    m_bd = bd;
    m_mainTitleListIndex = -1;
    const int main = bd_get_main_title(bd);

    for (uint32_t i = 0; i < count; ++i) {
        BLURAY_TITLE_INFO *info = bd_get_title_info(bd, i, 0);
        if (!info)
            continue;

        Title title;
        title.libraryIndex = i;
        title.playlistId = info->playlist;
        title.durationMs = static_cast<qint64>(info->duration) / 90; // 90 kHz -> ms

        for (uint32_t c = 0; c < info->chapter_count; ++c) {
            Chapter chapter;
            chapter.startMs = static_cast<qint64>(info->chapters[c].start) / 90;
            chapter.label = QObject::tr("Chapter %1").arg(c + 1);
            title.chapters.append(chapter);
        }

        if (info->clip_count > 0) {
            const BLURAY_CLIP_INFO &clip = info->clips[0];
            for (int a = 0; a < clip.audio_stream_count; ++a) {
                title.audioLanguages.append(QString::fromLatin1(
                        reinterpret_cast<const char *>(clip.audio_streams[a].lang), 4)
                        .trimmed().remove(QChar(u'\0')));
                title.audioCodingTypes.append(clip.audio_streams[a].coding_type);
            }
            for (int p = 0; p < clip.pg_stream_count; ++p) {
                title.subtitleLanguages.append(QString::fromLatin1(
                        reinterpret_cast<const char *>(clip.pg_streams[p].lang), 4)
                        .trimmed().remove(QChar(u'\0')));
            }
        }

        // One ClipRun per clip -- see the struct's own doc comment for the
        // PCR/PTS/DTS rewrite scheme that lets a multi-clip title be served
        // to QMediaPlayer as a single, gapless, continuously-timestamped
        // device. byteSize uses the standard BDAV-TS packet size (188-byte
        // MPEG-TS packet + 4-byte TP_extra_header = 192 bytes) -- the same
        // convention already confirmed elsewhere in this project for
        // BD-sourced streams.
        qint64 cumulativeBytes = 0;
        for (unsigned c = 0; c < info->clip_count; ++c) {
            const BLURAY_CLIP_INFO &clip = info->clips[c];
            ClipRun run;
            run.byteOffset = cumulativeBytes;
            run.byteSize = static_cast<qint64>(clip.pkt_count) * 192;
            run.startMs = static_cast<qint64>(clip.start_time) / 90;
            run.durationMs = static_cast<qint64>(clip.out_time - clip.in_time) / 90;
            run.offsetUnits = static_cast<qint64>(clip.start_time) - static_cast<qint64>(clip.in_time);
            title.clipRuns.append(run);
            cumulativeBytes += run.byteSize;
        }

        m_titles.append(title);
        if (main >= 0 && static_cast<uint32_t>(main) == i)
            m_mainTitleListIndex = m_titles.size() - 1;

        bd_free_title_info(info);
    }

    if (m_titles.isEmpty()) {
        close();
        return false;
    }
    if (m_mainTitleListIndex < 0)
        m_mainTitleListIndex = 0; // fall back to the first title

    return true;
}

void BlurayDisc::close()
{
    if (m_bd) {
        bd_close(reinterpret_cast<BLURAY *>(m_bd));
        m_bd = nullptr;
    }
    m_titles.clear();
    m_mainTitleListIndex = -1;
    m_selectedTitleListIndex = -1;
}

bool BlurayDisc::selectTitle(int titleListIndex)
{
    if (!m_bd || titleListIndex < 0 || titleListIndex >= m_titles.size())
        return false;
    BLURAY *bd = reinterpret_cast<BLURAY *>(m_bd);
    if (!bd_select_title(bd, m_titles.at(titleListIndex).libraryIndex))
        return false;
    m_selectedTitleListIndex = titleListIndex;
    return true;
}

QIODevice *BlurayDisc::createDevice(QObject *parent)
{
    if (!m_bd || m_selectedTitleListIndex < 0)
        return nullptr;
    return new BlurayTitleDevice(m_bd, m_titles.at(m_selectedTitleListIndex).clipRuns, parent);
}

qint64 BlurayDisc::currentTitleSizeBytes() const
{
    if (!m_bd || m_selectedTitleListIndex < 0)
        return 0;
    return static_cast<qint64>(bd_get_title_size(reinterpret_cast<BLURAY *>(m_bd)));
}

#else // !VIVACE_HAVE_BLURAY

bool BlurayDisc::open(const QString &) { return false; }
void BlurayDisc::close() { m_titles.clear(); m_mainTitleListIndex = -1; m_selectedTitleListIndex = -1; }
bool BlurayDisc::selectTitle(int) { return false; }
QIODevice *BlurayDisc::createDevice(QObject *) { return nullptr; }
qint64 BlurayDisc::currentTitleSizeBytes() const { return 0; }

#endif // VIVACE_HAVE_BLURAY

BlurayTitleDevice::BlurayTitleDevice(void *bd, QList<BlurayDisc::ClipRun> clips, QObject *parent)
    : QIODevice(parent), m_bd(bd), m_clips(std::move(clips))
{
    for (const BlurayDisc::ClipRun &c : m_clips)
        m_size += c.byteSize;
}

bool BlurayTitleDevice::open(OpenMode mode)
{
    // NOT Unbuffered: readData() below produces rewritten bytes in whatever
    // chunks bd_read() happens to hand back, buffered into m_leftover and
    // doled out to the caller from there -- letting QIODevice's own base
    // class request larger chunks than any one caller-visible read() is
    // simpler than reimplementing that buffering ourselves too.
    return QIODevice::open(mode);
}

void BlurayTitleDevice::close()
{
    QIODevice::close();
}

bool BlurayTitleDevice::seek(qint64 pos)
{
#ifdef VIVACE_HAVE_BLURAY
    if (!m_bd)
        return false;
    const int64_t result = bd_seek(reinterpret_cast<BLURAY *>(m_bd), static_cast<uint64_t>(pos));
    if (result < 0)
        return false;
    m_readCursor = pos;
    m_currentClip = clipIndexForByte(pos);
    m_leftover.clear(); // any buffered lookahead was rewritten for the OLD position
    return QIODevice::seek(pos);
#else
    Q_UNUSED(pos);
    return false;
#endif
}

int BlurayTitleDevice::clipIndexForByte(qint64 absPos)
{
    // Reads are normally sequential, so the common case is "still within
    // m_currentClip" or "just crossed into the next one" -- only fall back
    // to a full search (e.g. right after seek()) when that's not true.
    if (m_currentClip >= 0 && m_currentClip < m_clips.size()) {
        const auto &cur = m_clips.at(m_currentClip);
        if (absPos >= cur.byteOffset && absPos < cur.byteOffset + cur.byteSize)
            return m_currentClip;
        if (m_currentClip + 1 < m_clips.size()) {
            const auto &next = m_clips.at(m_currentClip + 1);
            if (absPos >= next.byteOffset && absPos < next.byteOffset + next.byteSize)
                return m_currentClip + 1;
        }
    }
    for (int i = 0; i < m_clips.size(); ++i) {
        const auto &c = m_clips.at(i);
        if (absPos >= c.byteOffset && absPos < c.byteOffset + c.byteSize)
            return i;
    }
    return qMax(0, m_clips.size() - 1); // past the end -- clamp, readData() will hit EOF anyway
}

namespace {

// Rewrites the PCR (adaptation field) and any PES PTS/DTS found in ONE
// 192-byte BDAV-TS packet by adding offsetUnits (90 kHz units) to each,
// modulo 2^33 (PTS/DTS/PCR-base are all 33-bit clocks) -- so a title's
// clips, each with their own independent timestamp domain, present one
// continuously increasing timeline end to end. Never changes the packet's
// length; unrecognized/malformed structure is left untouched (defensive:
// this is the difference between "plays correctly" and "silently corrupts
// unrelated payload bytes" for a stream Vivace itself does not author).
//
// Layout reference (ISO/IEC 13818-1): the 192-byte packet is a 4-byte
// BD "TP_extra_header" (ignored -- FFmpeg's mpegts demuxer already treats
// M2TS's 192-byte packets by skipping these 4 bytes itself) followed by an
// ordinary 188-byte MPEG-TS packet.
void rewriteTsPacketTimestamps(quint8 *pkt192, qint64 offsetUnits)
{
    quint8 *ts = pkt192 + 4;
    if (ts[0] != 0x47) // sync_byte -- not a well-formed TS packet, leave it alone
        return;

    const bool payloadStart = (ts[1] & 0x40) != 0;
    const int adaptationFieldControl = (ts[3] >> 4) & 0x3;
    const bool hasAdaptation = adaptationFieldControl == 0x2 || adaptationFieldControl == 0x3;
    const bool hasPayload = adaptationFieldControl == 0x1 || adaptationFieldControl == 0x3;

    int afLen = 0;
    if (hasAdaptation) {
        afLen = ts[4];
        if (afLen > 0 && 5 + afLen <= 188) {
            const quint8 afFlags = ts[5];
            const bool pcrFlag = (afFlags & 0x10) != 0;
            if (pcrFlag && afLen >= 7) {
                quint8 *pcr = ts + 6;
                const quint64 raw48 = (quint64(pcr[0]) << 40) | (quint64(pcr[1]) << 32)
                        | (quint64(pcr[2]) << 24) | (quint64(pcr[3]) << 16)
                        | (quint64(pcr[4]) << 8) | quint64(pcr[5]);
                const quint64 base = (raw48 >> 15) & 0x1FFFFFFFFULL;
                const quint64 reserved = (raw48 >> 9) & 0x3F;
                const quint64 ext = raw48 & 0x1FF;
                const quint64 newBase = quint64((qint64(base) + offsetUnits) & 0x1FFFFFFFFLL);
                const quint64 raw48b = (newBase << 15) | (reserved << 9) | ext;
                pcr[0] = quint8(raw48b >> 40);
                pcr[1] = quint8(raw48b >> 32);
                pcr[2] = quint8(raw48b >> 24);
                pcr[3] = quint8(raw48b >> 16);
                pcr[4] = quint8(raw48b >> 8);
                pcr[5] = quint8(raw48b);
            }
        }
    }

    if (!payloadStart || !hasPayload)
        return;
    const int payloadOffset = hasAdaptation ? (5 + afLen) : 4;
    if (payloadOffset + 9 > 188)
        return;
    quint8 *p = ts + payloadOffset;
    if (!(p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x01)) // PES start code prefix
        return;
    const quint8 streamId = p[3];
    // Only streams that actually use the "optional PES header" layout below
    // carry PTS/DTS this way -- video (0xE0-0xEF), audio (0xC0-0xDF),
    // private_stream_1 (0xBD, classic AC3/DTS/PGS-subtitle carrier) and
    // 0xFD (the MPEG-2 "extended stream ID" marker BD reuses for its own
    // HD audio codecs -- DTS-HD Master Audio/TrueHD -- confirmed 2026-08-18
    // via real diagnostic capture against an actual disc: 0xFD was the ONLY
    // stream_id ever seen on the audio PID, never 0xBD/0xC0-0xDF. Missing
    // this entirely skipped rewriting audio's PTS/DTS, leaving it at its
    // raw, un-shifted clip-relative value while video's got correctly
    // shifted -- symptoms: audio silent after a seek into a later clip
    // (its stale PTS was nowhere near where the demuxer was now looking),
    // and a large constant audio/video delay even on a single-clip title
    // (audio's PTS sat ~11.65s ahead of the now-corrected video timeline).
    // Anything else (padding, private_stream_2/PCI-like data, PSI tables,
    // ...) must be left alone: reading byte offset 7 in those payloads
    // would just be interpreting arbitrary data bytes as PTS_DTS_flags.
    const bool pesTimedStream = streamId == 0xBD || streamId == 0xFD
            || (streamId >= 0xC0 && streamId <= 0xEF);
    if (!pesTimedStream)
        return;
    const quint8 ptsDtsFlags = (p[7] >> 6) & 0x3;
    if (ptsDtsFlags == 0)
        return; // no PTS/DTS present
    const int tsOffset = payloadOffset + 9; // where the optional fields begin

    auto rewriteTimestamp = [&](int off) -> bool {
        if (off + 5 > 188) // off is already absolute within the 188-byte TS packet
            return false;
        quint8 *b = ts + off;
        // Sanity-check the marker bits before touching anything (defensive
        // against a false-positive PES/PTS_DTS_flags match).
        if ((b[0] & 0x01) != 0x01 || (b[2] & 0x01) != 0x01 || (b[4] & 0x01) != 0x01)
            return false;
        const quint8 prefix = (b[0] >> 4) & 0x0F;
        if (prefix != 0x1 && prefix != 0x2 && prefix != 0x3)
            return false;
        const qint64 pts = (qint64(b[0] & 0x0E) << 29) | (qint64(b[1]) << 22)
                | (qint64(b[2] & 0xFE) << 14) | (qint64(b[3]) << 7) | (qint64(b[4]) >> 1);
        const quint64 newPts = quint64((pts + offsetUnits) & 0x1FFFFFFFFLL);
        b[0] = quint8((prefix << 4) | (((newPts >> 30) & 0x07) << 1) | 0x01);
        b[1] = quint8((newPts >> 22) & 0xFF);
        b[2] = quint8((((newPts >> 15) & 0x7F) << 1) | 0x01);
        b[3] = quint8((newPts >> 7) & 0xFF);
        b[4] = quint8(((newPts & 0x7F) << 1) | 0x01);
        return true;
    };

    if (ptsDtsFlags == 0x2) { // PTS only
        rewriteTimestamp(tsOffset);
    } else if (ptsDtsFlags == 0x3) { // PTS + DTS
        if (rewriteTimestamp(tsOffset))
            rewriteTimestamp(tsOffset + 5);
    }
}

} // namespace

qint64 BlurayTitleDevice::readData(char *data, qint64 maxSize)
{
#ifdef VIVACE_HAVE_BLURAY
    if (!m_bd)
        return -1;

    // Top up m_leftover with freshly rewritten, packet-aligned data until
    // it can satisfy the caller's request or the title's own stream ends.
    constexpr qint64 kChunkBytes = 192 * 512; // 96 KiB, a multiple of 192
    QByteArray rawBuf(kChunkBytes, Qt::Uninitialized); // heap, not stack -- see the prefetch buffer's own precedent
    while (m_leftover.size() < maxSize) {
        if (m_readCursor >= m_size)
            break; // at the title's own end
        quint8 *raw = reinterpret_cast<quint8 *>(rawBuf.data());
        const qint64 want = qMin<qint64>(kChunkBytes, m_size - m_readCursor);
        const int n = bd_read(reinterpret_cast<BLURAY *>(m_bd), raw,
                               static_cast<int>(qMin<qint64>(want, INT_MAX)));
        if (n <= 0)
            break; // EOF or a real read error -- either way, stop here
        const int wholePackets = n / 192;
        for (int i = 0; i < wholePackets; ++i) {
            quint8 *pkt = raw + i * 192;
            const int clipIdx = clipIndexForByte(m_readCursor);
            m_currentClip = clipIdx;
            if (clipIdx >= 0 && clipIdx < m_clips.size())
                rewriteTsPacketTimestamps(pkt, m_clips.at(clipIdx).offsetUnits);
            m_leftover.append(reinterpret_cast<const char *>(pkt), 192);
            m_readCursor += 192;
        }
        // A short, non-192-aligned trailing read (shouldn't normally
        // happen for a well-formed BDAV-TS stream) is passed through
        // unrewritten rather than dropped, so no bytes are silently lost.
        const int leftoverBytes = n - wholePackets * 192;
        if (leftoverBytes > 0) {
            m_leftover.append(reinterpret_cast<const char *>(raw + wholePackets * 192), leftoverBytes);
            m_readCursor += leftoverBytes;
        }
    }

    const qint64 give = qMin<qint64>(maxSize, m_leftover.size());
    if (give <= 0)
        return m_readCursor >= m_size ? 0 : -1;
    memcpy(data, m_leftover.constData(), give);
    m_leftover.remove(0, give);
    return give;
#else
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return -1;
#endif
}
