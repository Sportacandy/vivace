/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Structure ported from SMPlayer's mpris2/ (GPL-2.0-or-later), originally
    from KDE (Eike Hein, 2012).
*/

#include "mpris2.h"

#include "playercontroller.h"

#include <QAudioOutput>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QFileInfo>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QMetaClassInfo>
#include <QWindow>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

// ---- Mpris2 (service/object registration) ----------------------------------

Mpris2::Mpris2(PlayerController *controller, QWindow *window, QObject *parent)
    : QObject(parent)
{
    const QString name = QStringLiteral("org.mpris.MediaPlayer2.vivace");
    bool ok = QDBusConnection::sessionBus().registerService(name);
    if (!ok) {
        // The well-known name is taken (another instance): the spec wants a
        // unique fallback name.
        ok = QDBusConnection::sessionBus().registerService(
            name + QStringLiteral(".instance")
            + QString::number(QCoreApplication::applicationPid()));
    }
    if (!ok)
        return;

    new MprisRoot(window, this);
    new MprisPlayer(controller, this);
    QDBusConnection::sessionBus().registerObject(
        QStringLiteral("/org/mpris/MediaPlayer2"), this,
        QDBusConnection::ExportAdaptors);
}

void Mpris2::signalPropertiesChange(const QObject *adaptor,
                                    const QVariantMap &properties)
{
    QDBusMessage msg = QDBusMessage::createSignal(
        QStringLiteral("/org/mpris/MediaPlayer2"),
        QStringLiteral("org.freedesktop.DBus.Properties"),
        QStringLiteral("PropertiesChanged"));

    QVariantList args;
    args << QString::fromLatin1(adaptor->metaObject()->classInfo(0).value());
    args << properties;
    args << QStringList();
    msg.setArguments(args);

    QDBusConnection::sessionBus().send(msg);
}

// ---- MprisRoot (org.mpris.MediaPlayer2) ------------------------------------

MprisRoot::MprisRoot(QWindow *window, QObject *parent)
    : QDBusAbstractAdaptor(parent), m_window(window)
{
}

QStringList MprisRoot::SupportedUriSchemes() const
{
    return { QStringLiteral("file"), QStringLiteral("http"),
             QStringLiteral("https"), QStringLiteral("ftp"),
             QStringLiteral("rtsp") };
}

QStringList MprisRoot::SupportedMimeTypes() const
{
    return { QStringLiteral("video/mp4"), QStringLiteral("video/x-matroska"),
             QStringLiteral("video/webm"), QStringLiteral("video/mpeg"),
             QStringLiteral("video/quicktime"), QStringLiteral("audio/mpeg"),
             QStringLiteral("audio/flac"), QStringLiteral("audio/x-wav"),
             QStringLiteral("audio/ogg"), QStringLiteral("audio/mp4") };
}

void MprisRoot::Raise()
{
    if (m_window) {
        m_window->show();
        m_window->raise();
        m_window->requestActivate();
    }
}

void MprisRoot::Quit()
{
    QCoreApplication::quit();
}

// ---- MprisPlayer (org.mpris.MediaPlayer2.Player) ---------------------------

static QByteArray makeTrackId(const QString &source)
{
    return QByteArray("/org/vivace/tid_")
           + QCryptographicHash::hash(source.toUtf8(), QCryptographicHash::Sha1)
                 .toHex();
}

MprisPlayer::MprisPlayer(PlayerController *controller, QObject *parent)
    : QDBusAbstractAdaptor(parent), m_controller(controller)
{
    QMediaPlayer *player = m_controller->player();
    connect(player, &QMediaPlayer::playbackStateChanged,
            this, &MprisPlayer::onStateChanged);
    connect(player, &QMediaPlayer::metaDataChanged,
            this, &MprisPlayer::onMetadataChanged);
    connect(player, &QMediaPlayer::sourceChanged,
            this, &MprisPlayer::onMetadataChanged);
    connect(player, &QMediaPlayer::durationChanged,
            this, &MprisPlayer::onMetadataChanged);
    connect(player, &QMediaPlayer::seekableChanged,
            this, &MprisPlayer::onStateChanged);
    connect(m_controller->audioOutput(), &QAudioOutput::volumeChanged,
            this, &MprisPlayer::onVolumeChanged);
    connect(m_controller, &PlayerController::seeked,
            this, &MprisPlayer::onSeeked);
}

QString MprisPlayer::PlaybackStatus() const
{
    switch (m_controller->player()->playbackState()) {
    case QMediaPlayer::PlayingState:
        return QStringLiteral("Playing");
    case QMediaPlayer::PausedState:
        return QStringLiteral("Paused");
    default:
        return QStringLiteral("Stopped");
    }
}

double MprisPlayer::Rate() const
{
    return m_controller->player()->playbackRate();
}

void MprisPlayer::setRate(double rate) const
{
    m_controller->player()->setPlaybackRate(rate);
}

QVariantMap MprisPlayer::Metadata() const
{
    QVariantMap map;
    QMediaPlayer *player = m_controller->player();
    const QUrl source = player->source();
    if (source.isEmpty())
        return map;

    map[QStringLiteral("mpris:trackid")] = QVariant::fromValue(
        QDBusObjectPath(makeTrackId(source.toString()).constData()));
    if (player->duration() > 0)
        map[QStringLiteral("mpris:length")] =
            qlonglong(player->duration()) * 1000; // ms -> µs
    map[QStringLiteral("xesam:url")] = source.toString();

    const QMediaMetaData meta = player->metaData();
    QString title = meta.stringValue(QMediaMetaData::Title);
    if (title.isEmpty() && source.isLocalFile())
        title = QFileInfo(source.toLocalFile()).fileName();
    if (!title.isEmpty())
        map[QStringLiteral("xesam:title")] = title;

    const QString artist = meta.stringValue(QMediaMetaData::ContributingArtist);
    if (!artist.isEmpty())
        map[QStringLiteral("xesam:artist")] = QStringList { artist };
    const QString album = meta.stringValue(QMediaMetaData::AlbumTitle);
    if (!album.isEmpty())
        map[QStringLiteral("xesam:album")] = album;

    return map;
}

double MprisPlayer::Volume() const
{
    return m_controller->audioOutput()->volume();
}

void MprisPlayer::setVolume(double volume) const
{
    m_controller->audioOutput()->setVolume(float(qBound(0.0, volume, 1.0)));
}

qlonglong MprisPlayer::Position() const
{
    return qlonglong(m_controller->player()->position()) * 1000; // ms -> µs
}

bool MprisPlayer::CanGoNext() const
{
    return m_controller->playlist()->count() > 0;
}

bool MprisPlayer::CanGoPrevious() const
{
    return m_controller->playlist()->count() > 0;
}

bool MprisPlayer::CanSeek() const
{
    return m_controller->player()->isSeekable();
}

void MprisPlayer::Next()
{
    m_controller->next();
}

void MprisPlayer::Previous()
{
    m_controller->previous();
}

void MprisPlayer::Pause()
{
    m_controller->player()->pause();
}

void MprisPlayer::PlayPause()
{
    m_controller->togglePlayPause();
}

void MprisPlayer::Stop()
{
    m_controller->stop();
}

void MprisPlayer::Play()
{
    m_controller->player()->play();
}

void MprisPlayer::Seek(qlonglong offset)
{
    m_controller->seekRelative(offset / 1000); // µs -> ms
}

void MprisPlayer::SetPosition(const QDBusObjectPath &trackId, qlonglong position)
{
    const QUrl source = m_controller->player()->source();
    if (trackId.path().toUtf8() == makeTrackId(source.toString()))
        m_controller->player()->setPosition(position / 1000); // µs -> ms
}

void MprisPlayer::onStateChanged()
{
    QVariantMap props;
    props[QStringLiteral("PlaybackStatus")] = PlaybackStatus();
    props[QStringLiteral("CanSeek")] = CanSeek();
    Mpris2::signalPropertiesChange(this, props);
}

void MprisPlayer::onMetadataChanged()
{
    QVariantMap props;
    props[QStringLiteral("Metadata")] = Metadata();
    props[QStringLiteral("CanGoNext")] = CanGoNext();
    props[QStringLiteral("CanGoPrevious")] = CanGoPrevious();
    Mpris2::signalPropertiesChange(this, props);
}

void MprisPlayer::onVolumeChanged()
{
    QVariantMap props;
    props[QStringLiteral("Volume")] = Volume();
    Mpris2::signalPropertiesChange(this, props);
}

void MprisPlayer::onSeeked()
{
    emit Seeked(Position());
}
