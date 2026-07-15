/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef LIVESTREAMDEVICE_H
#define LIVESTREAMDEVICE_H

#include <QByteArray>
#include <QIODevice>
#include <QMutex>
#include <QWaitCondition>

/*  A bounded, growing in-memory QIODevice fed incrementally by a producer (an
    HTTP reader, or a file for testing) and consumed by QMediaPlayer via
    setSourceDevice(). It blocks reads until data arrives, so the FFmpeg backend
    can analyse a live stream whose data starts slowly (e.g. a TV tuner that
    pads the connection until the channel locks). The producer strips any
    leading junk before appending, so the backend sees valid stream data from
    byte 0.

    Memory is bounded: already-read history is trimmed (keeping a small back
    margin so early backend seeks and short rewinds still work), and the
    producer throttles feeding via bufferedAhead() so a paused stream can't grow
    the unread buffer without limit. Positions stay absolute across trims;
    seeking into trimmed-away history fails.

    appendData()/finish()/abort() are thread-safe; readData() blocks on the
    backend's demuxer thread until data is available, the stream finishes, or
    the device is aborted. */
class LiveStreamDevice : public QIODevice
{
    Q_OBJECT
public:
    explicit LiveStreamDevice(QObject *parent = nullptr);

    bool isSequential() const override { return false; }
    qint64 size() const override;
    bool seek(qint64 pos) override;
    qint64 bytesAvailable() const override;
    bool atEnd() const override;

    // Bytes received but not yet read — the producer uses this to throttle.
    qint64 bufferedAhead() const;

    // Producer side (any thread):
    void appendData(const QByteArray &data);
    void finish(); // no more data will arrive (normal end)
    void abort();  // stop waiting readers (teardown)

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *, qint64) override { return -1; }

private:
    void trimConsumed(); // drop read history beyond the back margin (locked)

    mutable QMutex m_mutex;
    QWaitCondition m_cond;
    QByteArray m_buffer;      // holds absolute [m_baseOffset, m_baseOffset+size)
    qint64 m_baseOffset = 0;  // bytes trimmed from the front of the stream
    qint64 m_pos = 0;         // absolute read position
    bool m_finished = false;
    bool m_aborted = false;
};

#endif // LIVESTREAMDEVICE_H
