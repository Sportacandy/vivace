/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Central keyboard-shortcut registry (SMPlayer's actionseditor concept, QML
    edition). Every user-editable shortcut has a stable id, a default sequence
    and a human label in a fixed catalog; user overrides are persisted in
    QSettings. The QML menu Actions / Shortcuts bind their sequence to
    Shortcuts.sequences[id], so edits apply live. Exposed as a QML singleton.
*/

#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include <QMap>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

class Shortcuts : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // id -> effective sequence (default, overridden by the user's choice).
    Q_PROPERTY(QVariantMap sequences READ sequences NOTIFY changed)

public:
    explicit Shortcuts(QObject *parent = nullptr);

    QVariantMap sequences() const;

    Q_INVOKABLE QString sequence(const QString &id) const;
    Q_INVOKABLE QString defaultSequence(const QString &id) const;
    Q_INVOKABLE QString label(const QString &id) const;
    Q_INVOKABLE bool isCustom(const QString &id) const;

    Q_INVOKABLE void setSequence(const QString &id, const QString &seq);
    Q_INVOKABLE void reset(const QString &id);
    Q_INVOKABLE void resetAll();

    // id of the action already bound to `seq` (excluding excludeId), else "".
    Q_INVOKABLE QString conflict(const QString &seq, const QString &excludeId) const;

    // Ordered rows for the editor: {id, section, label, sequence, custom}.
    Q_INVOKABLE QVariantList actionList() const;

    // Portable-text sequence from a QML key event (empty for a lone modifier).
    Q_INVOKABLE QString sequenceForKey(int key, int modifiers) const;

signals:
    void changed();

private:
    struct Def {
        QString id;
        QString section;
        QString label;
        QString def;
    };
    static const QList<Def> &catalog();
    static const Def *find(const QString &id);
    static QString normalize(const QString &seq);

    QMap<QString, QString> m_custom; // id -> user override
    QSettings m_store;
};

#endif // SHORTCUTS_H
