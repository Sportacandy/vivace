/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "externaldownloader.h"

#include <QDir>
#include <QFileInfo>

ExternalDownloader::ExternalDownloader(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(this);
    // Merge stderr into stdout: batch downloaders echo progress to both.
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    connect(m_process, &QProcess::finished, this,
            &ExternalDownloader::onFinished);
    connect(m_process, &QProcess::errorOccurred, this,
            &ExternalDownloader::onErrorOccurred);
    connect(m_process, &QProcess::readyReadStandardOutput, this,
            &ExternalDownloader::onReadyRead);
}

ExternalDownloader::~ExternalDownloader()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

void ExternalDownloader::setCommand(const QString &command)
{
    if (command == m_command)
        return;
    m_command = command;
    emit configChanged();
}

void ExternalDownloader::setArguments(const QString &arguments)
{
    if (arguments == m_arguments)
        return;
    m_arguments = arguments;
    emit configChanged();
}

void ExternalDownloader::setOutputFolder(const QString &folder)
{
    if (folder == m_outputFolder)
        return;
    m_outputFolder = folder;
    emit configChanged();
}

void ExternalDownloader::setBusy(bool busy)
{
    if (busy == m_busy)
        return;
    m_busy = busy;
    emit busyChanged();
}

bool ExternalDownloader::isConfigured() const
{
    return !m_command.isEmpty() && !m_outputFolder.isEmpty()
            && QFileInfo(m_outputFolder).isDir();
}

bool ExternalDownloader::isMediaFile(const QString &fileName)
{
    static const QStringList exts = {
        QStringLiteral("mp4"),  QStringLiteral("mkv"),  QStringLiteral("webm"),
        QStringLiteral("mov"),  QStringLiteral("avi"),  QStringLiteral("m4v"),
        QStringLiteral("ts"),   QStringLiteral("flv"),  QStringLiteral("wmv"),
        QStringLiteral("mp3"),  QStringLiteral("m4a"),  QStringLiteral("opus"),
        QStringLiteral("flac"), QStringLiteral("wav"),  QStringLiteral("aac"),
        QStringLiteral("ogg")
    };
    return exts.contains(QFileInfo(fileName).suffix().toLower());
}

QString ExternalDownloader::newestOutputFile() const
{
    // The file the tool just produced: the most recently modified media file in
    // the output folder whose timestamp is at or after this run's start. (The
    // tool names the file itself — often the video title — so we can't predict
    // it, only detect its appearance.)
    QDir dir(m_outputFolder);
    QString best;
    QDateTime bestTime; // invalid until a candidate is found
    const QFileInfoList files = dir.entryInfoList(QDir::Files);
    for (const QFileInfo &fi : files) {
        if (!isMediaFile(fi.fileName()))
            continue;
        const QDateTime mt = fi.lastModified();
        if (mt < m_startTime)
            continue; // predates this run
        if (!bestTime.isValid() || mt > bestTime) {
            bestTime = mt;
            best = fi.absoluteFilePath();
        }
    }
    return best;
}

void ExternalDownloader::download(const QString &url)
{
    cancel();

    if (m_command.isEmpty()) {
        emit failed(tr("No downloader command is configured "
                       "(Preferences > Network)."));
        return;
    }
    if (m_outputFolder.isEmpty() || !QFileInfo(m_outputFolder).isDir()) {
        emit failed(tr("The download folder does not exist "
                       "(Preferences > Network)."));
        return;
    }

    // Back-date the start slightly so a file written immediately (coarse
    // filesystem timestamps) is still detected as "new".
    m_startTime = QDateTime::currentDateTime().addSecs(-3);

    // Build the tool arguments from the template, substituting {url}.
    QStringList toolArgs;
    bool urlPlaced = false;
    const QStringList tokens =
            m_arguments.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (const QString &token : tokens) {
        if (token.contains(QLatin1String("{url}"))) {
            QString t = token;
            t.replace(QLatin1String("{url}"), url);
            toolArgs << t;
            urlPlaced = true;
        } else {
            toolArgs << token;
        }
    }
    if (!urlPlaced)
        toolArgs << url;

    // A .bat/.cmd must be launched through the command interpreter.
    QString program = m_command;
    QStringList args;
    const QString suffix = QFileInfo(m_command).suffix().toLower();
    if (suffix == QLatin1String("bat") || suffix == QLatin1String("cmd")) {
        program = QStringLiteral("cmd");
        args << QStringLiteral("/c") << m_command << toolArgs;
    } else {
        args = toolArgs;
    }

    m_process->setWorkingDirectory(m_outputFolder);
    setBusy(true);
    m_process->start(program, args);
}

void ExternalDownloader::cancel()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
    setBusy(false);
}

void ExternalDownloader::onReadyRead()
{
    const QString text = QString::fromLocal8Bit(m_process->readAllStandardOutput());
    const QStringList lines = text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (!trimmed.isEmpty())
            emit progress(trimmed);
    }
}

void ExternalDownloader::onErrorOccurred(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart) {
        setBusy(false);
        emit failed(tr("Could not run the downloader (\"%1\").").arg(m_command));
    }
}

void ExternalDownloader::onFinished(int exitCode, QProcess::ExitStatus status)
{
    setBusy(false);

    // Prefer detecting a produced file over trusting the exit code: batch
    // wrappers often return a non-zero or unreliable code even on success.
    const QString file = newestOutputFile();
    if (!file.isEmpty()) {
        emit downloaded(QUrl::fromLocalFile(file), QFileInfo(file).completeBaseName());
        return;
    }

    if (status != QProcess::NormalExit) {
        emit failed(tr("The downloader did not finish normally."));
        return;
    }
    emit failed(tr("The downloader produced no media file (exit code %1).")
                        .arg(exitCode));
}
