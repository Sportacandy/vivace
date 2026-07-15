/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "dvdpciparser.h"

#include <QFile>

namespace DvdMenu {

namespace {

constexpr qint64 sectorSize = 2048;

/*  Within the PCI packet (pci_t, starting right after the 1-byte substream
    id), byte offsets, per nav_types.h packing:
      pci_gi  60 bytes, nsml_agli 36 -> hli at 96.
      hli: hl_gi 22 bytes, btn_colit 24 -> btnit at 142.
      hl_gi.btn_ns is at hl_gi + 17.
    On disk each 18-byte button is laid out (natural order, not the rotated
    C struct): 3 bytes x-position, 3 bytes y-position, 4 bytes neighbours,
    8 bytes command. */
constexpr int kHlgiOffset = 96;
constexpr int kBtnNsOffset = kHlgiOffset + 17;
constexpr int kBtnColitOffset = kHlgiOffset + 22; // 118: three 8-byte sets
constexpr int kBtnItOffset = 142;

quint32 be32(const QByteArray &d, int o)
{
    if (o < 0 || o + 4 > d.size())
        return 0;
    return (quint32(quint8(d.at(o))) << 24) | (quint32(quint8(d.at(o + 1))) << 16)
            | (quint32(quint8(d.at(o + 2))) << 8) | quint32(quint8(d.at(o + 3)));
}

Button parseButton(const uchar *d)
{
    Button b;
    b.colorNumber = (d[0] >> 6) & 0x3;
    const int xStart = ((d[0] & 0x3F) << 4) | (d[1] >> 4);
    const int xEnd = ((d[1] & 0x03) << 8) | d[2];
    b.autoAction = (d[3] >> 6) != 0;
    const int yStart = ((d[3] & 0x3F) << 4) | (d[4] >> 4);
    const int yEnd = ((d[4] & 0x03) << 8) | d[5];
    b.up = d[6] & 0x3F;
    b.down = d[7] & 0x3F;
    b.left = d[8] & 0x3F;
    b.right = d[9] & 0x3F;
    if (xEnd >= xStart && yEnd >= yStart)
        b.rect = QRect(xStart, yStart, xEnd - xStart + 1, yEnd - yStart + 1);
    b.command = QByteArray(reinterpret_cast<const char *>(d + 10), 8);
    return b;
}

ButtonSet parsePci(const QByteArray &sector)
{
    ButtonSet out;
    // Locate the private_stream_2 (0xBF) PES; pci_t begins after its
    // substream-id byte (which must be 0x00 = PCI, not 0x01 = DSI).
    const int nav = sector.indexOf(QByteArrayLiteral("\x00\x00\x01\xBF"));
    if (nav < 0 || nav + 7 > sector.size() || quint8(sector.at(nav + 6)) != 0x00)
        return out;
    const int pci = nav + 7;

    if (pci + kBtnItOffset > sector.size())
        return out;
    const int btnNs = quint8(sector.at(pci + kBtnNsOffset)) & 0x3F;
    if (btnNs <= 0 || btnNs > 36)
        return out;

    const uchar *base = reinterpret_cast<const uchar *>(sector.constData()) + pci;
    // fosl_btnn (forcedly-selected button, hl_gi offset 20, low 6 bits) is the
    // initial highlight; default to button 1 when unset.
    out.startButton = quint8(sector.at(pci + kHlgiOffset + 20)) & 0x3F;
    if (out.startButton < 1 || out.startButton > btnNs)
        out.startButton = 1;

    // BTN_COLIT: three (select, action) colour-contrast sets.
    for (int i = 0; i < 3; ++i) {
        out.selectColor[i] = be32(sector, pci + kBtnColitOffset + i * 8);
        out.actionColor[i] = be32(sector, pci + kBtnColitOffset + i * 8 + 4);
    }

    for (int i = 0; i < btnNs; ++i) {
        const int off = pci + kBtnItOffset + i * 18;
        if (off + 18 > sector.size())
            break;
        out.buttons.append(parseButton(base + kBtnItOffset + i * 18));
    }
    return out;
}

} // namespace

ButtonSet parseButtons(const QString &vobPath, qint64 firstSector,
                       qint64 lastSector)
{
    QFile file(vobPath);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    if (!file.seek(firstSector * sectorSize))
        return {};

    for (qint64 s = firstSector; s <= lastSector; ++s) {
        const QByteArray sector = file.read(sectorSize);
        if (sector.size() < sectorSize)
            break;
        // NAV packs are pack-headed (00 00 01 BA).
        if (!sector.startsWith(QByteArrayLiteral("\x00\x00\x01\xBA")))
            continue;
        const ButtonSet set = parsePci(sector);
        if (set.isValid())
            return set;
    }
    return {};
}

} // namespace DvdMenu
