/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// WinRT headers are included before Qt so Qt's `signals`/`slots`/`emit` macros
// don't leak into the generated C++/WinRT projections.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.h>
#include <winrt/Windows.Storage.Streams.h>
#include <systemmediatransportcontrolsinterop.h>

#include <chrono>

#include "windowssmtc.h"

#include "playercontroller.h"

#include <QAudioOutput>
#include <QFileInfo>
#include <QMediaMetaData>
#include <QMediaPlayer>
#include <QMetaObject>
#include <QUrl>
#include <QWindow>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Media;

struct WindowsSmtc::Impl {
    SystemMediaTransportControls smtc{ nullptr };
    SystemMediaTransportControls::ButtonPressed_revoker buttonRevoker;
};

static hstring toHstring(const QString &s)
{
    return hstring{ reinterpret_cast<const wchar_t *>(s.utf16()),
                    static_cast<uint32_t>(s.size()) };
}

WindowsSmtc::WindowsSmtc(PlayerController *controller, QWindow *window,
                         QObject *parent)
    : QObject(parent), m_controller(controller)
{
    // Qt already puts the GUI thread in an STA (OleInitialize); init_apartment
    // then just no-ops. Guard anyway.
    try {
        init_apartment(apartment_type::single_threaded);
    } catch (...) {
    }

    const HWND hwnd = reinterpret_cast<HWND>(window->winId());
    try {
        auto interop = get_activation_factory<SystemMediaTransportControls,
                                              ISystemMediaTransportControlsInterop>();
        SystemMediaTransportControls smtc{ nullptr };
        check_hresult(interop->GetForWindow(
            hwnd, guid_of<SystemMediaTransportControls>(), put_abi(smtc)));

        m_impl = std::make_unique<Impl>();
        m_impl->smtc = smtc;
        smtc.IsEnabled(true);
        smtc.IsPlayEnabled(true);
        smtc.IsPauseEnabled(true);
        smtc.IsStopEnabled(true);
        smtc.IsNextEnabled(true);
        smtc.IsPreviousEnabled(true);

        // The callback arrives on a WinRT thread pool thread; marshal to the
        // Qt (GUI) thread before touching the player.
        m_impl->buttonRevoker = smtc.ButtonPressed(
            auto_revoke,
            [this](SystemMediaTransportControls const &,
                   SystemMediaTransportControlsButtonPressedEventArgs const &args) {
                const int button = static_cast<int>(args.Button());
                QMetaObject::invokeMethod(this, "handleButton",
                                          Qt::QueuedConnection,
                                          Q_ARG(int, button));
            });
    } catch (...) {
        m_impl.reset();
        return;
    }

    QMediaPlayer *player = m_controller->player();
    connect(player, &QMediaPlayer::playbackStateChanged,
            this, &WindowsSmtc::updatePlaybackState);
    connect(player, &QMediaPlayer::metaDataChanged,
            this, &WindowsSmtc::updateMetadata);
    connect(player, &QMediaPlayer::sourceChanged,
            this, &WindowsSmtc::updateMetadata);
    connect(player, &QMediaPlayer::hasVideoChanged,
            this, &WindowsSmtc::updateMetadata);
    connect(player, &QMediaPlayer::durationChanged,
            this, &WindowsSmtc::updateTimeline);
    connect(m_controller, &PlayerController::seeked,
            this, &WindowsSmtc::updateTimeline);

    updatePlaybackState();
    updateMetadata();
}

WindowsSmtc::~WindowsSmtc()
{
    if (m_impl) {
        try {
            m_impl->buttonRevoker.revoke();
            m_impl->smtc.IsEnabled(false);
        } catch (...) {
        }
    }
}

void WindowsSmtc::handleButton(int button)
{
    if (!m_impl)
        return;
    QMediaPlayer *player = m_controller->player();
    switch (static_cast<SystemMediaTransportControlsButton>(button)) {
    case SystemMediaTransportControlsButton::Play:
        player->play();
        break;
    case SystemMediaTransportControlsButton::Pause:
        m_controller->pause();
        break;
    case SystemMediaTransportControlsButton::Stop:
        m_controller->stop();
        break;
    case SystemMediaTransportControlsButton::Next:
        m_controller->next();
        break;
    case SystemMediaTransportControlsButton::Previous:
        m_controller->previous();
        break;
    default:
        break;
    }
}

void WindowsSmtc::updatePlaybackState()
{
    if (!m_impl)
        return;
    MediaPlaybackStatus status = MediaPlaybackStatus::Stopped;
    switch (m_controller->player()->playbackState()) {
    case QMediaPlayer::PlayingState:
        status = MediaPlaybackStatus::Playing;
        break;
    case QMediaPlayer::PausedState:
        status = MediaPlaybackStatus::Paused;
        break;
    default:
        status = MediaPlaybackStatus::Stopped;
        break;
    }
    try {
        m_impl->smtc.PlaybackStatus(status);
    } catch (...) {
    }
}

void WindowsSmtc::updateMetadata()
{
    if (!m_impl)
        return;
    QMediaPlayer *player = m_controller->player();
    try {
        auto updater = m_impl->smtc.DisplayUpdater();
        const QUrl source = player->source();
        if (source.isEmpty()) {
            updater.ClearAll();
            updater.Update();
            return;
        }

        const QMediaMetaData meta = player->metaData();
        QString title = meta.stringValue(QMediaMetaData::Title);
        if (title.isEmpty() && source.isLocalFile())
            title = QFileInfo(source.toLocalFile()).fileName();
        const QString artist = meta.stringValue(QMediaMetaData::ContributingArtist);

        if (player->hasVideo()) {
            updater.Type(MediaPlaybackType::Video);
            auto props = updater.VideoProperties();
            props.Title(toHstring(title));
            if (!artist.isEmpty())
                props.Subtitle(toHstring(artist));
        } else {
            updater.Type(MediaPlaybackType::Music);
            auto props = updater.MusicProperties();
            props.Title(toHstring(title));
            if (!artist.isEmpty())
                props.Artist(toHstring(artist));
            const QString album = meta.stringValue(QMediaMetaData::AlbumTitle);
            if (!album.isEmpty())
                props.AlbumTitle(toHstring(album));
        }
        updater.Update();
    } catch (...) {
    }
    updateTimeline();
}

void WindowsSmtc::updateTimeline()
{
    if (!m_impl)
        return;
    try {
        QMediaPlayer *player = m_controller->player();
        SystemMediaTransportControlsTimelineProperties tl;
        tl.StartTime(TimeSpan{ std::chrono::milliseconds{ 0 } });
        tl.MinSeekTime(TimeSpan{ std::chrono::milliseconds{ 0 } });
        tl.EndTime(TimeSpan{ std::chrono::milliseconds{ player->duration() } });
        tl.MaxSeekTime(TimeSpan{ std::chrono::milliseconds{ player->duration() } });
        tl.Position(TimeSpan{ std::chrono::milliseconds{ player->position() } });
        m_impl->smtc.UpdateTimelineProperties(tl);
    } catch (...) {
    }
}
