/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "windowplacementguard.h"

#include <limits>

#include <QEvent>
#include <QGuiApplication>
#include <QPointer>
#include <QScreen>
#include <QTimer>
#include <QWindow>

WindowPlacementGuard::WindowPlacementGuard(QObject *parent)
    : QObject(parent)
{
}

bool WindowPlacementGuard::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Show) {
        if (auto *window = qobject_cast<QWindow *>(watched); window && window->isTopLevel()) {
            // Deferred: the native frame margins (title bar height) are only
            // reliable once the platform window has actually been shown.
            QPointer<QWindow> guarded(window);
            QTimer::singleShot(0, this, [guarded]() {
                if (guarded)
                    keepOnScreen(guarded);
            });
        }
    }
    return QObject::eventFilter(watched, event);
}

void WindowPlacementGuard::keepOnScreen(QWindow *window)
{
    if (!window->isVisible() || window->visibility() != QWindow::Windowed)
        return;
    // Menus, tooltips and other popups place themselves.
    switch (window->type()) {
    case Qt::Popup:
    case Qt::ToolTip:
    case Qt::SplashScreen:
    case Qt::Desktop:
    case Qt::SubWindow:
        return;
    default:
        break;
    }
    const QMargins frame = window->frameMargins();
    const QRect frameRect = window->frameGeometry();

    // All rectangles are in virtual-desktop coordinates, which are negative
    // for screens left of or above the primary one. A window may legitimately
    // span several screens, so only act when no screen shows a grabbable part
    // of the title bar: its full height and at least kMinVisibleWidth pixels.
    // window->screen() isn't used to decide this -- right after show() it can
    // still be the screen the window was created on.
    constexpr int kMinVisibleWidth = 100;
    const QRect titleBar(frameRect.left(), frameRect.top(), frameRect.width(),
                         qMax(frame.top(), 1));
    const QList<QScreen *> screens = QGuiApplication::screens();
    for (QScreen *s : screens) {
        const QRect visible = s->availableGeometry().intersected(titleBar);
        if (visible.height() == titleBar.height()
            && visible.width() >= qMin(kMinVisibleWidth, titleBar.width()))
            return;
    }

    // Relocate onto the screen under the window's center, or the nearest one.
    QScreen *screen = QGuiApplication::screenAt(frameRect.center());
    if (!screen) {
        qint64 best = std::numeric_limits<qint64>::max();
        for (QScreen *s : screens) {
            const QPoint d = s->availableGeometry().center() - frameRect.center();
            const qint64 dist = qint64(d.x()) * d.x() + qint64(d.y()) * d.y();
            if (dist < best) {
                best = dist;
                screen = s;
            }
        }
    }
    if (!screen)
        return;
    const QRect area = screen->availableGeometry();

    // Shrink to fit (never below the window's own minimum size).
    const QSize fitted = window->size()
                             .boundedTo(area.size().shrunkBy(frame))
                             .expandedTo(window->minimumSize());
    if (fitted != window->size())
        window->resize(fitted);

    // Then move the whole frame inside the area. Clamping the top edge last
    // keeps the title bar reachable even if the frame is taller than the area.
    const QSize frameSize = fitted.grownBy(frame);
    const QPoint framePos = window->framePosition();
    int x = qMin(framePos.x(), area.right() + 1 - frameSize.width());
    int y = qMin(framePos.y(), area.bottom() + 1 - frameSize.height());
    x = qMax(x, area.left());
    y = qMax(y, area.top());
    if (QPoint(x, y) != framePos)
        window->setFramePosition(QPoint(x, y));
}
