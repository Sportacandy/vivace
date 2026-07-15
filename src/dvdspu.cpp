/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "dvdspu.h"

#include <QFile>

namespace DvdMenu {

namespace {

constexpr qint64 sectorSize = 2048;

quint16 be16(const QByteArray &d, qint64 o)
{
    return (o < 0 || o + 2 > d.size())
                   ? 0 : quint16((quint8(d.at(o)) << 8) | quint8(d.at(o + 1)));
}

// Assemble the subpicture unit: concatenate the private_stream_1 substream
// 0x20 payloads across the menu cell's NAV-packed sectors until the SPU's
// declared size (its first 2 bytes) is reached.
QByteArray assembleSpu(const QString &vobPath, qint64 firstSector,
                       qint64 lastSector)
{
    QFile file(vobPath);
    if (!file.open(QIODevice::ReadOnly) || !file.seek(firstSector * sectorSize))
        return {};

    QByteArray spu;
    int declared = -1;
    for (qint64 s = firstSector; s <= lastSector; ++s) {
        const QByteArray sector = file.read(sectorSize);
        if (sector.size() < sectorSize)
            break;
        int i = 0;
        while (true) {
            const int j = sector.indexOf(QByteArrayLiteral("\x00\x00\x01\xBD"), i);
            if (j < 0)
                break;
            const int pesLen = be16(sector, j + 4);
            const int hdrLen = quint8(sector.at(j + 8));
            const int payloadStart = j + 9 + hdrLen;
            const int payloadEnd = j + 6 + pesLen;
            if (payloadStart < payloadEnd && payloadEnd <= sector.size()
                && quint8(sector.at(payloadStart)) == 0x20) { // subpicture
                spu.append(sector.constData() + payloadStart + 1,
                           payloadEnd - payloadStart - 1);
                if (declared < 0 && spu.size() >= 2)
                    declared = be16(spu, 0);
            }
            i = j + 4;
        }
        if (declared > 0 && spu.size() >= declared)
            break;
    }
    if (declared > 0 && spu.size() >= declared)
        return spu.left(declared);
    return {};
}

// Nibble reader over the RLE pixel data.
struct NibbleReader {
    const uchar *d;
    qint64 size;
    qint64 pos;
    bool high = true;
    int next()
    {
        if (pos >= size)
            return 0;
        const uchar b = d[pos];
        if (high) {
            high = false;
            return b >> 4;
        }
        high = true;
        ++pos;
        return b & 0x0F;
    }
    void align()
    {
        if (!high) {
            high = true;
            ++pos;
        }
    }
};

// Decode one RLE field into rows of colour indices (0-3).
void decodeField(const uchar *data, qint64 size, qint64 offset, int width,
                 int rows, QByteArray &out, int rowStride, int startRow)
{
    NibbleReader nr { data, size, offset };
    for (int y = 0; y < rows; ++y) {
        int x = 0;
        char *line = out.data() + qsizetype(startRow + y * 2) * rowStride;
        while (x < width) {
            int code = nr.next();
            if (code < 0x4) {
                code = (code << 4) | nr.next();
                if (code < 0x10) {
                    code = (code << 4) | nr.next();
                    if (code < 0x40)
                        code = (code << 4) | nr.next();
                }
            }
            int run = code >> 2;
            const int color = code & 0x3;
            if (run == 0 || run > width - x)
                run = width - x;
            for (int k = 0; k < run && x < width; ++k, ++x)
                line[x] = char(color);
        }
        nr.align();
    }
}

int rgbFromYCrCb(quint32 v)
{
    const int Y = (v >> 16) & 0xFF;
    const int Cr = (v >> 8) & 0xFF;
    const int Cb = v & 0xFF;
    auto clamp = [](double x) { return x < 0 ? 0 : (x > 255 ? 255 : int(x)); };
    const int r = clamp(Y + 1.402 * (Cr - 128));
    const int g = clamp(Y - 0.344136 * (Cb - 128) - 0.714136 * (Cr - 128));
    const int b = clamp(Y + 1.772 * (Cb - 128));
    return (r << 16) | (g << 8) | b;
}

} // namespace

Subpicture decodeSubpicture(const QString &vobPath, qint64 firstSector,
                            qint64 lastSector)
{
    const QByteArray spu = assembleSpu(vobPath, firstSector, lastSector);
    Subpicture out;
    if (spu.size() < 4)
        return out;

    const qint64 ctrl = be16(spu, 2);
    // First SP_DCSQ: delay(2) next(2) then commands until 0xFF.
    qint64 p = ctrl + 4;
    int topOff = -1, botOff = -1;
    int daX0 = 0, daX1 = 719, daY0 = 0, daY1 = 479;
    const uchar *d = reinterpret_cast<const uchar *>(spu.constData());
    while (p < spu.size()) {
        const int cmd = quint8(spu.at(p));
        if (cmd == 0xFF)
            break;
        switch (cmd) {
        case 0x00: // FSTA_DSP
        case 0x01: // STA_DSP
        case 0x02: // STP_DSP
            ++p;
            break;
        case 0x03: { // SET_COLOR (index per SPU colour 0..3)
            const int v = be16(spu, p + 1);
            out.baseColor[3] = (v >> 12) & 0xF;
            out.baseColor[2] = (v >> 8) & 0xF;
            out.baseColor[1] = (v >> 4) & 0xF;
            out.baseColor[0] = v & 0xF;
            p += 3;
            break;
        }
        case 0x04: { // SET_CONTR (alpha per SPU colour 0..3)
            const int v = be16(spu, p + 1);
            out.baseAlpha[3] = (v >> 12) & 0xF;
            out.baseAlpha[2] = (v >> 8) & 0xF;
            out.baseAlpha[1] = (v >> 4) & 0xF;
            out.baseAlpha[0] = v & 0xF;
            p += 3;
            break;
        }
        case 0x05: { // SET_DAREA
            daX0 = (quint8(spu.at(p + 1)) << 4) | (quint8(spu.at(p + 2)) >> 4);
            daX1 = ((quint8(spu.at(p + 2)) & 0xF) << 8) | quint8(spu.at(p + 3));
            daY0 = (quint8(spu.at(p + 4)) << 4) | (quint8(spu.at(p + 5)) >> 4);
            daY1 = ((quint8(spu.at(p + 5)) & 0xF) << 8) | quint8(spu.at(p + 6));
            p += 7;
            break;
        }
        case 0x06: // SET_DSPXA
            topOff = be16(spu, p + 1);
            botOff = be16(spu, p + 3);
            p += 5;
            break;
        default:
            p = spu.size(); // unknown command — stop
            break;
        }
    }

    if (topOff < 0 || botOff < 0)
        return out;
    const int width = daX1 - daX0 + 1;
    const int height = daY1 - daY0 + 1;
    if (width <= 0 || width > 4096 || height <= 0 || height > 2048)
        return out;

    // The two fields interlace into the full picture; render at full frame size
    // (720xH-ish) with the display area offset so overlay coordinates match.
    out.width = daX1 + 1;
    out.height = daY1 + 1;
    out.pixels = QByteArray(qsizetype(out.width) * out.height, char(0));
    const int rows = height / 2;
    // Decode into the display-area sub-rectangle (top field even, bottom odd).
    QByteArray area(qsizetype(width) * height, char(0));
    decodeField(d, spu.size(), topOff, width, rows, area, width, 0);
    decodeField(d, spu.size(), botOff, width, rows, area, width, 1);
    for (int y = 0; y < height; ++y) {
        const char *src = area.constData() + qsizetype(y) * width;
        char *dst = out.pixels.data() + qsizetype(daY0 + y) * out.width + daX0;
        memcpy(dst, src, width);
    }
    return out;
}

QImage renderHighlight(const Subpicture &sp, const quint32 palette[16],
                       const QRect &selectRect, quint32 selectColorContrast)
{
    if (!sp.isValid())
        return {};

    // Select-state mapping from a BTN_COLIT entry.
    int selColor[4], selAlpha[4];
    selColor[3] = (selectColorContrast >> 28) & 0xF;
    selColor[2] = (selectColorContrast >> 24) & 0xF;
    selColor[1] = (selectColorContrast >> 20) & 0xF;
    selColor[0] = (selectColorContrast >> 16) & 0xF;
    selAlpha[3] = (selectColorContrast >> 12) & 0xF;
    selAlpha[2] = (selectColorContrast >> 8) & 0xF;
    selAlpha[1] = (selectColorContrast >> 4) & 0xF;
    selAlpha[0] = selectColorContrast & 0xF;

    // Precompute ARGB for base and select states.
    QRgb baseArgb[4], selArgb[4];
    for (int i = 0; i < 4; ++i) {
        const int a = sp.baseAlpha[i] * 17;
        baseArgb[i] = qRgba(0, 0, 0, 0);
        if (a > 0) {
            const int rgb = rgbFromYCrCb(palette[sp.baseColor[i] & 0xF]);
            baseArgb[i] = qRgba((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF, a);
        }
        const int sa = selAlpha[i] * 17;
        selArgb[i] = qRgba(0, 0, 0, 0);
        if (sa > 0) {
            const int rgb = rgbFromYCrCb(palette[selColor[i] & 0xF]);
            selArgb[i] = qRgba((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF, sa);
        }
    }

    QImage img(sp.width, sp.height, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    bool anyVisible = false;
    for (int y = 0; y < sp.height; ++y) {
        const char *src = sp.pixels.constData() + qsizetype(y) * sp.width;
        QRgb *dst = reinterpret_cast<QRgb *>(img.scanLine(y));
        const bool rowInSel = selectRect.isValid()
                && y >= selectRect.top() && y <= selectRect.bottom();
        for (int x = 0; x < sp.width; ++x) {
            const int idx = src[x] & 0x3;
            const bool inSel = rowInSel && x >= selectRect.left()
                    && x <= selectRect.right();
            const QRgb c = inSel ? selArgb[idx] : baseArgb[idx];
            dst[x] = c;
            if (qAlpha(c) > 0)
                anyVisible = true;
        }
    }
    return anyVisible ? img : QImage();
}

} // namespace DvdMenu
