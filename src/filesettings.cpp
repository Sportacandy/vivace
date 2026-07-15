/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "filesettings.h"

#include <QCryptographicHash>
#include <QDateTime>

FileSettings::FileSettings(QObject *parent)
    : QObject(parent),
      m_store(QSettings::IniFormat, QSettings::UserScope,
              QStringLiteral("vivace-player"), QStringLiteral("vivace_files"))
{
}

QString FileSettings::keyFor(const QUrl &url)
{
    const QByteArray hash = QCryptographicHash::hash(
            url.toString().toUtf8(), QCryptographicHash::Sha1);
    return QStringLiteral("files/") + QString::fromLatin1(hash.toHex());
}

qint64 FileSettings::position(const QUrl &url) const
{
    if (url.isEmpty())
        return -1;
    return m_store.value(keyFor(url) + QStringLiteral("/position"), -1)
            .toLongLong();
}

void FileSettings::setPosition(const QUrl &url, qint64 positionMs)
{
    if (url.isEmpty())
        return;
    const QString key = keyFor(url);
    m_store.setValue(key + QStringLiteral("/url"), url.toString());
    m_store.setValue(key + QStringLiteral("/position"), positionMs);
    m_store.setValue(key + QStringLiteral("/updated"),
                     QDateTime::currentSecsSinceEpoch());
    prune();
}

int FileSettings::audioTrack(const QUrl &url) const
{
    if (url.isEmpty())
        return notStored;
    return m_store.value(keyFor(url) + QStringLiteral("/audioTrack"), notStored)
            .toInt();
}

int FileSettings::subtitleTrack(const QUrl &url) const
{
    if (url.isEmpty())
        return notStored;
    return m_store.value(keyFor(url) + QStringLiteral("/subtitleTrack"), notStored)
            .toInt();
}

void FileSettings::setTracks(const QUrl &url, int audioTrack, int subtitleTrack)
{
    if (url.isEmpty())
        return;
    const QString key = keyFor(url);
    m_store.setValue(key + QStringLiteral("/url"), url.toString());
    m_store.setValue(key + QStringLiteral("/audioTrack"), audioTrack);
    m_store.setValue(key + QStringLiteral("/subtitleTrack"), subtitleTrack);
    m_store.setValue(key + QStringLiteral("/updated"),
                     QDateTime::currentSecsSinceEpoch());
}

void FileSettings::remove(const QUrl &url)
{
    if (url.isEmpty())
        return;
    m_store.remove(keyFor(url));
}

void FileSettings::clearAll()
{
    m_store.remove(QStringLiteral("files"));
}

void FileSettings::prune()
{
    m_store.beginGroup(QStringLiteral("files"));
    QStringList groups = m_store.childGroups();
    if (groups.size() > maxEntries) {
        std::sort(groups.begin(), groups.end(),
                  [this](const QString &a, const QString &b) {
                      return m_store.value(a + QStringLiteral("/updated")).toLongLong()
                             < m_store.value(b + QStringLiteral("/updated")).toLongLong();
                  });
        for (qsizetype i = 0; i < groups.size() - maxEntries; ++i)
            m_store.remove(groups.at(i));
    }
    m_store.endGroup();
}
