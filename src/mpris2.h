/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    MPRIS2 (org.mpris.MediaPlayer2) D-Bus integration for Linux desktops: lets
    the desktop's media widgets and the multimedia keys control Vivace and show
    the current track. Structure ported from SMPlayer's mpris2/ (itself derived
    from KDE's reference adaptors), wired to PlayerController instead of Core.
    Only instantiated on Linux; the session bus is absent elsewhere.
*/

#ifndef MPRIS2_H
#define MPRIS2_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QObject>
#include <QStringList>
#include <QVariantMap>

class PlayerController;
class QWindow;

// Owns the two adaptors and registers the service/object on the session bus.
class Mpris2 : public QObject
{
    Q_OBJECT

public:
    Mpris2(PlayerController *controller, QWindow *window, QObject *parent = nullptr);

    // Emits org.freedesktop.DBus.Properties.PropertiesChanged for an adaptor.
    static void signalPropertiesChange(const QObject *adaptor,
                                       const QVariantMap &properties);
};

// org.mpris.MediaPlayer2 — the application root node.
class MprisRoot : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")

    Q_PROPERTY(bool CanQuit READ CanQuit)
    Q_PROPERTY(bool CanRaise READ CanRaise)
    Q_PROPERTY(bool HasTrackList READ HasTrackList)
    Q_PROPERTY(QString Identity READ Identity)
    Q_PROPERTY(QString DesktopEntry READ DesktopEntry)
    Q_PROPERTY(QStringList SupportedUriSchemes READ SupportedUriSchemes)
    Q_PROPERTY(QStringList SupportedMimeTypes READ SupportedMimeTypes)
    Q_PROPERTY(bool Fullscreen READ Fullscreen WRITE setFullscreen)
    Q_PROPERTY(bool CanSetFullscreen READ CanSetFullscreen)

public:
    MprisRoot(QWindow *window, QObject *parent);

    bool CanQuit() const { return true; }
    bool CanRaise() const { return m_window != nullptr; }
    bool HasTrackList() const { return false; }
    QString Identity() const { return QStringLiteral("Vivace"); }
    QString DesktopEntry() const { return QStringLiteral("vivace"); }
    QStringList SupportedUriSchemes() const;
    QStringList SupportedMimeTypes() const;
    bool Fullscreen() const { return false; }
    void setFullscreen(bool) const {}
    bool CanSetFullscreen() const { return false; }

public slots:
    void Raise();
    void Quit();

private:
    QWindow *m_window;
};

// org.mpris.MediaPlayer2.Player — playback control and metadata.
class MprisPlayer : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

    Q_PROPERTY(QString PlaybackStatus READ PlaybackStatus)
    Q_PROPERTY(double Rate READ Rate WRITE setRate)
    Q_PROPERTY(QVariantMap Metadata READ Metadata)
    Q_PROPERTY(double Volume READ Volume WRITE setVolume)
    Q_PROPERTY(qlonglong Position READ Position)
    Q_PROPERTY(double MinimumRate READ MinimumRate)
    Q_PROPERTY(double MaximumRate READ MaximumRate)
    Q_PROPERTY(bool CanGoNext READ CanGoNext)
    Q_PROPERTY(bool CanGoPrevious READ CanGoPrevious)
    Q_PROPERTY(bool CanPlay READ CanPlay)
    Q_PROPERTY(bool CanPause READ CanPause)
    Q_PROPERTY(bool CanSeek READ CanSeek)
    Q_PROPERTY(bool CanControl READ CanControl)

public:
    MprisPlayer(PlayerController *controller, QObject *parent);

    QString PlaybackStatus() const;
    double Rate() const;
    void setRate(double rate) const;
    QVariantMap Metadata() const;
    double Volume() const;
    void setVolume(double volume) const;
    qlonglong Position() const;
    double MinimumRate() const { return 0.25; }
    double MaximumRate() const { return 4.0; }
    bool CanGoNext() const;
    bool CanGoPrevious() const;
    bool CanPlay() const { return true; }
    bool CanPause() const { return true; }
    bool CanSeek() const;
    bool CanControl() const { return true; }

signals:
    void Seeked(qlonglong Position);

public slots:
    void Next();
    void Previous();
    void Pause();
    void PlayPause();
    void Stop();
    void Play();
    void Seek(qlonglong Offset);
    void SetPosition(const QDBusObjectPath &TrackId, qlonglong Position);

private slots:
    void onStateChanged();
    void onMetadataChanged();
    void onVolumeChanged();
    void onSeeked();

private:
    PlayerController *m_controller;
};

#endif // MPRIS2_H
