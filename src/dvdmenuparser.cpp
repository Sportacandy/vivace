/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Offsets follow the DVD-Video specification (VMGI_MAT / VTSI_MAT sector
    pointers, PGCI_UT / PGCIT / PGC layout) and were validated byte-for-byte
    against real menu discs. Every read is bounds-checked; anomalies leave
    the affected domain empty (the caller falls back to plain title play).
*/

#include "dvdmenuparser.h"

#include <QDir>
#include <QFile>
#include <QtEndian>

namespace DvdMenu {

namespace {

constexpr qint64 sectorSize = 2048;

quint8 u8(const QByteArray &d, qint64 o)
{
    return o >= 0 && o < d.size() ? quint8(d.at(o)) : 0;
}
quint16 be16(const QByteArray &d, qint64 o)
{
    return (o < 0 || o + 2 > d.size()) ? 0 : qFromBigEndian<quint16>(d.constData() + o);
}
quint32 be32(const QByteArray &d, qint64 o)
{
    return (o < 0 || o + 4 > d.size()) ? 0 : qFromBigEndian<quint32>(d.constData() + o);
}

QByteArray readIfo(const QDir &dir, const QString &name)
{
    QFile file(dir.filePath(name));
    if (!file.open(QIODevice::ReadOnly)) {
        file.setFileName(dir.filePath(name.toLower()));
        if (!file.open(QIODevice::ReadOnly))
            return {};
    }
    return file.readAll();
}

// Existing menu VOB for a domain, or empty if none is present.
QString menuVob(const QDir &dir, int vtsNumber)
{
    const QString name = vtsNumber == 0
            ? QStringLiteral("VIDEO_TS.VOB")
            : QStringLiteral("VTS_%1_0.VOB").arg(vtsNumber, 2, 10, u'0');
    if (dir.exists(name))
        return dir.filePath(name);
    if (dir.exists(name.toLower()))
        return dir.filePath(name.toLower());
    return {};
}

QByteArray command(const QByteArray &ifo, qint64 offset)
{
    if (offset < 0 || offset + 8 > ifo.size())
        return QByteArray(8, '\0');
    return ifo.mid(offset, 8);
}

// Parse one PGC (menu domain) at absolute offset `pgc` in `ifo`.
Pgc parsePgc(const QByteArray &ifo, qint64 pgc, int menuId, bool entry)
{
    Pgc out;
    out.menuId = menuId;
    out.entry = entry;

    const int cellCount = u8(ifo, pgc + 3);
    const qint64 cmdTable = pgc + be16(ifo, pgc + 0xE4);
    const qint64 cellTable = pgc + be16(ifo, pgc + 0xE8);

    // Highlight palette: 16 entries of (reserved, Y, Cr, Cb) at PGC + 0xA4.
    for (int i = 0; i < 16; ++i)
        out.palette[i] = be32(ifo, pgc + 0xA4 + qint64(i) * 4) & 0xFFFFFF;

    // Command table: pre / post / cell counts, then 8-byte VM commands.
    if (be16(ifo, pgc + 0xE4) != 0) {
        const int nPre = be16(ifo, cmdTable);
        const int nPost = be16(ifo, cmdTable + 2);
        const int nCell = be16(ifo, cmdTable + 4);
        qint64 p = cmdTable + 8;
        for (int i = 0; i < nPre && i < 128; ++i, p += 8)
            out.preCommands.append(command(ifo, p));
        for (int i = 0; i < nPost && i < 128; ++i, p += 8)
            out.postCommands.append(command(ifo, p));
        for (int i = 0; i < nCell && i < 128; ++i, p += 8)
            out.cellCommands.append(command(ifo, p));
    }

    for (int c = 0; c < cellCount; ++c) {
        const qint64 cell = cellTable + qint64(c) * 24;
        DvdIfo::Cell range;
        range.firstSector = be32(ifo, cell + 0x08);
        range.lastSector = be32(ifo, cell + 0x14);
        if (range.lastSector >= range.firstSector)
            out.cells.append(range);
    }
    return out;
}

// Parse a PGCI_UT (menu PGC unit table) whose header is at absolute
// offset `base` in `ifo`. Reads the first language unit only.
Domain parseDomain(const QByteArray &ifo, qint64 base, int vtsNumber,
                   const QString &vobPath)
{
    Domain domain;
    domain.vtsNumber = vtsNumber;

    const int nLu = be16(ifo, base);
    if (nLu <= 0 || nLu > 100)
        return domain;

    // First language unit -> its PGCIT.
    const qint64 lu = base + 8; // LU[0]
    const qint64 pgcit = base + be32(ifo, lu + 4);
    const int nPgc = be16(ifo, pgcit);
    if (nPgc <= 0 || nPgc > 512)
        return domain;

    for (int p = 0; p < nPgc; ++p) {
        const qint64 srp = pgcit + 8 + qint64(p) * 8;
        const quint8 cat = u8(ifo, srp);
        const bool entry = (cat & 0x80) != 0;
        const int menuId = cat & 0x0F;
        const qint64 pgc = pgcit + be32(ifo, srp + 4);
        domain.pgcs.append(parsePgc(ifo, pgc, entry ? menuId : 0, entry));
    }

    if (!domain.pgcs.isEmpty())
        domain.vobPath = vobPath;
    return domain;
}

} // namespace

int Domain::entryPgc(int menuId) const
{
    for (int i = 0; i < pgcs.size(); ++i) {
        if (pgcs.at(i).entry && pgcs.at(i).menuId == menuId)
            return i + 1;
    }
    return 0;
}

bool Structure::hasMenus() const
{
    if (vmgm.isValid())
        return true;
    for (const Domain &d : vtsm)
        if (d.isValid())
            return true;
    return false;
}

Structure parse(const QString &videoTsDir)
{
    Structure out;
    const QDir dir(videoTsDir);

    const QByteArray vmg = readIfo(dir, QStringLiteral("VIDEO_TS.IFO"));
    if (!vmg.startsWith(QByteArrayLiteral("DVDVIDEO-VMG")))
        return out;

    // First-Play PGC: VMGI_MAT byte offset (not a sector) at 0x84. Its cells,
    // if any, live in the VMGM menu VOB (VIDEO_TS.VOB).
    const qint64 fpOffset = qint64(be32(vmg, 0x84));
    if (fpOffset > 0 && fpOffset < vmg.size())
        out.firstPlay = parsePgc(vmg, fpOffset, 0, false);

    // VMGM_PGCI_UT: VMGI_MAT sector pointer at 0xC8.
    const qint64 vmgmPgciUt = qint64(be32(vmg, 0xC8)) * sectorSize;
    if (vmgmPgciUt > 0) {
        const QString vob = menuVob(dir, 0);
        if (!vob.isEmpty())
            out.vmgm = parseDomain(vmg, vmgmPgciUt, 0, vob);
    }

    // Enumerate the title sets from TT_SRPT (VMGI_MAT sector pointer 0xC4),
    // then parse each VTSM_PGCI_UT (VTSI_MAT sector pointer 0xD0).
    const qint64 ttSrpt = qint64(be32(vmg, 0xC4)) * sectorSize;
    const int titleCount = be16(vmg, ttSrpt);
    QList<int> vtsSeen;
    for (int i = 0; i < titleCount && i < 99; ++i) {
        const int vtsNumber = u8(vmg, ttSrpt + 8 + qint64(i) * 12 + 6);
        if (vtsNumber < 1 || vtsNumber > 99 || vtsSeen.contains(vtsNumber))
            continue;
        vtsSeen.append(vtsNumber);

        const QByteArray vts = readIfo(dir, QStringLiteral("VTS_%1_0.IFO")
                                                .arg(vtsNumber, 2, 10, u'0'));
        if (!vts.startsWith(QByteArrayLiteral("DVDVIDEO-VTS")))
            continue;
        const qint64 vtsmPgciUt = qint64(be32(vts, 0xD0)) * sectorSize;
        if (vtsmPgciUt <= 0)
            continue;
        const QString vob = menuVob(dir, vtsNumber);
        if (vob.isEmpty())
            continue;
        const Domain domain = parseDomain(vts, vtsmPgciUt, vtsNumber, vob);
        if (domain.isValid())
            out.vtsm.insert(vtsNumber, domain);
    }

    return out;
}

} // namespace DvdMenu
