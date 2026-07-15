/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Screensaver / display-sleep inhibitor (SMPlayer's disable_screensaver).
    Qt has no cross-platform API for this, so each platform is handled
    natively: Windows uses SetThreadExecutionState, Linux the freedesktop
    ScreenSaver DBus service, macOS an IOKit power-management assertion.
*/

#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <QObject>

#ifdef Q_OS_MACOS
#include <cstdint>
#endif

class QDBusInterface;

class ScreenSaver : public QObject
{
    Q_OBJECT

public:
    explicit ScreenSaver(QObject *parent = nullptr);
    ~ScreenSaver() override;

    // Idempotent: enables/restores the screensaver when inhibited is false,
    // suspends it while true.
    void setInhibited(bool inhibited);
    bool inhibited() const { return m_inhibited; }

private:
    void doInhibit();
    void doUninhibit();

    bool m_inhibited = false;

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    QDBusInterface *m_interface = nullptr;
    unsigned int m_cookie = 0;
    bool m_haveCookie = false;
#elif defined(Q_OS_MACOS)
    uint32_t m_assertion = 0; // IOPMAssertionID
    bool m_haveAssertion = false;
#endif
};

#endif // SCREENSAVER_H
