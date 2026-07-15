/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Windows System Media Transport Controls (SMTC) integration: the OS media
    overlay shown on media-key presses, plus the volume-flyout "now playing"
    tile. Attaches to the main window via ISystemMediaTransportControlsInterop
    and drives it from PlayerController. Windows only.
*/

#ifndef WINDOWSSMTC_H
#define WINDOWSSMTC_H

#include <QObject>

#include <memory>

class PlayerController;
class QWindow;

class WindowsSmtc : public QObject
{
    Q_OBJECT

public:
    WindowsSmtc(PlayerController *controller, QWindow *window,
                QObject *parent = nullptr);
    ~WindowsSmtc() override;

private slots:
    void handleButton(int button); // invoked on the Qt thread from the SMTC cb
    void updatePlaybackState();
    void updateMetadata();
    void updateTimeline();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl; // hides the WinRT types from this header
    PlayerController *m_controller = nullptr;
};

#endif // WINDOWSSMTC_H
