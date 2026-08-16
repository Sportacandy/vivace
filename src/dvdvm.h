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

    // Set by a System-Set instruction (systemSet()'s case 1) actually
    // WRITING sprm[1]/sprm[2] -- true on execution, independent of whether
    // the new value differs from the old one. A caller who needs to know
    // "did a specific run() actually assign to this register" (e.g. to
    // distinguish an explicit user menu choice from some other command
    // chain that happens to write the same value) should clear these
    // before calling run() and check them after -- NOT compare sprm[]
    // before/after, which misses the case where the newly-chosen value
    // happens to equal the old one (found 2026-08-16: a disc's "no
    // subtitle" button writes 0, which can coincide with an unrelated
    // command chain's own earlier write of 0, making a before/after value
    // diff silently miss the real button click).
    bool touchedSprm1 = false;
    bool touchedSprm2 = false;

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
