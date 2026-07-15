/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "mediacontrols.h"

#include "playercontroller.h"

#if defined(Q_OS_WIN)
#include "windowssmtc.h"
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
#include "mpris2.h"
#endif

MediaControls::MediaControls(QObject *parent) : QObject(parent) {}

MediaControls::~MediaControls() = default;

void MediaControls::setController(PlayerController *controller)
{
    if (controller == m_controller)
        return;
    m_controller = controller;
    emit controllerChanged();
    initialize();
}

void MediaControls::setWindow(QWindow *window)
{
    if (window == m_window)
        return;
    m_window = window;
    emit windowChanged();
    initialize();
}

void MediaControls::initialize()
{
    // Both the controller and the native window are needed (the window for the
    // SMTC HWND / MPRIS Raise). Only wire up once.
    if (m_initialized || !m_controller || !m_window)
        return;
    m_initialized = true;

#if defined(Q_OS_WIN)
    m_backend = new WindowsSmtc(m_controller, m_window, this);
#elif defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    m_backend = new Mpris2(m_controller, m_window, this);
#endif
}
