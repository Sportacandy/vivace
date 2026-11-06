/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef WINDOWPLACEMENTGUARD_H
#define WINDOWPLACEMENTGUARD_H

#include <QObject>

class QWindow;

/*  Keeps every top-level window reachable: when one is shown and no screen
    shows a grabbable part of its title bar, it is moved onto the screen under
    its center (or the nearest one), shrunk to fit that screen's available
    area (excluding the taskbar), and placed fully inside it. Windows whose
    title bar is reachable are left alone, including ones that span screens.

    Without this, a small logical screen (e.g. 1920x1080 at 150% scaling =
    1280x672 usable) left the 1280x760 main window with its client area at
    (0,0) and its title bar above the screen; Vivace then remembered that
    position, and dialogs centered on the main window (Preferences, Help)
    landed with their title bars off-screen too -- no way to drag, minimize,
    maximize or close them with the mouse.

    Installed as an application-wide event filter, so it covers the main
    window, every dialog and the separate playlist window without each of
    them having to clamp its own placement.
*/
class WindowPlacementGuard : public QObject
{
    Q_OBJECT

public:
    explicit WindowPlacementGuard(QObject *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    static void keepOnScreen(QWindow *window);
};

#endif // WINDOWPLACEMENTGUARD_H
