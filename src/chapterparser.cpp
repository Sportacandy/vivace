/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "chapterparser.h"

#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QStringConverter>
#include <algorithm>

namespace {

// ---- MP4 / MOV (ISO base media): moov > udta > chpl (Nero chapters) --------

constexpr quint32 fourcc(char a, char b, char c, char d)
{
    return (quint32(quint8(a)) << 24) | (quint32(quint8(b)) << 16)
           | (quint32(quint8(c)) << 8) | quint32(quint8(d));
}

quint32 beU32(const uchar *p) { return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]; }
quint64 beU64(const uchar *p)
{
    return (quint64(beU32(p)) << 32) | beU32(p + 4);
}

struct Box {
    quint32 type = 0;
    qint64 contentStart = 0;
    qint64 contentEnd = 0;
};

// Reads the box header at *pos within [pos, end) and advances pos past the box.
bool nextBox(QFile &f, qint64 &pos, qint64 end, Box &box)
{
    if (pos + 8 > end || !f.seek(pos))
        return false;
    uchar hdr[8];
    if (f.read(reinterpret_cast<char *>(hdr), 8) != 8)
        return false;
    quint64 size = beU32(hdr);
    qint64 headerSize = 8;
    if (size == 1) {
        uchar big[8];
        if (f.read(reinterpret_cast<char *>(big), 8) != 8)
            return false;
        size = beU64(big);
        headerSize = 16;
    } else if (size == 0) {
        size = quint64(end - pos); // extends to the end of the container
    }
    if (size < quint64(headerSize) || pos + qint64(size) > end)
        return false;
    box.type = beU32(hdr + 4);
    box.contentStart = pos + headerSize;
    box.contentEnd = pos + qint64(size);
    pos = box.contentEnd;
    return true;
}

bool findBox(QFile &f, qint64 start, qint64 end, quint32 type, Box &out)
{
    qint64 pos = start;
    Box box;
    while (nextBox(f, pos, end, box)) {
        if (box.type == type) {
            out = box;
            return true;
        }
    }
    return false;
}

QList<Box> childBoxesOfType(QFile &f, qint64 start, qint64 end, quint32 type)
{
    QList<Box> out;
    qint64 pos = start;
    Box box;
    while (nextBox(f, pos, end, box))
        if (box.type == type)
            out.append(box);
    return out;
}

QByteArray boxData(QFile &f, const Box &box)
{
    if (!f.seek(box.contentStart))
        return {};
    return f.read(box.contentEnd - box.contentStart);
}

// ---- MP4 Nero 'chpl' -------------------------------------------------------

QList<ChapterParse::Chapter> parseMp4Chpl(QFile &f)
{
    QList<ChapterParse::Chapter> result;
    const qint64 fileEnd = f.size();

    Box moov;
    if (!findBox(f, 0, fileEnd, fourcc('m', 'o', 'o', 'v'), moov))
        return result;
    Box udta;
    if (!findBox(f, moov.contentStart, moov.contentEnd, fourcc('u', 'd', 't', 'a'), udta))
        return result;
    Box chpl;
    if (!findBox(f, udta.contentStart, udta.contentEnd, fourcc('c', 'h', 'p', 'l'), chpl))
        return result;

    if (!f.seek(chpl.contentStart))
        return result;
    const QByteArray data = f.read(chpl.contentEnd - chpl.contentStart);
    const auto *p = reinterpret_cast<const uchar *>(data.constData());
    const int n = int(data.size());
    if (n < 5)
        return result;

    const quint8 version = p[0];
    int i = 4;                    // skip version(1) + flags(3)
    if (version >= 1)
        i += 4;                   // reserved
    if (i >= n)
        return result;
    const int count = p[i++];     // u8 chapter count
    for (int c = 0; c < count && i + 9 <= n; ++c) {
        const quint64 start100ns = beU64(p + i);
        i += 8;
        const int len = p[i++];
        if (i + len > n)
            break;
        ChapterParse::Chapter ch;
        ch.startMs = qint64(start100ns / 10000); // 100 ns units -> ms
        ch.title = QString::fromUtf8(data.constData() + i, len);
        i += len;
        result.append(ch);
    }
    return result;
}

// ---- MP4/MOV QuickTime chapter track (tref 'chap' -> a text track) ---------

QString decodeChapterText(const QByteArray &bytes)
{
    if (bytes.size() >= 2) {
        const uchar b0 = uchar(bytes[0]);
        const uchar b1 = uchar(bytes[1]);
        if (b0 == 0xFE && b1 == 0xFF) {
            QStringDecoder dec(QStringDecoder::Utf16BE);
            return dec.decode(QByteArrayView(bytes).sliced(2));
        }
        if (b0 == 0xFF && b1 == 0xFE) {
            QStringDecoder dec(QStringDecoder::Utf16LE);
            return dec.decode(QByteArrayView(bytes).sliced(2));
        }
    }
    return QString::fromUtf8(bytes); // MacRoman is rare in the wild; UTF-8 default
}

QList<ChapterParse::Chapter> parseMp4ChapterTrack(QFile &f)
{
    QList<ChapterParse::Chapter> result;
    const qint64 fileEnd = f.size();

    Box moov;
    if (!findBox(f, 0, fileEnd, fourcc('m', 'o', 'o', 'v'), moov))
        return result;

    const QList<Box> traks =
        childBoxesOfType(f, moov.contentStart, moov.contentEnd, fourcc('t', 'r', 'a', 'k'));

    // Track IDs referenced as chapter tracks (any trak's tref > chap).
    QSet<quint32> chapterIds;
    for (const Box &trak : traks) {
        Box tref, chap;
        if (findBox(f, trak.contentStart, trak.contentEnd, fourcc('t', 'r', 'e', 'f'), tref)
            && findBox(f, tref.contentStart, tref.contentEnd, fourcc('c', 'h', 'a', 'p'), chap)) {
            const QByteArray d = boxData(f, chap);
            const auto *p = reinterpret_cast<const uchar *>(d.constData());
            for (int i = 0; i + 4 <= d.size(); i += 4)
                chapterIds.insert(beU32(p + i));
        }
    }
    if (chapterIds.isEmpty())
        return result;

    // The trak whose tkhd track_id is referenced is the chapter (text) track.
    Box chapterTrak;
    bool found = false;
    for (const Box &trak : traks) {
        Box tkhd;
        if (!findBox(f, trak.contentStart, trak.contentEnd, fourcc('t', 'k', 'h', 'd'), tkhd))
            continue;
        const QByteArray d = boxData(f, tkhd);
        if (d.size() < 8)
            continue;
        const int idOff = 4 + (uchar(d[0]) == 1 ? 16 : 8);
        if (idOff + 4 > d.size())
            continue;
        if (chapterIds.contains(beU32(reinterpret_cast<const uchar *>(d.constData()) + idOff))) {
            chapterTrak = trak;
            found = true;
            break;
        }
    }
    if (!found)
        return result;

    Box mdia, mdhd, minf, stbl;
    if (!findBox(f, chapterTrak.contentStart, chapterTrak.contentEnd, fourcc('m', 'd', 'i', 'a'), mdia)
        || !findBox(f, mdia.contentStart, mdia.contentEnd, fourcc('m', 'd', 'h', 'd'), mdhd)
        || !findBox(f, mdia.contentStart, mdia.contentEnd, fourcc('m', 'i', 'n', 'f'), minf)
        || !findBox(f, minf.contentStart, minf.contentEnd, fourcc('s', 't', 'b', 'l'), stbl))
        return result;

    // Media timescale.
    quint32 timescale = 0;
    {
        const QByteArray d = boxData(f, mdhd);
        if (d.size() < 8)
            return result;
        const int tsOff = 4 + (uchar(d[0]) == 1 ? 16 : 8);
        if (tsOff + 4 > d.size())
            return result;
        timescale = beU32(reinterpret_cast<const uchar *>(d.constData()) + tsOff);
    }
    if (timescale == 0)
        return result;

    // Sample sizes (stsz).
    QList<qint64> sizes;
    {
        Box stsz;
        if (!findBox(f, stbl.contentStart, stbl.contentEnd, fourcc('s', 't', 's', 'z'), stsz))
            return result;
        const QByteArray d = boxData(f, stsz);
        if (d.size() < 12)
            return result;
        const auto *p = reinterpret_cast<const uchar *>(d.constData());
        const quint32 uniform = beU32(p + 4);
        const quint32 count = beU32(p + 8);
        for (quint32 i = 0; i < count; ++i) {
            if (uniform)
                sizes.append(uniform);
            else if (12 + 4 * qint64(i) + 4 <= d.size())
                sizes.append(beU32(p + 12 + 4 * i));
        }
    }
    const int sampleCount = int(sizes.size());
    if (sampleCount == 0)
        return result;

    // Chunk offsets (stco / co64).
    QList<qint64> chunkOffsets;
    {
        Box stco;
        if (findBox(f, stbl.contentStart, stbl.contentEnd, fourcc('s', 't', 'c', 'o'), stco)) {
            const QByteArray d = boxData(f, stco);
            const auto *p = reinterpret_cast<const uchar *>(d.constData());
            if (d.size() >= 8) {
                const quint32 nc = beU32(p + 4);
                for (quint32 i = 0; i < nc && 8 + 4 * qint64(i) + 4 <= d.size(); ++i)
                    chunkOffsets.append(beU32(p + 8 + 4 * i));
            }
        } else if (findBox(f, stbl.contentStart, stbl.contentEnd, fourcc('c', 'o', '6', '4'), stco)) {
            const QByteArray d = boxData(f, stco);
            const auto *p = reinterpret_cast<const uchar *>(d.constData());
            if (d.size() >= 8) {
                const quint32 nc = beU32(p + 4);
                for (quint32 i = 0; i < nc && 8 + 8 * qint64(i) + 8 <= d.size(); ++i)
                    chunkOffsets.append(qint64(beU64(p + 8 + 8 * i)));
            }
        }
    }
    if (chunkOffsets.isEmpty())
        return result;

    // Sample-to-chunk (stsc): first_chunk, samples_per_chunk pairs.
    struct StscEntry { quint32 firstChunk; quint32 samplesPerChunk; };
    QList<StscEntry> stsc;
    {
        Box box;
        if (findBox(f, stbl.contentStart, stbl.contentEnd, fourcc('s', 't', 's', 'c'), box)) {
            const QByteArray d = boxData(f, box);
            const auto *p = reinterpret_cast<const uchar *>(d.constData());
            if (d.size() >= 8) {
                const quint32 ne = beU32(p + 4);
                for (quint32 i = 0; i < ne && 8 + 12 * qint64(i) + 12 <= d.size(); ++i)
                    stsc.append({ beU32(p + 8 + 12 * i), beU32(p + 8 + 12 * i + 4) });
            }
        }
    }
    if (stsc.isEmpty())
        stsc.append({ 1, 1 });

    // Resolve each sample's file offset from the chunk/size tables.
    QList<qint64> sampleOffsets;
    int sIdx = 0;
    for (int c = 0; c < chunkOffsets.size() && sIdx < sampleCount; ++c) {
        const quint32 chunkNo = quint32(c) + 1;
        quint32 spc = stsc.last().samplesPerChunk;
        for (int e = 0; e < stsc.size(); ++e) {
            const quint32 next = (e + 1 < stsc.size()) ? stsc[e + 1].firstChunk : 0xFFFFFFFFu;
            if (chunkNo >= stsc[e].firstChunk && chunkNo < next) {
                spc = stsc[e].samplesPerChunk;
                break;
            }
        }
        qint64 off = chunkOffsets[c];
        for (quint32 s = 0; s < spc && sIdx < sampleCount; ++s) {
            sampleOffsets.append(off);
            off += sizes[sIdx];
            ++sIdx;
        }
    }

    // Sample start times (stts): run-length list of durations.
    QList<qint64> startUnits;
    {
        Box stts;
        if (!findBox(f, stbl.contentStart, stbl.contentEnd, fourcc('s', 't', 't', 's'), stts))
            return result;
        const QByteArray d = boxData(f, stts);
        if (d.size() < 8)
            return result;
        const auto *p = reinterpret_cast<const uchar *>(d.constData());
        const quint32 ne = beU32(p + 4);
        qint64 cumulative = 0;
        for (quint32 i = 0; i < ne && 8 + 8 * qint64(i) + 8 <= d.size(); ++i) {
            const quint32 cnt = beU32(p + 8 + 8 * i);
            const quint32 delta = beU32(p + 8 + 8 * i + 4);
            for (quint32 k = 0; k < cnt; ++k) {
                startUnits.append(cumulative);
                cumulative += delta;
            }
        }
    }

    const int chapters =
        std::min({ int(sampleOffsets.size()), int(startUnits.size()), sampleCount });
    for (int i = 0; i < chapters; ++i) {
        if (!f.seek(sampleOffsets[i]))
            continue;
        const QByteArray sd = f.read(sizes[i]);
        if (sd.size() < 2)
            continue;
        const auto *p = reinterpret_cast<const uchar *>(sd.constData());
        int textLen = (p[0] << 8) | p[1];
        if (2 + textLen > sd.size())
            textLen = int(sd.size()) - 2;
        ChapterParse::Chapter ch;
        ch.startMs = startUnits[i] * 1000 / timescale;
        ch.title = decodeChapterText(sd.mid(2, textLen));
        result.append(ch);
    }
    return result;
}

QList<ChapterParse::Chapter> parseMp4(QFile &f)
{
    // Prefer Nero 'chpl' (simple); fall back to a QuickTime chapter track.
    QList<ChapterParse::Chapter> result = parseMp4Chpl(f);
    if (result.isEmpty())
        result = parseMp4ChapterTrack(f);
    return result;
}

// ---- Matroska / WebM (EBML): Segment > Chapters > EditionEntry > ChapterAtom

enum EbmlId : quint32 {
    IdSegment = 0x18538067,
    IdChapters = 0x1043A770,
    IdEditionEntry = 0x45B9,
    IdChapterAtom = 0xB6,
    IdChapterTimeStart = 0x91,
    IdChapterDisplay = 0x80,
    IdChapterString = 0x85,
};

// Byte length of a VINT from its first byte (leading-zero count + 1).
int vintLength(quint8 first)
{
    for (int i = 0; i < 8; ++i)
        if (first & (0x80 >> i))
            return i + 1;
    return 0;
}

// Reads an EBML element ID (marker bits kept, as Matroska IDs include them).
quint32 readId(QFile &f, int &len)
{
    uchar first;
    if (f.read(reinterpret_cast<char *>(&first), 1) != 1) {
        len = 0;
        return 0;
    }
    len = vintLength(first);
    if (len == 0 || len > 4) {
        len = 0;
        return 0;
    }
    quint32 id = first;
    for (int i = 1; i < len; ++i) {
        uchar b;
        if (f.read(reinterpret_cast<char *>(&b), 1) != 1) {
            len = 0;
            return 0;
        }
        id = (id << 8) | b;
    }
    return id;
}

// Reads an EBML size (marker bits cleared). Returns -1 on error; for the
// (rare, streaming-only) unknown size it returns a value large enough to run
// to the parent's end.
qint64 readSize(QFile &f, qint64 parentEnd)
{
    uchar first;
    if (f.read(reinterpret_cast<char *>(&first), 1) != 1)
        return -1;
    const int len = vintLength(first);
    if (len == 0)
        return -1;
    quint64 value = first & (0xFF >> len);
    quint64 unknown = (quint64(1) << (7 * len)) - 1; // all data bits set
    for (int i = 1; i < len; ++i) {
        uchar b;
        if (f.read(reinterpret_cast<char *>(&b), 1) != 1)
            return -1;
        value = (value << 8) | b;
    }
    if (value == unknown)
        return parentEnd - f.pos();
    return qint64(value);
}

quint64 readUInt(QFile &f, qint64 size)
{
    quint64 value = 0;
    for (qint64 i = 0; i < size && i < 8; ++i) {
        uchar b;
        if (f.read(reinterpret_cast<char *>(&b), 1) != 1)
            break;
        value = (value << 8) | b;
    }
    return value;
}

// Iterates the direct children of [start, end), calling fn(id, dataStart, size)
// for each. fn returns the position to continue from (normally dataStart+size).
template <typename Fn>
void forEachElement(QFile &f, qint64 start, qint64 end, Fn fn)
{
    qint64 pos = start;
    while (pos < end) {
        if (!f.seek(pos))
            return;
        int idLen = 0;
        const quint32 id = readId(f, idLen);
        if (idLen == 0)
            return;
        const qint64 size = readSize(f, end);
        if (size < 0)
            return;
        const qint64 dataStart = f.pos();
        if (dataStart + size > end)
            return;
        fn(id, dataStart, size);
        pos = dataStart + size;
    }
}

void parseChapterAtom(QFile &f, qint64 start, qint64 end,
                      QList<ChapterParse::Chapter> &out)
{
    ChapterParse::Chapter chapter;
    bool haveStart = false;
    forEachElement(f, start, end, [&](quint32 id, qint64 dataStart, qint64 size) {
        if (id == IdChapterTimeStart) {
            f.seek(dataStart);
            chapter.startMs = qint64(readUInt(f, size) / 1000000); // ns -> ms
            haveStart = true;
        } else if (id == IdChapterDisplay) {
            forEachElement(f, dataStart, dataStart + size,
                           [&](quint32 sid, qint64 ss, qint64 sl) {
                               if (sid == IdChapterString && chapter.title.isEmpty()) {
                                   f.seek(ss);
                                   chapter.title = QString::fromUtf8(f.read(sl));
                               }
                           });
        }
        // Nested ChapterAtoms are intentionally not flattened into separate
        // chapters.
    });
    if (haveStart)
        out.append(chapter);
}

QList<ChapterParse::Chapter> parseMatroska(QFile &f)
{
    QList<ChapterParse::Chapter> result;
    const qint64 fileEnd = f.size();

    // Top level: find the Segment.
    qint64 segmentStart = -1, segmentEnd = -1;
    forEachElement(f, 0, fileEnd, [&](quint32 id, qint64 dataStart, qint64 size) {
        if (id == IdSegment && segmentStart < 0) {
            segmentStart = dataStart;
            segmentEnd = dataStart + size;
        }
    });
    if (segmentStart < 0)
        return result;

    // Segment level: find Chapters.
    qint64 chaptersStart = -1, chaptersEnd = -1;
    forEachElement(f, segmentStart, segmentEnd,
                   [&](quint32 id, qint64 dataStart, qint64 size) {
                       if (id == IdChapters && chaptersStart < 0) {
                           chaptersStart = dataStart;
                           chaptersEnd = dataStart + size;
                       }
                   });
    if (chaptersStart < 0)
        return result;

    // Use the first EditionEntry that yields chapters.
    forEachElement(f, chaptersStart, chaptersEnd,
                   [&](quint32 id, qint64 dataStart, qint64 size) {
                       if (id != IdEditionEntry || !result.isEmpty())
                           return;
                       forEachElement(f, dataStart, dataStart + size,
                                      [&](quint32 aid, qint64 as, qint64 al) {
                                          if (aid == IdChapterAtom)
                                              parseChapterAtom(f, as, as + al, result);
                                      });
                   });
    return result;
}

} // namespace

namespace ChapterParse {

QList<Chapter> chapters(const QString &filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly))
        return {};

    // Sniff the container: EBML magic or an ISO-BMFF 'ftyp' box, falling back
    // to the extension.
    uchar head[12] = {0};
    const qint64 got = f.read(reinterpret_cast<char *>(head), 12);

    bool isMatroska = got >= 4 && head[0] == 0x1A && head[1] == 0x45
            && head[2] == 0xDF && head[3] == 0xA3;
    bool isMp4 = got >= 8 && head[4] == 'f' && head[5] == 't'
            && head[6] == 'y' && head[7] == 'p';

    if (!isMatroska && !isMp4) {
        const QString ext = QFileInfo(filePath).suffix().toLower();
        if (ext == "mkv" || ext == "webm" || ext == "mka" || ext == "mk3d")
            isMatroska = true;
        else if (ext == "mp4" || ext == "m4v" || ext == "mov" || ext == "m4a"
                 || ext == "m4b" || ext == "3gp")
            isMp4 = true;
    }

    QList<Chapter> result;
    if (isMatroska)
        result = parseMatroska(f);
    else if (isMp4)
        result = parseMp4(f);

    std::stable_sort(result.begin(), result.end(),
                     [](const Chapter &a, const Chapter &b) {
                         return a.startMs < b.startMs;
                     });
    return result;
}

} // namespace ChapterParse
