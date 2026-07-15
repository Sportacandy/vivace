/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractListModel>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

struct PlaylistEntry
{
    QUrl url;
    QString title;          // empty → derived from the url on insertion
    qint64 durationMs = -1; // -1 = unknown (from #EXTINF or learned on play)
};

class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("PlaylistModel is owned by PlayerController")
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(bool displayTitleName READ displayTitleName
               WRITE setDisplayTitleName NOTIFY displayTitleNameChanged)
    Q_PROPERTY(bool autoSort READ autoSort WRITE setAutoSort NOTIFY autoSortChanged)

public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        UrlRole,
        DurationRole
    };

    explicit PlaylistModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_entries.size(); }
    int currentIndex() const { return m_currentIndex; }
    void setCurrentIndex(int index);

    bool displayTitleName() const { return m_displayTitleName; }
    void setDisplayTitleName(bool display);
    bool autoSort() const { return m_autoSort; }
    void setAutoSort(bool autoSort);

    QUrl urlAt(int index) const;
    int nextIndex() const;
    int previousIndex() const;

    // Records a learned duration for an entry (e.g. from the player on load).
    void setDuration(int index, qint64 ms);

    void add(const QList<PlaylistEntry> &entries);
    QList<PlaylistEntry> entries() const { return m_entries; }
    Q_INVOKABLE void removeAt(int index);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void move(int from, int to);

signals:
    void countChanged();
    void currentIndexChanged();
    void displayTitleNameChanged();
    void autoSortChanged();

private:
    void sortEntries(); // stable sort by displayed name, keeps current entry
    int indexOfUrl(const QUrl &url) const;

    QList<PlaylistEntry> m_entries;
    int m_currentIndex = -1;
    bool m_displayTitleName = true;
    bool m_autoSort = false;
};

#endif // PLAYLISTMODEL_H
