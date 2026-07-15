/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef FAVORITESMODEL_H
#define FAVORITESMODEL_H

#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

/*  A tree of favorites (SMPlayer's favorites with submenus). Items and
    submenus at any level are addressed by a path string: "" is the root,
    "2" is child 2, "2/0" the first child of that submenu. Persisted as
    JSON in the configuration folder.
*/
class FavoritesModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("FavoritesModel is owned by PlayerController")
    Q_PROPERTY(int revision READ revision NOTIFY changed) // bindings re-read on change

public:
    explicit FavoritesModel(const QString &filePath, QObject *parent = nullptr);

    int revision() const { return m_revision; }

    // Rows at `path`: each { name, url, isSubmenu } (url empty for submenus).
    Q_INVOKABLE QVariantList items(const QString &path) const;
    Q_INVOKABLE QUrl urlAt(const QString &path, int index) const;
    Q_INVOKABLE bool isSubmenu(const QString &path, int index) const;

    Q_INVOKABLE void addItem(const QString &path, const QString &name,
                             const QString &url);
    Q_INVOKABLE void addSubmenu(const QString &path, const QString &name);
    Q_INVOKABLE void updateItem(const QString &path, int index,
                                const QString &name, const QString &url);
    Q_INVOKABLE void removeAt(const QString &path, int index);
    Q_INVOKABLE void clearAt(const QString &path);
    Q_INVOKABLE void move(const QString &path, int from, int to);

    // Convenience for "Add current media" (top level).
    void addUrl(const QString &name, const QString &url);

    struct Node {
        QString name;
        QString url; // empty => submenu
        bool submenu = false;
        QList<Node> children;
    };

signals:
    void changed();

private:
    Node *nodeAt(const QString &path); // the submenu addressed by path
    void load();
    void save() const;

    QString m_filePath;
    Node m_root;
    int m_revision = 0;
};

#endif // FAVORITESMODEL_H
