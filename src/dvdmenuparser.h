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
    // still_time (PGC + 0xA2, per the DVD-Video spec): 0 = never freeze on
    // the last frame, run post-commands the instant the cells finish
    // playing; 1-254 = hold the last frame that many seconds then run
    // post-commands; 0xFF = hold forever until the user picks a button.
    // This is the disc's own authoritative signal for "is this PGC a
    // one-shot clip that should advance on its own" vs "a real menu that
    // waits for input" -- NOT whether it happens to declare any buttons
    // (an intro/logo animation can have a single dummy button just so a
    // remote's ENTER can skip it, without being a real interactive menu).
    int stillTime = 0;
    // Program map (PGC + 0xE6): 1-based program N's entry cell, 0-based
    // (programEntryCells[N-1] = the cell index program N starts at).
    // A DVD "program" groups one or more consecutive cells; on a real
    // disc a multi-page menu (see DvdIfo::Cell::stillTime's own doc
    // comment) typically has ONE PROGRAM PER PAGE, each starting at its
    // own cell, and a button's LinkPgn(N) command jumps straight to
    // program N's entry cell rather than playing through earlier pages
    // first (found 2026-08-15: a 4-cell, 4-program chapter-index PGC
    // whose "1-6/7-12/13-18/19-21" buttons are LinkPgn(1..4)).
    QList<int> programEntryCells;
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
