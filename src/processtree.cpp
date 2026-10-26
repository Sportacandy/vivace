/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "processtree.h"

#include <QProcess>

#ifdef Q_OS_WIN
#  include <QElapsedTimer>
#  include <qt_windows.h>
#  include <vector>
#else
#  include <signal.h>
#endif

#ifdef Q_OS_WIN

struct ProcessTree::WinState
{
    HANDLE job = nullptr;
    bool killOnRelease = true;
    // CreateProcess reads these during start(), after the modifier returns.
    std::vector<char> attributeBuffer;
    STARTUPINFOEXW startupInfo{};
    bool attributeListInitialized = false;

    void reset()
    {
        if (attributeListInitialized) {
            DeleteProcThreadAttributeList(
                    reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributeBuffer.data()));
            attributeListInitialized = false;
        }
        if (job) {
            // With KILL_ON_JOB_CLOSE, anything still running from the
            // previous start() dies here.
            CloseHandle(job);
            job = nullptr;
        }
    }
};

ProcessTree::ProcessTree(QProcess *process, OnRelease onRelease)
    : m_process(process), m_win(new WinState)
{
    WinState *win = m_win.get();
    win->killOnRelease = (onRelease == OnRelease::KillDescendants);
    m_process->setCreateProcessArgumentsModifier([win](QProcess::CreateProcessArguments *args) {
        win->reset();

        HANDLE job = CreateJobObjectW(nullptr, nullptr);
        if (!job)
            return; // start without a job: same behavior as before

        if (win->killOnRelease) {
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
            limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits,
                                         sizeof(limits))) {
                CloseHandle(job);
                return;
            }
        }

        SIZE_T size = 0;
        InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
        win->attributeBuffer.resize(size);
        auto *list = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(win->attributeBuffer.data());
        if (!InitializeProcThreadAttributeList(list, 1, 0, &size)) {
            CloseHandle(job);
            return;
        }
        win->attributeListInitialized = true;
        win->job = job;

        // PROC_THREAD_ATTRIBUTE_JOB_LIST assigns the job atomically at
        // creation, before the process can start any child of its own.
        if (!UpdateProcThreadAttribute(list, 0, PROC_THREAD_ATTRIBUTE_JOB_LIST, &win->job,
                                       sizeof(win->job), nullptr, nullptr)) {
            win->reset();
            return;
        }

        win->startupInfo = {};
        win->startupInfo.StartupInfo = *args->startupInfo; // keeps Qt's stdio handles
        win->startupInfo.StartupInfo.cb = sizeof(win->startupInfo);
        win->startupInfo.lpAttributeList = list;
        args->startupInfo = &win->startupInfo.StartupInfo;
        args->flags |= EXTENDED_STARTUPINFO_PRESENT;
    });
}

ProcessTree::~ProcessTree()
{
    m_win->reset();
}

void ProcessTree::killTree(int timeoutMs)
{
    HANDLE job = m_win->job;
    if (!job) {
        m_process->kill();
        return;
    }
    // TerminateJobObject() doesn't wait, and the job's ActiveProcesses count
    // drops to 0 before the processes have exited (measured). Open a handle
    // to each member first, so we can wait for the real exits: until then
    // their open files can't be deleted.
    constexpr DWORD maxIds = 256;
    std::vector<char> buffer(sizeof(JOBOBJECT_BASIC_PROCESS_ID_LIST)
                             + maxIds * sizeof(ULONG_PTR));
    auto *ids = reinterpret_cast<JOBOBJECT_BASIC_PROCESS_ID_LIST *>(buffer.data());
    std::vector<HANDLE> members;
    // A full buffer reports ERROR_MORE_DATA; the ids it holds are still valid.
    if (QueryInformationJobObject(job, JobObjectBasicProcessIdList, ids,
                                  DWORD(buffer.size()), nullptr)
        || GetLastError() == ERROR_MORE_DATA) {
        for (DWORD i = 0; i < ids->NumberOfProcessIdsInList; ++i)
            if (HANDLE h = OpenProcess(SYNCHRONIZE, FALSE, DWORD(ids->ProcessIdList[i])))
                members.push_back(h);
    }

    TerminateJobObject(job, 1);

    QElapsedTimer timer;
    timer.start();
    for (size_t first = 0; first < members.size(); first += MAXIMUM_WAIT_OBJECTS) {
        const DWORD count = DWORD(qMin<size_t>(MAXIMUM_WAIT_OBJECTS, members.size() - first));
        const qint64 left = qMax<qint64>(0, timeoutMs - timer.elapsed());
        WaitForMultipleObjects(count, members.data() + first, TRUE, DWORD(left));
    }
    for (HANDLE h : members)
        CloseHandle(h);
}

#else // Unix

// OnRelease has no effect here: releasing never kills anything on Unix.
ProcessTree::ProcessTree(QProcess *process, OnRelease /*onRelease*/) : m_process(process)
{
    // A new session makes the child a process-group leader (pgid == pid);
    // everything it starts inherits that group.
    m_process->setUnixProcessParameters(QProcess::UnixProcessFlag::CreateNewSession);
}

ProcessTree::~ProcessTree() = default;

void ProcessTree::killTree(int /*timeoutMs*/)
{
    const qint64 pid = m_process->processId();
    if (pid > 0)
        ::kill(-static_cast<pid_t>(pid), SIGKILL);
    else
        m_process->kill();
    // Unix can delete files that a dying process still holds open, so there
    // is nothing to wait for here.
}

#endif
