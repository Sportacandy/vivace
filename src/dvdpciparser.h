/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef DVDPCIPARSER_H
#define DVDPCIPARSER_H

#include <QByteArray>
#include <QList>
#include <QRect>
#include <QString>

/*  Parses DVD-Video button information from a menu VOB's NAV packs. Each
    VOBU begins with a NAV pack whose PCI (Presentation Control Information,
    a private_stream_2 PES) carries the highlight info: the number of
    buttons, each button's rectangle in the sub-picture/overlay coordinate
    space (up to 720x480 NTSC / 720x576 PAL), its up/down/left/right
    neighbours, and its 8-byte VM command. Layout per libdvdread nav_types.h,
    validated byte-for-byte against real discs. */
namespace DvdMenu {

struct Button {
    QRect rect;         // overlay coordinates
    bool autoAction = false;
    int colorNumber = 0; // btn_coln: index into the BTN_COLIT sets (0-2)
    int up = 0, down = 0, left = 0, right = 0; // 1-based, 0 = none
    QByteArray command; // 8 bytes
};

struct ButtonSet {
    QList<Button> buttons;
    int startButton = 1; // fosl_btnn (1-based)
    // BTN_COLIT: three colour-contrast sets; [set][0] = select, [1] = action.
    // Each is nibbles [Ci3 Ci2 Ci1 Ci0 A3 A2 A1 A0].
    quint32 selectColor[3] = { 0, 0, 0 };
    quint32 actionColor[3] = { 0, 0, 0 };
    bool isValid() const { return !buttons.isEmpty(); }
    // Select colour-contrast for a button (by its btn_coln).
    quint32 selectFor(int buttonIndex) const
    {
        if (buttonIndex < 1 || buttonIndex > buttons.size())
            return 0;
        const int c = buttons.at(buttonIndex - 1).colorNumber;
        return selectColor[c < 0 || c > 2 ? 0 : c];
    }
};

// Reads the first NAV pack carrying buttons within the sector range
// [firstSector, lastSector] of `vobPath`. Empty if the menu has none.
ButtonSet parseButtons(const QString &vobPath, qint64 firstSector,
                       qint64 lastSector);

} // namespace DvdMenu

#endif // DVDPCIPARSER_H
