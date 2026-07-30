# View

The **View** menu shows information and secondary panels.

- **Information and properties…** — a detailed dialog about the current media:
  general info, demuxer, and the video/audio codecs (with the formats Vivace's
  backend supports highlighted). It is informational only.
- **Playlist** (`F9`) — show or hide the playlist. Depending on
  *Preferences ▸ Playlist*, it appears as a docked panel or a separate window.
  See [Open](menu-open.md) for loading playlists.
  - Each row shows a thumbnail — a sibling image file next to the media (a
    "<name>.jpg" already sitting beside it) if one exists, otherwise a frame
    grabbed automatically in the background. The button at the top right of
    the playlist switches the row/thumbnail size (**Small**/**Medium**/
    **Large**); at Small and Medium, the selected row and the now-playing
    row each grow to show a bigger preview. *Preferences ▸ Playlist ▸ Misc*
    has an independent setting for each of the two ("Selected item
    thumbnail" / "Now-playing thumbnail"): no enlargement, enlarge just
    that one row, or also taper the couple of rows next to it toward the
    same size (the default for both). Generated thumbnails are cached (a
    least-recently-shown one is dropped once the cache is full); the cache
    size is configurable at *Preferences ▸ Playlist ▸ Misc ▸ Maximum
    cached thumbnails*.
- **OSD** — the on-screen display shown over the video:
  - the level of detail (subtitles only; volume + seek; volume + seek + timer)
    is being expanded; OSD duration and font size are in
    *Preferences ▸ Interface ▸ Text*.
  - Error messages (a failed download, a folder with no DVD video, an
    unreadable shortcut file, and similar) always show for a fixed 20
    seconds, regardless of the configured OSD duration — long enough to
    read even a longer error message.
