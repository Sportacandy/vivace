/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "singleinstance.h"

#include <QLocalServer>
#include <QLocalSocket>

SingleInstance::SingleInstance(const QString &key, QObject *parent)
    : QObject(parent), m_key(key)
{
}

bool SingleInstance::sendToRunning(const QStringList &arguments)
{
    QLocalSocket socket;
    socket.connectToServer(m_key);
    if (!socket.waitForConnected(300))
        return false; // no primary instance listening

    // Newline-framed UTF-8; the server parses on disconnect.
    socket.write(arguments.join(QLatin1Char('\n')).toUtf8());
    socket.flush();
    socket.waitForBytesWritten(300);
    socket.disconnectFromServer();
    if (socket.state() != QLocalSocket::UnconnectedState)
        socket.waitForDisconnected(300);
    return true;
}

void SingleInstance::startServer()
{
    QLocalServer::removeServer(m_key); // clear a stale socket from a crash
    m_server = new QLocalServer(this);
    m_server->listen(m_key);

    connect(m_server, &QLocalServer::newConnection, this, [this]() {
        QLocalSocket *client = m_server->nextPendingConnection();
        if (!client)
            return;
        auto *buffer = new QByteArray;
        connect(client, &QLocalSocket::readyRead, client,
                [client, buffer]() { buffer->append(client->readAll()); });
        connect(client, &QLocalSocket::disconnected, client,
                [this, client, buffer]() {
                    buffer->append(client->readAll());
                    const QStringList args =
                            QString::fromUtf8(*buffer)
                                    .split(QLatin1Char('\n'), Qt::SkipEmptyParts);
                    emit messageReceived(args);
                    delete buffer;
                    client->deleteLater();
                });
    });
}
