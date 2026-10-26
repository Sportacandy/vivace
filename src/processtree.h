/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <QtGlobal>

#include <memory>

class QProcess;

// Lets a QProcess be killed together with every process it started.
// QProcess::kill() ends only the direct child, so a grandchild (yt-dlp's
// ffmpeg recording a live stream) survives as an orphan.
// Windows: each start() puts the process into a fresh Job Object.
// Unix: each start() puts the process into a new session/process group.
class ProcessTree
{
public:
    enum class OnRelease {
        // Windows: descendants still running when the job is released (the
        // next start(), ~ProcessTree(), or Vivace exiting/crashing) are killed.
        KillDescendants,
        // Leave them running. For a process whose child must outlive it:
        // "yt-dlp -U" on Windows starts a helper that deletes the replaced
        // yt-dlp.exe.old a few seconds after yt-dlp exits.
        LeaveDescendants,
    };

    // Call before the first start(); the setting applies to every later start().
    // Don't start() the process again after this object is destroyed.
    explicit ProcessTree(QProcess *process, OnRelease onRelease = OnRelease::KillDescendants);
    ~ProcessTree();

    ProcessTree(const ProcessTree &) = delete;
    ProcessTree &operator=(const ProcessTree &) = delete;

    // Kills the process and all its descendants. On Windows, also waits up to
    // timeoutMs for them to exit, since their open files can't be deleted
    // until they do. The caller still reaps the QProcess itself
    // (waitForFinished()).
    void killTree(int timeoutMs = 2000);

private:
    QProcess *m_process;
#ifdef Q_OS_WIN
    struct WinState;
    std::unique_ptr<WinState> m_win;
#endif
};
