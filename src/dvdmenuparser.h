/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef DVDMENUPARSER_H
#define DVDMENUPARSER_H

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QString>

#include "dvdifoparser.h"

/*  DVD-Video menu structure parsing (experimental "menu-lite"): the
    VMGM_PGCI_UT (VIDEO_TS.IFO) and each VTSM_PGCI_UT (VTS_xx_0.IFO) menu
    PGC tables — the menu program chains, their cells (sector ranges within
    the menu VOB domain: VIDEO_TS.VOB for VMGM, VTS_xx_0.VOB for VTSM) and
    the pre / post / cell command blocks that a small VM runs to navigate.

    Only the first language unit is read (menus rarely differ per language
    in a way that matters for navigation). Menu ids: 2 title, 3 root,
    4 subpicture, 5 audio, 6 angle, 7 chapter/ptt. */
namespace DvdMenu {

struct Pgc {
    int menuId = 0;   // 0 when not an entry PGC (referenced only by number)
    bool entry = false;
    QList<DvdIfo::Cell> cells; // sector ranges within the domain menu VOB
    QList<QByteArray> preCommands;  // each 8 bytes
    QList<QByteArray> postCommands;
    QList<QByteArray> cellCommands;
    quint32 palette[16] = { 0 }; // highlight palette (YCrCb), for subpictures
};

struct Domain {
    int vtsNumber = 0;   // 0 = VMGM, >= 1 = that title set's VTSM
    QString vobPath;     // the menu VOB file
    QList<Pgc> pgcs;     // pgcs[i] is logical PGC (i + 1)

    // 1-based PGC number of the entry PGC for `menuId`, or 0 if none.
    int entryPgc(int menuId) const;
    bool isValid() const { return !vobPath.isEmpty() && !pgcs.isEmpty(); }
};

struct Structure {
    Domain vmgm;
    QHash<int, Domain> vtsm; // vtsNumber -> VTSM domain
    Pgc firstPlay;           // VMGI first-play PGC (cells in the VMGM VOB)

    bool hasMenus() const;
    bool hasFirstPlay() const
    {
        return !firstPlay.preCommands.isEmpty() || !firstPlay.cells.isEmpty()
                || !firstPlay.postCommands.isEmpty();
    }
};

Structure parse(const QString &videoTsDir);

} // namespace DvdMenu

#endif // DVDMENUPARSER_H
