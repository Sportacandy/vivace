/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef DVDSPU_H
#define DVDSPU_H

#include <QByteArray>
#include <QImage>
#include <QRect>
#include <QString>

/*  DVD-Video subpicture (SPU) decoding for menu highlights. A menu's
    subpicture is a full-screen 4-colour RLE bitmap (indices 0-3) shown over
    the video; the PGC palette + the SPU's SET_COLOR/SET_CONTR map those
    indices to colours/alpha. A selected button recolours the indices within
    its rectangle using the button's select colour-contrast (PCI BTN_COLIT).

    We decode the menu's still subpicture once, then compose an ARGB overlay
    per selection. Layout per the DVD spec, validated against real discs. */
namespace DvdMenu {

struct Subpicture {
    int width = 0;
    int height = 0;
    QByteArray pixels;          // width*height, each byte a colour index 0-3
    int baseColor[4] = { 0, 1, 2, 3 }; // SPU index -> PGC palette entry
    int baseAlpha[4] = { 0, 0, 0, 0 }; // SPU index -> alpha 0-15
    bool isValid() const { return width > 0 && height > 0
                                  && pixels.size() == qsizetype(width) * height; }
};

// Extract and decode the still menu subpicture from the VOB cell range.
Subpicture decodeSubpicture(const QString &vobPath, qint64 firstSector,
                            qint64 lastSector);

// Compose the subpicture to an ARGB image using the PGC palette. Inside
// `selectRect` the indices are remapped by `selectColorContrast` (a BTN_COLIT
// entry: nibbles [Ci3 Ci2 Ci1 Ci0 A3 A2 A1 A0]); elsewhere the base
// SET_COLOR/SET_CONTR applies. Returns a null image when nothing is visible.
QImage renderHighlight(const Subpicture &sp, const quint32 palette[16],
                       const QRect &selectRect, quint32 selectColorContrast);

} // namespace DvdMenu

#endif // DVDSPU_H
