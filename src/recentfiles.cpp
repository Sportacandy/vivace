/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "recentfiles.h"

namespace {
constexpr auto urlsKey = "history/recentUrls";
constexpr auto titlesKey = "history/recentTitles";
}

RecentFiles::RecentFiles(QObject *parent)
    : QObject(parent)
{
    const QStringList urls = m_store.value(urlsKey).toStringList();
    const QStringList titles = m_store.value(titlesKey).toStringList();
    for (qsizetype i = 0; i < urls.size(); ++i)
        m_entries.append({ QUrl(urls.at(i)),
                           i < titles.size() ? titles.at(i) : QString() });
}

QStringList RecentFiles::titles() const
{
    QStringList out;
    out.reserve(m_entries.size());
    for (const Entry &entry : m_entries)
        out << (entry.title.isEmpty() ? entry.url.toDisplayString()
                                      : entry.title);
    return out;
}

void RecentFiles::add(const QUrl &url, const QString &title)
{
    if (url.isEmpty())
        return;

    for (qsizetype i = 0; i < m_entries.size(); ++i) {
        if (m_entries.at(i).url == url) {
            m_entries.removeAt(i);
            break;
        }
    }
    m_entries.prepend({ url, title });
    while (m_entries.size() > m_maxItems)
        m_entries.removeLast();

    save();
    emit changed();
}

void RecentFiles::setMaxItems(int maxItems)
{
    maxItems = qBound(1, maxItems, 50);
    if (maxItems == m_maxItems)
        return;
    m_maxItems = maxItems;

    if (m_entries.size() > m_maxItems) {
        while (m_entries.size() > m_maxItems)
            m_entries.removeLast();
        save();
        emit changed();
    }
    emit maxItemsChanged();
}

QUrl RecentFiles::urlAt(int index) const
{
    if (index < 0 || index >= m_entries.size())
        return {};
    return m_entries.at(index).url;
}

void RecentFiles::clear()
{
    if (m_entries.isEmpty())
        return;
    m_entries.clear();
    save();
    emit changed();
}

void RecentFiles::save()
{
    QStringList urls;
    QStringList titles;
    for (const Entry &entry : m_entries) {
        urls << entry.url.toString();
        titles << entry.title;
    }
    m_store.setValue(urlsKey, urls);
    m_store.setValue(titlesKey, titles);
}
