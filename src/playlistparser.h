/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef PLAYLISTPARSER_H
#define PLAYLISTPARSER_H

#include <QList>
#include <QUrl>

#include "playlistmodel.h"

/*  Playlist file loading (m3u/m3u8 for now; pls/xspf planned — see
    SMPlayer's playlist.cpp for the reference implementations).

    Only local files count as playlist files: a remote *.m3u8 URL is an
    HLS stream, which QMediaPlayer's FFmpeg backend plays natively.
*/
namespace PlaylistParser {

bool isPlaylistFile(const QUrl &url);
QList<PlaylistEntry> load(const QUrl &url);
bool save(const QUrl &url, const QList<PlaylistEntry> &entries);

} // namespace PlaylistParser

#endif // PLAYLISTPARSER_H
