/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    QML-facing dispatcher for OS media integration: Windows System Media
    Transport Controls, or Linux MPRIS2. Instantiated from Main.qml with the
    player controller and the main window; it creates the platform backend
    once both are set.
*/

#ifndef MEDIACONTROLS_H
#define MEDIACONTROLS_H

#include <QObject>
#include <QWindow>
#include <QtQml/qqmlregistration.h>

#include "playercontroller.h"

class MediaControls : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(PlayerController *controller READ controller WRITE setController
                       NOTIFY controllerChanged)
    Q_PROPERTY(QWindow *window READ window WRITE setWindow NOTIFY windowChanged)

public:
    explicit MediaControls(QObject *parent = nullptr);
    ~MediaControls() override;

    PlayerController *controller() const { return m_controller; }
    void setController(PlayerController *controller);
    QWindow *window() const { return m_window; }
    void setWindow(QWindow *window);

signals:
    void controllerChanged();
    void windowChanged();

private:
    void initialize();

    PlayerController *m_controller = nullptr;
    QWindow *m_window = nullptr;
    QObject *m_backend = nullptr;
    bool m_initialized = false;
};

#endif // MEDIACONTROLS_H
