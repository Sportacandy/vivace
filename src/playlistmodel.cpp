/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "playlistmodel.h"

#include <algorithm>

namespace {

QString titleForUrl(const QUrl &url)
{
    const QString name = url.fileName();
    return name.isEmpty() ? url.toDisplayString() : name;
}

PlaylistEntry normalized(const PlaylistEntry &e)
{
    return { e.url, e.title.isEmpty() ? titleForUrl(e.url) : e.title,
             e.durationMs };
}

QString formatDuration(qint64 ms)
{
    if (ms < 0)
        return {};
    const qint64 s = ms / 1000;
    if (s >= 3600)
        return QStringLiteral("%1:%2:%3").arg(s / 3600)
                .arg((s % 3600) / 60, 2, 10, u'0').arg(s % 60, 2, 10, u'0');
    return QStringLiteral("%1:%2").arg(s / 60).arg(s % 60, 2, 10, u'0');
}

} // namespace

PlaylistModel::PlaylistModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};

    const PlaylistEntry &entry = m_entries.at(index.row());
    switch (role) {
    case TitleRole:
        // "Display title name instead of filename": the stored title (which
        // may be a real name from a playlist) vs. the plain file name.
        return m_displayTitleName ? entry.title : titleForUrl(entry.url);
    case UrlRole:
        return entry.url;
    case DurationRole:
        return formatDuration(entry.durationMs);
    default:
        return {};
    }
}

QHash<int, QByteArray> PlaylistModel::roleNames() const
{
    return {
        { TitleRole, QByteArrayLiteral("title") },
        { UrlRole, QByteArrayLiteral("url") },
        { DurationRole, QByteArrayLiteral("duration") }
    };
}

void PlaylistModel::setCurrentIndex(int index)
{
    if (index < -1 || index >= m_entries.size())
        index = -1;
    if (index == m_currentIndex)
        return;
    m_currentIndex = index;
    emit currentIndexChanged();
}

QUrl PlaylistModel::urlAt(int index) const
{
    if (index < 0 || index >= m_entries.size())
        return {};
    return m_entries.at(index).url;
}

int PlaylistModel::nextIndex() const
{
    if (m_currentIndex < 0 || m_currentIndex + 1 >= m_entries.size())
        return -1;
    return m_currentIndex + 1;
}

int PlaylistModel::previousIndex() const
{
    if (m_currentIndex <= 0 || m_entries.isEmpty())
        return -1;
    return m_currentIndex - 1;
}

void PlaylistModel::setDuration(int index, qint64 ms)
{
    if (index < 0 || index >= m_entries.size() || ms < 0)
        return;
    if (m_entries.at(index).durationMs == ms)
        return;
    m_entries[index].durationMs = ms;
    emit dataChanged(this->index(index), this->index(index), { DurationRole });
}

void PlaylistModel::add(const QList<PlaylistEntry> &entries)
{
    if (entries.isEmpty())
        return;

    // Auto sort reorders the whole list, so reset rather than insert-append.
    if (m_autoSort) {
        const QUrl current = (m_currentIndex >= 0
                              && m_currentIndex < m_entries.size())
                ? m_entries.at(m_currentIndex).url : QUrl();
        beginResetModel();
        for (const PlaylistEntry &entry : entries)
            m_entries.append(normalized(entry));
        sortEntries();
        endResetModel();
        emit countChanged();

        const int idx = indexOfUrl(current);
        if (idx != m_currentIndex) {
            m_currentIndex = idx;
            emit currentIndexChanged();
        }
        return;
    }

    beginInsertRows(QModelIndex(), m_entries.size(),
                    m_entries.size() + entries.size() - 1);
    for (const PlaylistEntry &entry : entries)
        m_entries.append(normalized(entry));
    endInsertRows();
    emit countChanged();
}

void PlaylistModel::removeAt(int index)
{
    if (index < 0 || index >= m_entries.size())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_entries.removeAt(index);
    endRemoveRows();
    emit countChanged();

    // Keep the playing entry marked; drop the mark if it was the one removed.
    if (index == m_currentIndex) {
        m_currentIndex = -1;
        emit currentIndexChanged();
    } else if (index < m_currentIndex) {
        --m_currentIndex;
        emit currentIndexChanged();
    }
}

void PlaylistModel::move(int from, int to)
{
    if (from < 0 || from >= m_entries.size() || to < 0
        || to >= m_entries.size() || from == to) {
        return;
    }

    // beginMoveRows takes the destination in pre-move coordinates.
    if (!beginMoveRows(QModelIndex(), from, from, QModelIndex(),
                       to > from ? to + 1 : to)) {
        return;
    }
    m_entries.move(from, to);

    const int oldCurrent = m_currentIndex;
    if (m_currentIndex == from)
        m_currentIndex = to;
    else if (from < m_currentIndex && to >= m_currentIndex)
        --m_currentIndex;
    else if (from > m_currentIndex && to <= m_currentIndex)
        ++m_currentIndex;
    endMoveRows();

    if (m_currentIndex != oldCurrent)
        emit currentIndexChanged();
}

void PlaylistModel::clear()
{
    if (m_entries.isEmpty())
        return;

    beginResetModel();
    m_entries.clear();
    endResetModel();
    emit countChanged();

    if (m_currentIndex != -1) {
        m_currentIndex = -1;
        emit currentIndexChanged();
    }
}

void PlaylistModel::setDisplayTitleName(bool display)
{
    if (display == m_displayTitleName)
        return;
    m_displayTitleName = display;
    emit displayTitleNameChanged();
    if (!m_entries.isEmpty())
        emit dataChanged(index(0), index(m_entries.size() - 1), { TitleRole });
}

void PlaylistModel::setAutoSort(bool autoSort)
{
    if (autoSort == m_autoSort)
        return;
    m_autoSort = autoSort;
    emit autoSortChanged();
    if (m_autoSort && m_entries.size() > 1) {
        const QUrl current = (m_currentIndex >= 0
                              && m_currentIndex < m_entries.size())
                ? m_entries.at(m_currentIndex).url : QUrl();
        beginResetModel();
        sortEntries();
        endResetModel();
        const int idx = indexOfUrl(current);
        if (idx != m_currentIndex) {
            m_currentIndex = idx;
            emit currentIndexChanged();
        }
    }
}

void PlaylistModel::sortEntries()
{
    std::stable_sort(m_entries.begin(), m_entries.end(),
                     [](const PlaylistEntry &a, const PlaylistEntry &b) {
                         return QString::localeAwareCompare(a.title, b.title) < 0;
                     });
}

int PlaylistModel::indexOfUrl(const QUrl &url) const
{
    if (url.isEmpty())
        return -1;
    for (qsizetype i = 0; i < m_entries.size(); ++i)
        if (m_entries.at(i).url == url)
            return int(i);
    return -1;
}
