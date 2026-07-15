/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include <QList>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

/*  Per-file bookmarks (SMPlayer's mset.bookmarks): named time positions in
    the current media. Kept in their own ini ("vivace_bookmarks"), keyed by a
    hash of the media key, so they survive the resume store being cleared when
    a file is watched to the end. Reflects one file at a time — PlayerController
    switches the key as media loads.
*/
class Bookmarks : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Bookmarks is owned by PlayerController")
    Q_PROPERTY(int count READ count NOTIFY changed)
    Q_PROPERTY(int revision READ revision NOTIFY changed) // bindings re-read

public:
    explicit Bookmarks(QObject *parent = nullptr);

    int count() const { return int(m_entries.size()); }
    int revision() const { return m_revision; }

    // Switches to the bookmarks of `key` (a media url string or a synthetic
    // disc key), loading them from storage. A no-op when the key is unchanged.
    void setCurrentKey(const QString &key);
    QString currentKey() const { return m_key; }
    bool hasKey() const { return !m_key.isEmpty(); }

    // Rows for the menu / editor: each { time, name, label }, time in ms,
    // sorted by time ascending.
    Q_INVOKABLE QVariantList entries() const;

    // Adds a bookmark at timeMs; a same-time entry has its name replaced.
    Q_INVOKABLE void add(qint64 timeMs, const QString &name = QString());
    Q_INVOKABLE void removeAt(int index);
    Q_INVOKABLE void updateAt(int index, qint64 timeMs, const QString &name);
    Q_INVOKABLE void clear();

    Q_INVOKABLE qint64 timeAt(int index) const;

    static QString formatTime(qint64 ms);

signals:
    void changed();

private:
    struct Entry {
        qint64 timeMs = 0;
        QString name;
    };

    static QString keyFor(const QString &mediaKey);
    void load();
    void save() const;
    void sortEntries();

    mutable QSettings m_store;
    QString m_key;
    QList<Entry> m_entries;
    int m_revision = 0;
};

#endif // BOOKMARKS_H
