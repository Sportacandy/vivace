/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "bookmarks.h"

#include <QCryptographicHash>
#include <QVariantMap>

#include <algorithm>

Bookmarks::Bookmarks(QObject *parent)
    : QObject(parent),
      m_store(QSettings::IniFormat, QSettings::UserScope,
              QStringLiteral("vivace-player"), QStringLiteral("vivace_bookmarks"))
{
}

QString Bookmarks::keyFor(const QString &mediaKey)
{
    const QByteArray hash = QCryptographicHash::hash(
            mediaKey.toUtf8(), QCryptographicHash::Sha1);
    return QStringLiteral("bookmarks/") + QString::fromLatin1(hash.toHex());
}

QString Bookmarks::formatTime(qint64 ms)
{
    const qint64 totalSec = ms / 1000;
    return QStringLiteral("%1:%2:%3")
            .arg(totalSec / 3600, 2, 10, u'0')
            .arg((totalSec % 3600) / 60, 2, 10, u'0')
            .arg(totalSec % 60, 2, 10, u'0');
}

void Bookmarks::setCurrentKey(const QString &key)
{
    if (key == m_key)
        return;
    m_key = key;
    load();
    emit changed();
}

void Bookmarks::load()
{
    m_entries.clear();
    if (m_key.isEmpty())
        return;

    const QString group = keyFor(m_key);
    const int size = m_store.beginReadArray(group + QStringLiteral("/list"));
    for (int i = 0; i < size; ++i) {
        m_store.setArrayIndex(i);
        Entry e;
        e.timeMs = m_store.value(QStringLiteral("time")).toLongLong();
        e.name = m_store.value(QStringLiteral("name")).toString();
        m_entries.append(e);
    }
    m_store.endArray();
    sortEntries();
}

void Bookmarks::save() const
{
    if (m_key.isEmpty())
        return;

    const QString group = keyFor(m_key);
    m_store.remove(group);
    if (m_entries.isEmpty())
        return;

    m_store.setValue(group + QStringLiteral("/key"), m_key);
    m_store.beginWriteArray(group + QStringLiteral("/list"), int(m_entries.size()));
    for (int i = 0; i < m_entries.size(); ++i) {
        m_store.setArrayIndex(i);
        m_store.setValue(QStringLiteral("time"), m_entries.at(i).timeMs);
        m_store.setValue(QStringLiteral("name"), m_entries.at(i).name);
    }
    m_store.endArray();
}

void Bookmarks::sortEntries()
{
    std::sort(m_entries.begin(), m_entries.end(),
              [](const Entry &a, const Entry &b) { return a.timeMs < b.timeMs; });
}

QVariantList Bookmarks::entries() const
{
    QVariantList rows;
    rows.reserve(m_entries.size());
    for (const Entry &e : m_entries) {
        const QString time = formatTime(e.timeMs);
        const QString label = e.name.isEmpty()
                ? time
                : QStringLiteral("%1 (%2)").arg(e.name, time);
        rows << QVariantMap {
            { QStringLiteral("time"), e.timeMs },
            { QStringLiteral("name"), e.name },
            { QStringLiteral("label"), label },
        };
    }
    return rows;
}

void Bookmarks::add(qint64 timeMs, const QString &name)
{
    if (m_key.isEmpty())
        return;

    // Re-bookmarking the exact same instant updates the name in place.
    for (Entry &e : m_entries) {
        if (e.timeMs == timeMs) {
            e.name = name;
            save();
            emit changed();
            return;
        }
    }
    m_entries.append({ timeMs, name });
    sortEntries();
    save();
    emit changed();
}

void Bookmarks::removeAt(int index)
{
    if (index < 0 || index >= m_entries.size())
        return;
    m_entries.removeAt(index);
    save();
    emit changed();
}

void Bookmarks::updateAt(int index, qint64 timeMs, const QString &name)
{
    if (index < 0 || index >= m_entries.size())
        return;
    m_entries[index] = { timeMs, name };
    sortEntries();
    save();
    emit changed();
}

void Bookmarks::clear()
{
    if (m_entries.isEmpty())
        return;
    m_entries.clear();
    save();
    emit changed();
}

qint64 Bookmarks::timeAt(int index) const
{
    if (index < 0 || index >= m_entries.size())
        return -1;
    return m_entries.at(index).timeMs;
}
