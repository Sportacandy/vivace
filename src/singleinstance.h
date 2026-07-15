/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SINGLEINSTANCE_H
#define SINGLEINSTANCE_H

#include <QObject>
#include <QString>
#include <QStringList>

class QLocalServer;

/*  Single-instance guard (SMPlayer's use-one-instance option): the first
    process listens on a per-user local socket; later processes hand their
    command-line arguments over and exit, so the running window plays them.
*/
class SingleInstance : public QObject
{
    Q_OBJECT

public:
    explicit SingleInstance(const QString &key, QObject *parent = nullptr);

    // Tries to hand `arguments` to an already-running primary instance.
    // Returns true when one accepted them (this process should exit).
    bool sendToRunning(const QStringList &arguments);

    // Becomes the primary instance, listening for later ones.
    void startServer();

signals:
    void messageReceived(const QStringList &arguments);

private:
    QString m_key;
    QLocalServer *m_server = nullptr;
};

#endif // SINGLEINSTANCE_H
