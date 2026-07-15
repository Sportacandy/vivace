/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "dvdtitledevice.h"

#include <QDir>
#include <QMap>
#include <QRegularExpression>

namespace {
constexpr qint64 sectorSize = 2048;
}

DvdTitleDevice::DvdTitleDevice(QObject *parent)
    : QIODevice(parent)
{
}

QList<DvdTitleDevice::VobPart>
DvdTitleDevice::titleSetVobs(const QString &videoTsDir, int vtsNumber)
{
    const QDir dir(videoTsDir);
    QList<VobPart> parts;
    qint64 offset = 0;
    for (int part = 1; part <= 9; ++part) {
        QFileInfo info(dir.filePath(QStringLiteral("VTS_%1_%2.VOB")
                               .arg(vtsNumber, 2, 10, u'0').arg(part)));
        if (!info.exists()) {
            info = QFileInfo(dir.filePath(QStringLiteral("vts_%1_%2.vob")
                                     .arg(vtsNumber, 2, 10, u'0').arg(part)));
            if (!info.exists())
                break;
        }
        parts.append({ info.absoluteFilePath(), info.size(), offset });
        offset += info.size();
    }
    return parts;
}

void DvdTitleDevice::appendDomainRange(const QList<VobPart> &vobs,
                                       qint64 domainStart, qint64 domainEnd)
{
    for (const VobPart &vob : vobs) {
        const qint64 vobEnd = vob.domainOffset + vob.size;
        const qint64 start = qMax(domainStart, vob.domainOffset);
        const qint64 end = qMin(domainEnd, vobEnd);
        if (start >= end)
            continue;
        m_segments.append({ vob.path, start - vob.domainOffset, end - start,
                            m_totalSize });
        m_totalSize += end - start;
    }
}

DvdTitleDevice *DvdTitleDevice::createFromCells(const QString &videoTsDir,
                                                int vtsNumber,
                                                const QList<DvdIfo::Cell> &cells,
                                                QObject *parent)
{
    const QList<VobPart> vobs = titleSetVobs(videoTsDir, vtsNumber);
    if (vobs.isEmpty() || cells.isEmpty())
        return nullptr;

    auto *device = new DvdTitleDevice(parent);
    for (const DvdIfo::Cell &cell : cells) {
        device->appendDomainRange(vobs, cell.firstSector * sectorSize,
                                  (cell.lastSector + 1) * sectorSize);
    }
    if (device->m_segments.isEmpty()) {
        delete device;
        return nullptr;
    }
    return device;
}

DvdTitleDevice *DvdTitleDevice::createFromMenuCells(
        const QString &vobPath, const QList<DvdIfo::Cell> &cells,
        QObject *parent)
{
    QFileInfo info(vobPath);
    if (!info.exists() || cells.isEmpty())
        return nullptr;

    const QList<VobPart> vobs = { { info.absoluteFilePath(), info.size(), 0 } };
    auto *device = new DvdTitleDevice(parent);
    for (const DvdIfo::Cell &cell : cells) {
        device->appendDomainRange(vobs, cell.firstSector * sectorSize,
                                  (cell.lastSector + 1) * sectorSize);
    }
    if (device->m_segments.isEmpty()) {
        delete device;
        return nullptr;
    }
    return device;
}

DvdTitleDevice *DvdTitleDevice::create(const QString &videoTsDir,
                                       QObject *parent)
{
    // IFO-less fallback: the whole VOB chain of the largest title set.
    const QDir dir(videoTsDir);
    static const QRegularExpression pattern(
            QStringLiteral("^VTS_(\\d{2})_([1-9])\\.VOB$"),
            QRegularExpression::CaseInsensitiveOption);

    QMap<int, qint64> setSizes;
    const QFileInfoList files = dir.entryInfoList(
            { QStringLiteral("VTS_*.VOB"), QStringLiteral("vts_*.vob") },
            QDir::Files);
    for (const QFileInfo &info : files) {
        const QRegularExpressionMatch match = pattern.match(info.fileName());
        if (match.hasMatch())
            setSizes[match.captured(1).toInt()] += info.size();
    }
    if (setSizes.isEmpty())
        return nullptr;

    int mainSet = -1;
    qint64 mainSize = -1;
    for (auto it = setSizes.constBegin(); it != setSizes.constEnd(); ++it) {
        if (it.value() > mainSize) {
            mainSet = it.key();
            mainSize = it.value();
        }
    }

    const QList<VobPart> vobs = titleSetVobs(videoTsDir, mainSet);
    if (vobs.isEmpty())
        return nullptr;

    auto *device = new DvdTitleDevice(parent);
    device->appendDomainRange(vobs, 0,
                              vobs.last().domainOffset + vobs.last().size);
    return device;
}

bool DvdTitleDevice::open(OpenMode mode)
{
    if (mode != ReadOnly || m_segments.isEmpty())
        return false;

    for (const Segment &segment : m_segments) {
        if (m_files.contains(segment.path))
            continue;
        auto *file = new QFile(segment.path, this);
        if (!file->open(QIODevice::ReadOnly)) {
            close();
            return false;
        }
        m_files.insert(segment.path, file);
    }
    return QIODevice::open(mode);
}

void DvdTitleDevice::close()
{
    for (QFile *file : std::as_const(m_files)) {
        file->close();
        file->deleteLater();
    }
    m_files.clear();
    QIODevice::close();
}

qint64 DvdTitleDevice::readData(char *data, qint64 maxSize)
{
    qint64 position = pos();
    qint64 written = 0;

    while (written < maxSize && position < m_totalSize) {
        const Segment *segment = nullptr;
        for (const Segment &candidate : m_segments) {
            if (position >= candidate.offset
                && position < candidate.offset + candidate.size) {
                segment = &candidate;
                break;
            }
        }
        if (!segment)
            break;
        QFile *file = m_files.value(segment->path);
        if (!file)
            break;

        const qint64 local = position - segment->offset;
        if (!file->seek(segment->fileOffset + local))
            break;
        const qint64 chunk = qMin(maxSize - written, segment->size - local);
        const qint64 got = file->read(data + written, chunk);
        if (got <= 0)
            break;
        written += got;
        position += got;
    }

    return written;
}
