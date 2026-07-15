/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "livestreamdevice.h"

#include <cstring>

namespace {
// Keep this much already-read history so the backend's early seeks (e.g.
// find_stream_info re-reading from the start) and short rewinds still work...
constexpr qint64 kBackMargin = 4 * 1024 * 1024;
// ...but only actually memmove the buffer once the dead history exceeds the
// margin by this much, to amortise the cost of trimming.
constexpr qint64 kTrimBlock = 8 * 1024 * 1024;
} // namespace

LiveStreamDevice::LiveStreamDevice(QObject *parent) : QIODevice(parent) {}

qint64 LiveStreamDevice::size() const
{
    QMutexLocker lock(&m_mutex);
    return m_baseOffset + m_buffer.size();
}

bool LiveStreamDevice::seek(qint64 pos)
{
    QMutexLocker lock(&m_mutex);
    if (pos < m_baseOffset || pos > m_baseOffset + m_buffer.size())
        return false; // outside the retained window
    m_pos = pos;
    QIODevice::seek(pos);
    return true;
}

qint64 LiveStreamDevice::bufferedAhead() const
{
    QMutexLocker lock(&m_mutex);
    return (m_baseOffset + m_buffer.size()) - m_pos;
}

qint64 LiveStreamDevice::bytesAvailable() const
{
    QMutexLocker lock(&m_mutex);
    return ((m_baseOffset + m_buffer.size()) - m_pos) + QIODevice::bytesAvailable();
}

bool LiveStreamDevice::atEnd() const
{
    QMutexLocker lock(&m_mutex);
    return (m_finished || m_aborted) && m_pos >= m_baseOffset + m_buffer.size();
}

void LiveStreamDevice::appendData(const QByteArray &data)
{
    if (data.isEmpty())
        return;
    QMutexLocker lock(&m_mutex);
    m_buffer.append(data);
    m_cond.wakeAll();
}

void LiveStreamDevice::finish()
{
    QMutexLocker lock(&m_mutex);
    m_finished = true;
    m_cond.wakeAll();
}

void LiveStreamDevice::abort()
{
    QMutexLocker lock(&m_mutex);
    m_aborted = true;
    m_cond.wakeAll();
}

void LiveStreamDevice::trimConsumed()
{
    // Caller holds m_mutex. Drop history well behind the read cursor.
    const qint64 consumedInBuffer = m_pos - m_baseOffset;
    if (consumedInBuffer <= kBackMargin + kTrimBlock)
        return;
    const qint64 drop = consumedInBuffer - kBackMargin;
    m_buffer.remove(0, drop);
    m_baseOffset += drop;
}

qint64 LiveStreamDevice::readData(char *data, qint64 maxlen)
{
    QMutexLocker lock(&m_mutex);
    // Block until data is available, the stream ends, or we're torn down.
    // Runs on the backend's demuxer thread, so blocking here is fine.
    while (m_pos >= m_baseOffset + m_buffer.size() && !m_finished && !m_aborted)
        m_cond.wait(&m_mutex);
    if (m_pos >= m_baseOffset + m_buffer.size())
        return 0; // EOF (finished/aborted with nothing left)
    const qint64 start = m_pos - m_baseOffset;
    const qint64 n = qMin(maxlen, static_cast<qint64>(m_buffer.size()) - start);
    std::memcpy(data, m_buffer.constData() + start, n);
    m_pos += n;
    trimConsumed();
    return n;
}
