/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    OpenSubtitles search/download over the REST API (api.opensubtitles.com/v1).
    The REST API requires a per-application Api-Key; Vivace does not ship one
    (reusing another app's key is against the ToS), so the key — and, for
    downloads, an account login — are user-provided in Preferences > Network.
    Flow modelled on SMPlayer's findsubtitles/osclient.cpp (GPL-2.0-or-later).
    The movie hash is the classic OpenSubtitles hash (filesize + first/last 64K).
*/

#ifndef OPENSUBTITLESCLIENT_H
#define OPENSUBTITLESCLIENT_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

class OpenSubtitlesClient : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString apiKey READ apiKey WRITE setApiKey NOTIFY credentialsChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername
               NOTIFY credentialsChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword
               NOTIFY credentialsChanged)
    Q_PROPERTY(bool configured READ configured NOTIFY credentialsChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    // Rows: { language, release, fileName, downloads, rating, fileId }.
    Q_PROPERTY(QVariantList results READ results NOTIFY resultsChanged)

public:
    explicit OpenSubtitlesClient(QObject *parent = nullptr);

    QString apiKey() const { return m_apiKey; }
    void setApiKey(const QString &key);
    QString username() const { return m_username; }
    void setUsername(const QString &user);
    QString password() const { return m_password; }
    void setPassword(const QString &pass);
    bool configured() const { return !m_apiKey.isEmpty(); }
    bool busy() const { return m_busy; }
    QString status() const { return m_status; }
    QVariantList results() const { return m_results; }

    // The OpenSubtitles movie hash of a local file (empty on failure).
    Q_INVOKABLE static QString hashFile(const QString &filePath);

    // Search by movie hash (from filePath) and/or a text query, restricted to
    // comma-separated ISO-639 language codes (e.g. "en,ja"). Either may be empty.
    Q_INVOKABLE void search(const QString &filePath, const QString &query,
                            const QString &languages);
    // Download the chosen result and save it next to videoPath (or to a temp
    // file); emits subtitleReady(localPath) on success.
    Q_INVOKABLE void download(const QString &fileId, const QString &videoPath);
    Q_INVOKABLE void clearResults();

signals:
    void credentialsChanged();
    void busyChanged();
    void statusChanged();
    void resultsChanged();
    void subtitleReady(const QUrl &localUrl);

private:
    void setBusy(bool busy);
    void setStatus(const QString &status);
    QNetworkRequest apiRequest(const QString &path) const;
    void loginThenDownload(const QString &fileId, const QString &videoPath);
    void requestDownloadLink(const QString &fileId, const QString &videoPath);
    void fetchAndSave(const QUrl &link, const QString &videoPath,
                      const QString &suggestedName);

    QNetworkAccessManager m_nam;
    QString m_apiKey;
    QString m_username;
    QString m_password;
    QByteArray m_token; // JWT from /login, for downloads
    bool m_busy = false;
    QString m_status;
    QVariantList m_results;
};

#endif // OPENSUBTITLESCLIENT_H
