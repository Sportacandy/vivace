/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef DVDVM_H
#define DVDVM_H

#include <QByteArray>
#include <QList>

/*  A minimal DVD-Video navigation virtual machine — enough to run menu
    PGC pre/post commands and button commands so a "menu-lite" navigation
    works (show menus, pick a title). Ported from the semantics of
    libdvdnav's decoder.c (GPL-2+); it decodes an 8-byte command block,
    maintains the 16 GPRM / 24 SPRM registers, and yields the resulting
    navigation Action. It does NOT itself change playback — PlayerController
    resolves the Action against the parsed disc structure.

    Not implemented (beyond menu-lite): GPRM counter mode, RCE region
    checks, seamless-angle/parental subtleties. */
namespace DvdVm {

struct Action {
    enum Kind {
        None,          // block fell through, no link/jump
        Nop,
        Exit,
        // Link within the current domain / PGC:
        LinkPgcn,      // data1 = PGC number (menu domain)
        LinkPttn,      // data1 = PTT
        LinkPgn,       // data1 = program number (current PGC)
        LinkCn,        // data1 = cell number (current PGC)
        LinkTailPgc,
        LinkTopPgc, LinkNextPgc, LinkPrevPgc, LinkGoUpPgc,
        LinkTopCell, LinkNextCell, LinkPrevCell,
        LinkTopPg, LinkNextPg, LinkPrevPg,
        // Jumps (title domain / menu space):
        JumpTt,        // data1 = title (VMG global)
        JumpVtsTt,     // data1 = title (within current VTS)
        JumpVtsPtt,    // data1 = title, data2 = part
        JumpSsFp,
        JumpSsVmgmMenu,   // data1 = menu id
        JumpSsVtsm,       // data1 = vts, data2 = title, data3 = menu id
        JumpSsVmgmPgc,    // data1 = pgc
        CallSsFp,
        CallSsVmgmMenu,   // data1 = menu id
        CallSsVtsm,       // data1 = menu id
        CallSsVmgmPgc,    // data1 = pgc
    } kind = None;
    int data1 = 0;
    int data2 = 0;
    int data3 = 0;
    int button = 0; // link-subinstruction highlighted button (0 = keep)
};

class Machine
{
public:
    Machine() { reset(); }
    void reset();

    // Executes a command block (each entry 8 bytes). Returns the Action of
    // the first link/jump reached, or {None} if the block completes without
    // one. Registers are modified in place and persist across calls.
    Action run(const QList<QByteArray> &commands);

    quint16 gprm[16];
    quint16 sprm[24];

private:
    struct Cmd {
        quint64 instruction = 0;
    };
    quint32 bits(const Cmd &c, int start, int count) const;
    quint16 reg(const Cmd &c, quint8 r) const;
    quint16 regOrData(const Cmd &c, int imm, int start) const;
    quint16 regOrData2(const Cmd &c, int imm, int start) const;
    int compare(int op, quint16 a, quint16 b) const;
    int ifV1(const Cmd &c) const;
    int ifV2(const Cmd &c) const;
    int ifV3(const Cmd &c) const;
    int ifV4(const Cmd &c) const;
    int special(const Cmd &c, int cond);
    void setOp(int op, int r, int r2, int data);
    void setV1(const Cmd &c, int cond);
    void setV2(const Cmd &c, int cond);
    bool linkSub(const Cmd &c, int cond, Action &out) const;
    bool linkInstr(const Cmd &c, int cond, Action &out) const;
    bool jumpInstr(const Cmd &c, int cond, Action &out) const;
    bool systemSet(const Cmd &c, int cond, Action &out);
    // Returns >0 goto line (1-based), 0 continue, -1 link (fills `out`).
    int evalCommand(const QByteArray &bytes, Action &out);
};

} // namespace DvdVm

#endif // DVDVM_H
