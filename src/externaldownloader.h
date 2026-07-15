/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Runs a user-provided external tool that DOWNLOADS a URL to a file (e.g. a
    yt-dlp wrapper that merges separate video+audio DASH streams into an HD
    MP4), then reports the produced file so Vivace can play it. This is the
    "download then play" alternative to the yt-dlp streaming resolver: HD YouTube
    formats are adaptive video-only + audio-only streams that QMediaPlayer cannot
    mux on the fly, so streaming caps at the best progressive format (~720p).
    Delegating to an external downloader keeps that tool's format-selection,
    cookies and anti-bot handling entirely outside Vivace.

    The tool writes its output into a folder the user configures; after the tool
    exits, the newest media file created in that folder (since the run started)
    is taken as the result. Vivace does not parse or control the tool beyond the
    argument template — the tool owns how it picks and produces the file.
*/

#ifndef EXTERNALDOWNLOADER_H
#define EXTERNALDOWNLOADER_H

#include <QDateTime>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

class ExternalDownloader : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    // The downloader executable or script (a .bat/.cmd is run via cmd /c).
    Q_PROPERTY(QString command READ command WRITE setCommand NOTIFY configChanged)
    // Argument template; a "{url}" token is replaced by the URL (if absent, the
    // URL is appended as the last argument). Tokens are whitespace-separated.
    Q_PROPERTY(QString arguments READ arguments WRITE setArguments
                       NOTIFY configChanged)
    // Folder the tool writes its output into; watched for the produced file.
    Q_PROPERTY(QString outputFolder READ outputFolder WRITE setOutputFolder
                       NOTIFY configChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    explicit ExternalDownloader(QObject *parent = nullptr);
    ~ExternalDownloader() override;

    QString command() const { return m_command; }
    void setCommand(const QString &command);
    QString arguments() const { return m_arguments; }
    void setArguments(const QString &arguments);
    QString outputFolder() const { return m_outputFolder; }
    void setOutputFolder(const QString &folder);
    bool busy() const { return m_busy; }

    // True when a command and an existing output folder are configured.
    Q_INVOKABLE bool isConfigured() const;

    // Run the tool for `url`; emits downloaded() (with the produced file) or
    // failed(). A run already in progress is cancelled first.
    Q_INVOKABLE void download(const QString &url);
    Q_INVOKABLE void cancel();

signals:
    void configChanged();
    void busyChanged();
    // A line of the tool's merged stdout/stderr, for a progress indicator.
    void progress(const QString &line);
    // The produced local file (fileUrl) and a title derived from its name.
    void downloaded(const QUrl &fileUrl, const QString &title);
    void failed(const QString &message);

private:
    void setBusy(bool busy);
    void onFinished(int exitCode, QProcess::ExitStatus status);
    void onErrorOccurred(QProcess::ProcessError error);
    void onReadyRead();
    QString newestOutputFile() const;
    static bool isMediaFile(const QString &fileName);

    QProcess *m_process = nullptr;
    QString m_command;
    QString m_arguments = QStringLiteral("{url}");
    QString m_outputFolder;
    bool m_busy = false;
    QDateTime m_startTime;
};

#endif // EXTERNALDOWNLOADER_H
