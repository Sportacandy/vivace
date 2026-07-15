/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "screensaver.h"

#include <QtGlobal>

#if defined(Q_OS_WIN)
#include <windows.h>
#elif defined(Q_OS_MACOS)
#include <IOKit/pwr_mgt/IOPMLib.h>
#include <CoreFoundation/CoreFoundation.h>
#elif defined(Q_OS_UNIX)
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#endif

ScreenSaver::ScreenSaver(QObject *parent)
    : QObject(parent)
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    m_interface = new QDBusInterface(
        QStringLiteral("org.freedesktop.ScreenSaver"),
        QStringLiteral("/org/freedesktop/ScreenSaver"),
        QStringLiteral("org.freedesktop.ScreenSaver"),
        QDBusConnection::sessionBus(), this);
#endif
}

ScreenSaver::~ScreenSaver()
{
    // Always restore the screensaver on shutdown.
    setInhibited(false);
}

void ScreenSaver::setInhibited(bool inhibited)
{
    if (inhibited == m_inhibited)
        return;
    m_inhibited = inhibited;
    if (inhibited)
        doInhibit();
    else
        doUninhibit();
}

#if defined(Q_OS_WIN)

void ScreenSaver::doInhibit()
{
    // ES_CONTINUOUS keeps the request in effect until reset; the display and
    // system stay awake while media plays.
    SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED
                            | ES_SYSTEM_REQUIRED);
}

void ScreenSaver::doUninhibit()
{
    SetThreadExecutionState(ES_CONTINUOUS);
}

#elif defined(Q_OS_MACOS)

void ScreenSaver::doInhibit()
{
    IOPMAssertionID assertion = 0;
    const CFStringRef reason = CFSTR("Vivace is playing media");
    if (IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep,
                                    kIOPMAssertionLevelOn, reason, &assertion)
        == kIOReturnSuccess) {
        m_assertion = assertion;
        m_haveAssertion = true;
    }
}

void ScreenSaver::doUninhibit()
{
    if (m_haveAssertion) {
        IOPMAssertionRelease(m_assertion);
        m_haveAssertion = false;
    }
}

#elif defined(Q_OS_UNIX)

void ScreenSaver::doInhibit()
{
    if (!m_interface || !m_interface->isValid())
        return;
    QDBusReply<unsigned int> reply =
        m_interface->call(QStringLiteral("Inhibit"),
                          QStringLiteral("Vivace"),
                          QStringLiteral("Playing media"));
    if (reply.isValid()) {
        m_cookie = reply.value();
        m_haveCookie = true;
    }
}

void ScreenSaver::doUninhibit()
{
    if (m_interface && m_haveCookie) {
        m_interface->call(QStringLiteral("UnInhibit"), m_cookie);
        m_haveCookie = false;
    }
}

#else

void ScreenSaver::doInhibit() {}
void ScreenSaver::doUninhibit() {}

#endif
