/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Play > Cast > Smartphone/tablet: an embedded HTTP server (QTcpServer, no
    external process and no Chromecast/DLNA/UPnP protocol — unlike SMPlayer's
    Chromecast/mobile casting, which shells out to a bundled Mongoose-based
    web server or webfsd) that serves the currently playing local file to any
    browser on the LAN, so a phone or tablet can open a plain URL and play it
    with its own <video> element. Local files only: DVD/device sources and
    network streams are not served (see canServeCurrentSource()).
*/

#ifndef CASTSERVER_H
#define CASTSERVER_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

#include "playercontroller.h"

class QTcpServer;
class QTcpSocket;

class CastServer : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(PlayerController *controller READ controller WRITE setController
                       NOTIFY controllerChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QStringList urls READ urls NOTIFY runningChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit CastServer(QObject *parent = nullptr);

    PlayerController *controller() const { return m_controller; }
    void setController(PlayerController *controller);

    bool running() const;
    int port() const { return m_port; }
    void setPort(int port);
    QStringList urls() const;
    QString lastError() const { return m_lastError; }

    Q_INVOKABLE bool start();
    Q_INVOKABLE void stop();

    // Renders `text` (a cast URL) as a QR code, SMPlayer-style (same
    // underlying Nayuki qrcodegen library SMPlayer bundles for its own
    // Cast > Smartphone/tablet dialog), returned as a data: URL an Image
    // element can use directly as its source.
    Q_INVOKABLE QString qrCodeDataUrl(const QString &text) const;

signals:
    void controllerChanged();
    void runningChanged();
    void portChanged();
    void lastErrorChanged();

private slots:
    void handleNewConnection();
    void handleReadyRead();
    void handleDisconnected();

private:
    void dispatch(QTcpSocket *socket, const QByteArray &head);
    void serveIndex(QTcpSocket *socket);
    void serveStream(QTcpSocket *socket, const QByteArray &rangeHeader, bool headOnly);
    void serveSimple(QTcpSocket *socket, int status, const QByteArray &statusText,
                      const QByteArray &body);
    bool canServeCurrentSource() const;

    QTcpServer *m_server;
    QPointer<PlayerController> m_controller;
    int m_port = 47990;
    QString m_lastError;
    QHash<QTcpSocket *, QByteArray> m_pending; // buffered bytes until headers complete
};

#endif // CASTSERVER_H
