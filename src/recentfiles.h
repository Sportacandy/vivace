/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef RECENTFILES_H
#define RECENTFILES_H

#include <QObject>
#include <QSettings>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

/*  Most-recently-played history backing the Open > Recent files menu
    (SMPlayer: Recents in prefs, filled from playback history).
*/
class RecentFiles : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("RecentFiles is owned by PlayerController")
    Q_PROPERTY(QStringList titles READ titles NOTIFY changed)
    Q_PROPERTY(int count READ count NOTIFY changed)
    Q_PROPERTY(int maxItems READ maxItems WRITE setMaxItems NOTIFY maxItemsChanged)

public:
    explicit RecentFiles(QObject *parent = nullptr);

    QStringList titles() const;
    int count() const { return m_entries.size(); }
    int maxItems() const { return m_maxItems; }
    void setMaxItems(int maxItems);

    void add(const QUrl &url, const QString &title);
    Q_INVOKABLE QUrl urlAt(int index) const;
    Q_INVOKABLE void clear();

signals:
    void changed();
    void maxItemsChanged();

private:
    struct Entry {
        QUrl url;
        QString title;
    };

    void save();

    QSettings m_store;
    QList<Entry> m_entries;
    int m_maxItems = 10;
};

#endif // RECENTFILES_H
