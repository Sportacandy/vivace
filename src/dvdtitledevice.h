/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef DVDTITLEDEVICE_H
#define DVDTITLEDEVICE_H

#include <QFile>
#include <QHash>
#include <QIODevice>
#include <QList>

#include "dvdifoparser.h"

/*  Random-access QIODevice over a DVD-Video title: the title's cells
    (sector ranges in the VTS_xx_1..9 VOB domain, from the IFO structure)
    concatenated in playback order. The result is an MPEG program stream
    that QMediaPlayer's FFmpeg backend probes and plays directly.

    createFromCells() follows the parsed IFO structure; create() is the
    IFO-less fallback that plays the largest title set's VOB chain whole.
    No menus, no CSS decryption — encrypted discs need the libdvdread/
    libdvdcss integration planned as the follow-up.
*/
class DvdTitleDevice : public QIODevice
{
    Q_OBJECT

public:
    // nullptr when the folder holds no usable title set.
    static DvdTitleDevice *create(const QString &videoTsDir,
                                  QObject *parent = nullptr);
    static DvdTitleDevice *createFromCells(const QString &videoTsDir,
                                           int vtsNumber,
                                           const QList<DvdIfo::Cell> &cells,
                                           QObject *parent = nullptr);
    // Serves menu-domain cells from a single menu VOB (VIDEO_TS.VOB for the
    // VMGM domain, VTS_xx_0.VOB for a VTSM domain); cell sectors are relative
    // to the start of that file.
    static DvdTitleDevice *createFromMenuCells(const QString &vobPath,
                                               const QList<DvdIfo::Cell> &cells,
                                               QObject *parent = nullptr);

    bool open(OpenMode mode) override;
    void close() override;
    qint64 size() const override { return m_totalSize; }
    bool isSequential() const override { return false; }

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    explicit DvdTitleDevice(QObject *parent = nullptr);

    struct VobPart {
        QString path;
        qint64 size = 0;
        qint64 domainOffset = 0; // within the concatenated VOB domain
    };
    struct Segment {
        QString path;
        qint64 fileOffset = 0; // where this segment starts in its file
        qint64 size = 0;
        qint64 offset = 0; // within this device's stream
    };

    static QList<VobPart> titleSetVobs(const QString &videoTsDir,
                                       int vtsNumber);
    void appendDomainRange(const QList<VobPart> &vobs, qint64 domainStart,
                           qint64 domainEnd);

    QList<Segment> m_segments;
    QHash<QString, QFile *> m_files;
    qint64 m_totalSize = 0;
};

#endif // DVDTITLEDEVICE_H
