/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef ANDROIDCONTENTPATH_H
#define ANDROIDCONTENTPATH_H

#include <functional>

#include <QString>
#include <QUrl>

class QObject;

/*  Android helpers for playlists picked with the system file picker.

    The picker returns a content:// URI that grants access to that one
    document only, so a playlist's relative entries (bare file names, as
    Vivace itself writes them) can't be resolved from it. These helpers map
    the URI back to the real file path, so the entries resolve against the
    playlist's folder, and handle the media-read permission needed to then
    open those files by path.

    On other platforms they are no-ops: contentFilePath() returns an empty
    string and the permission is reported as granted.
*/
namespace AndroidContentPath {

// Real file-system path of a content:// document URI (from the Downloads,
// external-storage or media providers, or any URI exposing the _data
// column). Empty when it can't be determined.
QString contentFilePath(const QUrl &url);

// True when Vivace may read media files in shared storage by path
// (READ_EXTERNAL_STORAGE up to Android 12, READ_MEDIA_VIDEO or
// READ_MEDIA_AUDIO from Android 13).
bool hasMediaReadPermission();

// Asks the user for that permission; `done` runs on `context`'s thread with
// whether it was granted. Calls `done(true)` at once when already granted.
void requestMediaReadPermission(QObject *context, std::function<void(bool)> done);

} // namespace AndroidContentPath

#endif // ANDROIDCONTENTPATH_H
