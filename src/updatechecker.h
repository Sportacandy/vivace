/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Checks a remote endpoint for a newer Vivace release and reports it to the
    UI. Modelled on SMPlayer's updatechecker.cpp (GPL-2.0-or-later): fetch a
    small version-info document, compare its "stable" version against the
    running one (padded component-wise), and notify when a newer one exists.
    Automatic checks stay silent unless a newer version is found; a
    user-requested check also reports "up to date" and errors.

    NOTE: the check URL is a PLACEHOLDER (an RFC 6761 .invalid host) until a
    real version-info endpoint exists — see kDefaultCheckUrl.
*/

#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

class UpdateChecker : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    // Where to look for the latest version; placeholder by default.
    Q_PROPERTY(QUrl checkUrl READ checkUrl WRITE setCheckUrl NOTIFY checkUrlChanged)
    // The page opened when the user wants to know more / download; may be
    // overridden by the fetched document.
    Q_PROPERTY(QUrl downloadUrl READ downloadUrl WRITE setDownloadUrl
                       NOTIFY downloadUrlChanged)

public:
    explicit UpdateChecker(QObject *parent = nullptr);

    QString currentVersion() const { return m_currentVersion; }
    bool busy() const { return m_busy; }
    QUrl checkUrl() const { return m_checkUrl; }
    void setCheckUrl(const QUrl &url);
    QUrl downloadUrl() const { return m_downloadUrl; }
    void setDownloadUrl(const QUrl &url);

    // Start a check. Automatic checks (userInitiated == false) only signal when
    // a newer version is found; user checks also report up-to-date and errors.
    Q_INVOKABLE void check(bool userInitiated = true);

    // True when at least intervalDays have passed since lastCheckIso (an empty
    // date means "never checked" => due). Lets QML gate the startup check.
    Q_INVOKABLE bool isCheckDue(const QString &lastCheckIso, int intervalDays) const;

    // Compare two dotted versions; > 0 if a is newer than b (component-wise).
    Q_INVOKABLE static int compareVersions(const QString &a, const QString &b);

signals:
    void busyChanged();
    void checkUrlChanged();
    void downloadUrlChanged();
    // A newer version than the running one is available.
    void newVersionFound(const QString &version);
    // User-requested check only: already on the latest version.
    void upToDate(const QString &version);
    // User-requested check only: the check failed.
    void errorOccurred(const QString &message);
    // Emitted after every completed check; success == network+parse ok.
    void checkFinished(bool success);

private:
    void onReply(class QNetworkReply *reply, bool userInitiated);
    static QString parseVersion(const QByteArray &data, QString *downloadUrl);
    static QString formattedVersion(const QString &version);

    QNetworkAccessManager m_net;
    QString m_currentVersion;
    QUrl m_checkUrl;
    QUrl m_downloadUrl;
    bool m_busy = false;
};

#endif // UPDATECHECKER_H
