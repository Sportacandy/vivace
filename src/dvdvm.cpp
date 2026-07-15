/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Ported from the evaluation semantics of libdvdnav's src/vm/decoder.c
    (Håkan Hjort, GPL-2-or-later) — reusable under GPL-3. Only the subset
    needed for menu navigation is kept; GPRM counter mode is treated as a
    plain register.
*/

#include "dvdvm.h"

#include <QRandomGenerator>

namespace DvdVm {

void Machine::reset()
{
    for (quint16 &g : gprm)
        g = 0;
    for (quint16 &s : sprm)
        s = 0;
    // A few SPRM defaults matching a typical player state.
    sprm[0] = ('e' << 8) | 'n'; // menu language (informational)
    sprm[1] = 15;               // audio stream: none/default
    sprm[2] = 62;               // subpicture stream: none
    sprm[3] = 15;               // angle
    sprm[8] = 0;                // highlighted button (times 1024)
    sprm[9] = 0;                // navigation timer
}

quint32 Machine::bits(const Cmd &c, int start, int count) const
{
    if (count == 0 || count > 32 || start > 63 || start < 0 || (start - count) < -1)
        return 0;
    const int shift = start - count + 1;
    const quint64 mask = (count == 32) ? 0xFFFFFFFFull : ((1ull << count) - 1);
    return quint32((c.instruction >> shift) & mask);
}

quint16 Machine::reg(const Cmd &c, quint8 r) const
{
    if (r & 0x80)
        return sprm[r & 0x1F]; // SPRM (max 24; masking to 0x1F is harmless)
    return gprm[r & 0x0F];
}

quint16 Machine::regOrData(const Cmd &c, int imm, int start) const
{
    if (imm)
        return quint16(bits(c, start, 16));
    return reg(c, quint8(bits(c, start - 8, 8)));
}

quint16 Machine::regOrData2(const Cmd &c, int imm, int start) const
{
    if (imm)
        return quint16(bits(c, start - 1, 7));
    return gprm[bits(c, start - 4, 4) & 0x0F];
}

int Machine::compare(int op, quint16 a, quint16 b) const
{
    switch (op) {
    case 1: return a & b;
    case 2: return a == b;
    case 3: return a != b;
    case 4: return a >= b;
    case 5: return a > b;
    case 6: return a <= b;
    case 7: return a < b;
    }
    return 0;
}

int Machine::ifV1(const Cmd &c) const
{
    const int op = bits(c, 54, 3);
    if (op)
        return compare(op, reg(c, quint8(bits(c, 39, 8))),
                       regOrData(c, bits(c, 55, 1), 31));
    return 1;
}

int Machine::ifV2(const Cmd &c) const
{
    const int op = bits(c, 54, 3);
    if (op)
        return compare(op, reg(c, quint8(bits(c, 15, 8))),
                       reg(c, quint8(bits(c, 7, 8))));
    return 1;
}

int Machine::ifV3(const Cmd &c) const
{
    const int op = bits(c, 54, 3);
    if (op)
        return compare(op, reg(c, quint8(bits(c, 47, 8))),
                       regOrData(c, bits(c, 55, 1), 15));
    return 1;
}

int Machine::ifV4(const Cmd &c) const
{
    const int op = bits(c, 54, 3);
    if (op)
        return compare(op, reg(c, quint8(bits(c, 51, 4))),
                       regOrData(c, bits(c, 55, 1), 31));
    return 1;
}

int Machine::special(const Cmd &c, int cond)
{
    switch (bits(c, 51, 4)) {
    case 0: // NOP
        return 0;
    case 1: // Goto line
        return cond ? int(bits(c, 7, 8)) : 0;
    case 2: // Break
        return cond ? 256 : 0;
    case 3: // Set temporary parental level and goto (always succeeds)
        if (cond)
            sprm[13] = quint16(bits(c, 11, 4));
        return cond ? int(bits(c, 7, 8)) : 0;
    }
    return 0;
}

void Machine::setOp(int op, int r, int r2, int data)
{
    constexpr int shortmax = 0xFFFF;
    int tmp;
    switch (op) {
    case 1: gprm[r] = quint16(data); break;
    case 2: // swap
        gprm[r2] = gprm[r];
        gprm[r] = quint16(data);
        break;
    case 3: tmp = gprm[r] + data; gprm[r] = quint16(tmp > shortmax ? shortmax : tmp); break;
    case 4: tmp = gprm[r] - data; gprm[r] = quint16(tmp < 0 ? 0 : tmp); break;
    case 5: tmp = gprm[r] * data; gprm[r] = quint16(tmp > shortmax ? shortmax : tmp); break;
    case 6: gprm[r] = data ? quint16(gprm[r] / data) : 0xFFFF; break;
    case 7: gprm[r] = data ? quint16(gprm[r] % data) : 0xFFFF; break;
    case 8: gprm[r] = quint16(1 + (QRandomGenerator::global()->bounded(data > 0 ? data : 1))); break;
    case 9: gprm[r] = quint16(gprm[r] & data); break;
    case 10: gprm[r] = quint16(gprm[r] | data); break;
    case 11: gprm[r] = quint16(gprm[r] ^ data); break;
    }
}

void Machine::setV1(const Cmd &c, int cond)
{
    const int op = bits(c, 59, 4);
    const int r = bits(c, 35, 4);
    const int r2 = bits(c, 19, 4);
    const int data = regOrData(c, bits(c, 60, 1), 31);
    if (cond)
        setOp(op, r, r2, data);
}

void Machine::setV2(const Cmd &c, int cond)
{
    const int op = bits(c, 59, 4);
    const int r = bits(c, 51, 4);
    const int r2 = bits(c, 35, 4);
    const int data = regOrData(c, bits(c, 60, 1), 47);
    if (cond)
        setOp(op, r, r2, data);
}

bool Machine::linkSub(const Cmd &c, int cond, Action &out) const
{
    const int button = bits(c, 15, 6);
    const int linkop = bits(c, 4, 5);
    if (!cond)
        return false;
    out.button = button;
    switch (linkop) {
    case 0: out.kind = Action::Nop; break;
    case 1: out.kind = Action::LinkTopCell; break;
    case 2: out.kind = Action::LinkNextCell; break;
    case 3: out.kind = Action::LinkPrevCell; break;
    case 5: out.kind = Action::LinkTopPg; break;
    case 6: out.kind = Action::LinkNextPg; break;
    case 7: out.kind = Action::LinkPrevPg; break;
    case 9: out.kind = Action::LinkTopPgc; break;
    case 10: out.kind = Action::LinkNextPgc; break;
    case 11: out.kind = Action::LinkPrevPgc; break;
    case 12: out.kind = Action::LinkGoUpPgc; break;
    case 13: out.kind = Action::LinkTailPgc; break;
    default: out.kind = Action::Nop; break;
    }
    return true;
}

bool Machine::linkInstr(const Cmd &c, int cond, Action &out) const
{
    const int op = bits(c, 51, 4);
    switch (op) {
    case 1:
        return linkSub(c, cond, out);
    case 4:
        if (!cond) return false;
        out.kind = Action::LinkPgcn;
        out.data1 = bits(c, 14, 15);
        return true;
    case 5:
        if (!cond) return false;
        out.kind = Action::LinkPttn;
        out.data1 = bits(c, 9, 10);
        out.button = bits(c, 15, 6);
        return true;
    case 6:
        if (!cond) return false;
        out.kind = Action::LinkPgn;
        out.data1 = bits(c, 6, 7);
        out.button = bits(c, 15, 6);
        return true;
    case 7:
        if (!cond) return false;
        out.kind = Action::LinkCn;
        out.data1 = bits(c, 7, 8);
        out.button = bits(c, 15, 6);
        return true;
    }
    return false;
}

bool Machine::jumpInstr(const Cmd &c, int cond, Action &out) const
{
    if (!cond)
        return false;
    switch (bits(c, 51, 4)) {
    case 1: out.kind = Action::Exit; return true;
    case 2: out.kind = Action::JumpTt; out.data1 = bits(c, 22, 7); return true;
    case 3: out.kind = Action::JumpVtsTt; out.data1 = bits(c, 22, 7); return true;
    case 5:
        out.kind = Action::JumpVtsPtt;
        out.data1 = bits(c, 22, 7);
        out.data2 = bits(c, 41, 10);
        return true;
    case 6:
        switch (bits(c, 23, 2)) {
        case 0: out.kind = Action::JumpSsFp; return true;
        case 1: out.kind = Action::JumpSsVmgmMenu; out.data1 = bits(c, 19, 4); return true;
        case 2:
            out.kind = Action::JumpSsVtsm;
            out.data1 = bits(c, 31, 8);
            out.data2 = bits(c, 39, 8);
            out.data3 = bits(c, 19, 4);
            return true;
        case 3: out.kind = Action::JumpSsVmgmPgc; out.data1 = bits(c, 46, 15); return true;
        }
        break;
    case 8:
        switch (bits(c, 23, 2)) {
        case 0: out.kind = Action::CallSsFp; out.data1 = bits(c, 31, 8); return true;
        case 1: out.kind = Action::CallSsVmgmMenu; out.data1 = bits(c, 19, 4); out.data2 = bits(c, 31, 8); return true;
        case 2: out.kind = Action::CallSsVtsm; out.data1 = bits(c, 19, 4); out.data2 = bits(c, 31, 8); return true;
        case 3: out.kind = Action::CallSsVmgmPgc; out.data1 = bits(c, 46, 15); out.data2 = bits(c, 31, 8); return true;
        }
        break;
    }
    return false;
}

bool Machine::systemSet(const Cmd &c, int cond, Action &out)
{
    switch (bits(c, 59, 4)) {
    case 1: // SPRM 1/2/3 (audio, subpicture, angle)
        for (int i = 1; i <= 3; ++i) {
            if (bits(c, 63 - ((2 + i) * 8), 1)) {
                const quint16 data = regOrData2(c, bits(c, 60, 1), 47 - (i * 8));
                if (cond)
                    sprm[i] = data;
            }
        }
        break;
    case 2: // SPRM 9 (nav timer) & 10 (title PGC)
        if (cond) {
            sprm[9] = regOrData(c, bits(c, 60, 1), 47);
            sprm[10] = quint16(bits(c, 30, 15));
        }
        break;
    case 3: { // SetMode counter/register + set
        const int r = bits(c, 19, 4);
        const int data = regOrData(c, bits(c, 60, 1), 47);
        if (cond)
            setOp(1, r, 0, data);
        break;
    }
    case 6: // SPRM 8 (highlighted button)
        if (cond) {
            if (bits(c, 60, 1))
                sprm[8] = quint16(bits(c, 31, 16));
            else
                sprm[8] = gprm[bits(c, 19, 4) & 0x0F];
        }
        break;
    }
    // A system-set may carry a trailing link.
    if (bits(c, 51, 4))
        return linkInstr(c, cond, out);
    return false;
}

int Machine::evalCommand(const QByteArray &bytes, Action &out)
{
    Cmd c;
    const uchar *b = reinterpret_cast<const uchar *>(bytes.constData());
    if (bytes.size() < 8)
        return 0;
    c.instruction = (quint64(b[0]) << 56) | (quint64(b[1]) << 48)
            | (quint64(b[2]) << 40) | (quint64(b[3]) << 32)
            | (quint64(b[4]) << 24) | (quint64(b[5]) << 16)
            | (quint64(b[6]) << 8) | quint64(b[7]);

    int cond;
    bool linked = false;
    switch (bits(c, 63, 3)) {
    case 0: // special
        cond = ifV1(c);
        return special(c, cond);
    case 1: // link / jump
        if (bits(c, 60, 1)) {
            cond = ifV2(c);
            linked = jumpInstr(c, cond, out);
        } else {
            cond = ifV1(c);
            linked = linkInstr(c, cond, out);
        }
        break;
    case 2: // system set
        cond = ifV2(c);
        linked = systemSet(c, cond, out);
        break;
    case 3: // set gprm, then maybe link
        cond = ifV3(c);
        setV1(c, cond);
        if (bits(c, 51, 4))
            linked = linkInstr(c, cond, out);
        break;
    case 4: // set, compare -> link sub
        setV2(c, 1);
        cond = ifV4(c);
        linked = linkSub(c, cond, out);
        break;
    case 5: // compare -> (set and link sub)
        cond = ifV4(c);
        setV2(c, cond);
        linked = linkSub(c, cond, out);
        break;
    case 6: // compare -> set, always link sub
        cond = ifV4(c);
        setV2(c, cond);
        linked = linkSub(c, 1, out);
        break;
    default:
        break;
    }
    return linked ? -1 : 0;
}

Action Machine::run(const QList<QByteArray> &commands)
{
    Action out;
    int i = 0;
    int total = 0;
    const int n = int(commands.size());
    while (i < n && total < 100000) {
        Action step;
        const int line = evalCommand(commands.at(i), step);
        if (line < 0) {
            out = step;
            return out; // link/jump reached
        }
        if (line > 0)
            i = line - 1; // goto (1-based); Break (256) ends the loop
        else
            ++i;
        ++total;
    }
    return out; // {None}
}

} // namespace DvdVm
