# Changelog

All notable changes to Vivace are documented here. Format loosely follows
[Keep a Changelog](https://keepachangelog.com/).

Vivace's `v0.1.1` tag has been re-cut several times without a version bump,
each time replacing the previous release's binaries in place — so unlike a
typical changelog, the `[0.1.1]` entry below has grown across those re-cuts
rather than being written once. Future releases that bump the version will
each get their own entry instead.

## [Unreleased]

### Fixed
- Selecting certain embedded subtitle tracks (image-based formats like
  DVD subtitles, PGS, or DVB — anything that isn't text) could crash
  Vivace outright, on every platform. The bug is in Qt Multimedia's
  FFmpeg plugin itself (a missing null check before reading subtitle
  text that image-based tracks never populate). Fixing it — and making
  those bitmap subtitles actually display, which Qt Multimedia's public
  API had no way to do at all before — requires a custom-built Qt; see
  "Bitmap-based embedded subtitle tracks (crash, and missing display)"
  in README.md.
- Both bitmap *and* embedded text subtitles could silently fail to
  display at all — with no crash or error, just nothing shown — whenever
  a **negative** audio delay was configured (*Preferences ▸ Audio ▸
  Global audio delay*, or a per-file delay), since Vivace's delay-
  compensation routing re-dispatches frames through a second `QVideoSink`
  and a bug in Qt Multimedia wiped out any subtitle already attached to
  the frame during that re-dispatch. Same custom-built-Qt fix as above
  (same patch file).
- **macOS:** every menu Vivace fills in while running came up empty —
  *Video/Audio/Subtitles ▸ Track* showed only their `<empty>` placeholder
  (even for the video track that was visibly playing), and *Open ▸ Recent
  files*, *Open ▸ Favorites/TV/Radio*, *Browse ▸ Titles* and *Browse ▸
  Chapters* were blank the same way. Since Qt 6.8, Qt Quick Controls builds
  the menu bar as a real macOS menu, and Qt only carries a menu item's text
  across when the menu is first created — never for items added afterwards,
  which is how all of those menus are filled in. They reached macOS with no
  text at all and drew as blank rows. Qt also can't hide an item in a native
  menu, so the placeholder stayed on screen next to them. Both are handled
  now, and the menus stay where macOS users expect them, at the top of the
  screen. Playback itself was never affected: the tracks were read correctly
  all along, they just never reached the menu.
- **macOS:** Vivace showed up in Finder with the generic "unknown application"
  icon instead of its own, even though the icon looked right in the Dock once
  the app was running. The app bundle never carried an `.icns` file or named
  one in its `Info.plist`, which is where Finder looks; the icon Vivace sets at
  startup only reaches the running app. The bundle now carries a proper icon,
  generated during the build from the same artwork the other platforms use.
- **macOS:** the installed app identified itself to the system as
  `com.yourcompany.vivace`, a placeholder CMake/Qt bakes into `Info.plist`
  when a bundle doesn't name itself. It now uses `org.vivaceplayer.vivace`,
  matching the ID the macOS installer already used. Vivace's own settings
  were never stored under the placeholder and carry over untouched.
- **macOS:** the system menu read "About vivace"/"Hide vivace"/"Quit vivace"
  (lowercase), inconsistent with the app's own "Vivace" display name
  elsewhere. Finder's name for the app is unaffected.

## [0.2.0] — 2026-08-04

### Changed
- Saving a playlist now writes just the filename (instead of a full path)
  for any entry that lives in the same folder as the playlist file itself,
  so moving or copying a playlist together with its media to another
  folder, drive, or computer no longer breaks it. Entries in a different
  folder still store their full path as before.

### Fixed
- Dropping a file onto the main window from Explorer left keyboard focus
  on Explorer instead of Vivace, so the natural next move — a volume or
  speed shortcut — didn't reach the app. Vivace now claims keyboard focus
  right after a drop.
- Pressing Enter/Return in the playlist now plays the selected row —
  previously only Up/Down moved the selection, with no way to confirm it
  from the keyboard.
- The WSOLA/phase-vocoder pitch-compensation patch (`patches/
  qtmultimedia-wsola-pitch-compensation.patch`) failed to build Qt
  Multimedia's FFmpeg plugin on Linux, since the vendored SoundTouch
  library's exception-based error handling conflicted with the plugin
  module's `-fno-exceptions` policy there. SoundTouch's sources now
  auto-detect whether exceptions are enabled and fall back to its own
  assert-based mode when they aren't, fixing the Linux build (Windows
  is unaffected) — see "Audio speed/pitch compensation" in README.md.
- Saving/moving a cached YouTube video to a folder now overwrites a
  same-named file already there, instead of adding an Explorer-style
  " (2)" suffix — for a re-save of the same video, an accumulating pile of
  numbered duplicates wasn't the intended behavior.
- "Add to an existing playlist" / "Add to current playlist" (YouTube cache
  Save) no longer add a duplicate row when the video is already in that
  playlist and already sitting at the destination folder — matching a
  file already there is now recognized as "already added", not just
  "already overwritten".
- The Favorites/Radio cascading submenus (`Open ▸ Favorites`, `Open ▸ Radio`)
  could take an ever-growing amount of time to open, then stop opening at
  all, after repeatedly opening/canceling any file dialog while a video
  played — caused by the menu's dynamically-created items being left
  eligible for JavaScript garbage collection despite still being part of
  the menu.
- The seek slider's hover preview thumbnail was unreachable on a tablet,
  since touch has no hover phase at all — it now also shows while
  pressing/dragging the slider (touch or mouse), reflecting the exact
  position the handle is being dragged to.
- Error-type OSD messages (playback errors, YouTube/download failures, a
  folder with no DVD video, an unreadable shortcut file) now stay on
  screen for a fixed 20 seconds, instead of the user's regular (often much
  shorter) OSD duration setting — long error text needs more time to read
  than a routine status message like a volume or seek notification.
- Preferences ▸ Interface: moved "Gradient background for the toolbar and
  control bar" and "Use the system native file dialog" back from the
  *Text* subtab to *Interface*, alongside the other main-window/appearance
  settings they belong with.
- The remembered window position could point at empty space if a monitor
  was unplugged or the display layout changed since the last run, leaving
  the main window impossible to find. The remembered position is now only
  restored when it still lands on a currently connected screen; the
  remembered size is kept either way.
- The YouTube cache window's "Move or copy?" dialog now shows OK before
  Cancel (matching Windows convention), and its toolbar has a visible gap
  between the Save and Remove buttons' help marks so the two aren't
  mistaken for one control.
- Open ▸ File / Directory / Playlist now default to your Videos folder the
  very first time (before any folder has been remembered), instead of an
  arbitrary OS-chosen location; Open ▸ Directory also now remembers its
  own last-used folder like the other Open dialogs already did.
- The separate Playlist window (Preferences ▸ Playlist ▸ Playlist style =
  "Separate window") now remembers its position and size across restarts,
  with the same off-screen safety check as the main window: the remembered
  size is always restored, but the position falls back to a safe default
  if it no longer lands on a currently connected screen.
- The Playlist panel/window's header now shows the loaded playlist file's
  name (e.g. "Playlist: MyList (12)") instead of the generic "Playlist
  (12)" whenever the in-memory playlist corresponds to a saved `.m3u`/
  `.m3u8` file; it falls back to the generic label once plain media (a
  folder, a DVD, a single file/stream) replaces it. Fixed several places
  that could leave a stale playlist-file association behind after such a
  replacement — Open ▸ Directory, DVD title/menu playback, opening a
  single stream, and the "auto-add files from the same folder" case — so
  the header no longer keeps showing a previously loaded playlist's name.
- The in-app Help window (`F1`) was missing entire sections in every
  translated language except English and (partially) Japanese: the
  "Installing and updating yt-dlp" and "Installing Deno" sections on the
  Options help page, and a description of playlist row thumbnails on the
  View help page. Added the missing content, translated, to all 22
  previously-covered languages, and filled the one gap that remained in
  Japanese. Also fixed a pre-existing typo in the Ukrainian translation of
  the Preferences "Misc" tab label (a Cyrillic "и" where Ukrainian "і" was
  meant), found while cross-checking the new content against it.
- The Options help page never explained what the **ffmpeg location:**
  field (*Preferences ▸ Network ▸ YouTube ▸ Download & play*) is for or
  how to install ffmpeg itself, even though it's required to merge HD
  video and audio downloads. Added a new "Installing ffmpeg" section,
  covering Windows/macOS/Linux install options, to every translated
  language. Also fixed two pre-existing terminology mismatches found
  along the way, where the Hungarian and Catalan translations of
  "Streaming" mode in that page's Deno section used words that don't
  match what the Preferences dialog itself actually says.

### Added
- Linux prebuilt releases and the nightly build now get AV1 video
  playback and the improved (WSOLA/phase-vocoder) speed/pitch
  compensation, matching Windows — CI swaps in a patched, prebuilt Qt
  Multimedia FFmpeg plugin for Linux the same way it already did for
  Windows. See "AV1 support" and "Audio speed/pitch compensation" in
  README.md; macOS still uses stock Qt behavior for both.
- Playlist row thumbnails: each entry shows a thumbnail — a sibling
  "`<name>.jpg`" next to the media file if one exists, otherwise a frame
  grabbed automatically in the background and cached (as a small SQLite
  database, not one file per thumbnail) for reuse. A button at the top
  right of the playlist switches the row/thumbnail size (Small/Medium/
  Large); at Small and Medium, the selected row and the now-playing row
  each grow toward a larger preview — independently configurable at
  Preferences ▸ Playlist ▸ Misc, per row, to no enlargement, enlarge just
  that one row, or also taper its nearest neighbours toward it, Dock-style
  "wavy" style (the default for both). Thumbnails for a newly loaded
  playlist generate in playlist order (top to bottom) rather than in
  whatever order rows happen to become visible. The generated-thumbnail
  cache size is also configurable there (Maximum cached thumbnails,
  default 20,000).
- A **Save ▾** button in the YouTube cache window (alongside the existing
  Remove button), letting you keep a downloaded video permanently instead
  of leaving it to the cache's size-limited rotation: save it to a plain
  folder, a new playlist file, an existing playlist, or straight into the
  playlist currently loaded in Vivace. Every option relocates the file (and
  its thumbnail) out of the cache — or copies it, your choice — since a
  video left referenced only inside the cache can eventually be evicted.
  Adding to the currently loaded playlist updates it live, including in a
  separate (modeless) Playlist window, and auto-saves it if it's backed by
  a file.
- Automated yt-dlp setup and updates in Preferences ▸ Network ▸ YouTube,
  replacing the old Help ▸ "Install / Update YouTube support" menu item
  (removed): a new **Use managed yt-dlp** checkbox lets Vivace install and
  maintain its own copy of yt-dlp — with a manual **Install / Update
  yt-dlp…** button, and an **Update yt-dlp automatically** option (Never /
  every time yt-dlp runs / once a day / once a week) that runs yt-dlp's own
  self-update right before a YouTube URL is played. Turning the checkbox
  off instead points Vivace at a yt-dlp you manage yourself (e.g. via
  `pip`); Vivace never installs, updates, or auto-updates that copy. A
  failed automatic update is silently ignored and playback proceeds with
  whatever version is already installed.

## [0.1.2] — 2026-07-28

### Fixed
- The last video frame staying on screen forever after playback ended with
  a negative A/V delay set, instead of the window returning to the default
  "drop a file here" placeholder.
- Long OSD messages (e.g. a YouTube download failure) overflowing past the
  right edge of the main window instead of wrapping.
- Preferences pages taller than the window, making it easy to miss content
  below the fold: *Interface* is now split into *Interface* and *Text*
  subtabs (the latter absorbing the former standalone *High DPI* subtab),
  and *Playlist*'s "Remember the playlist between sessions" moved to the
  *Misc* subtab — both now fit without scrolling. *Keyboard and mouse ▸
  Keyboard shortcuts*' list keeps its scrollbar (needed, since the full
  shortcut list is long) but the scrollbar no longer auto-hides and no
  longer overlaps the row text.

### Added
- A **Deno path** field in *Preferences ▸ Network ▸ YouTube ▸ Download &
  play*, since yt-dlp itself now depends on an external Deno runtime for
  full-quality YouTube downloads and previously assumed it was already on
  PATH with no way to point at a different location.

## [0.1.1] — 2026-07-18 (updated 2026-07-19)

### Fixed
- Critical Windows issue where AV1 video hung and leaked memory instead of
  playing, caused by a hardware-decode fault in at least some GPU/driver
  combinations — AV1 now decodes through software `libdav1d` instead
  (requires a custom-built Qt; see the "AV1 support" section in README.md).
- Media files remaining locked and undeletable/unmovable in Explorer after
  playback stopped or finished.
- Unreadable Preferences text in Windows dark mode ([#1](https://github.com/Sportacandy/vivace/issues/1)).
- Audible vibrato/echo when playing above 1x speed, and audio splitting
  into "strips" with a buzz between them below 1x speed, with pitch
  compensation enabled — now uses a WSOLA algorithm (SoundTouch) for
  speed-up and the original phase-vocoder algorithm for slow-down, since
  each holds up better in the direction the other struggles with (requires
  a custom-built Qt; see "Audio speed/pitch compensation" in README.md).
- Stale "Vivace is not translated yet" text in the About dialog's
  Translations tab, left over from before any localization existed.
- Playback speed silently resetting to 1x when dragging the seek bar,
  instead of staying at the speed the user set (matches SMPlayer's
  behavior, which keeps the set speed across a seek).

### Added
- Linux and macOS release packaging (`.tar.gz` tarball / `.dmg`), alongside
  the existing Windows installer.
- GitHub Actions CI: cross-platform builds and automatic release
  publishing on version tags.
- The Windows (NSIS) installer now registers Vivace as a file-type option
  in Windows' Default Apps / "Open with" list right after installing,
  instead of only being discoverable via Preferences > File types.

## [0.1.0] — 2026-07-16

Initial public release of Vivace, a pure-Qt media player inspired by
SMPlayer: playback (mkv/mp4/mpeg2, seeking, embedded + external subtitles,
audio/subtitle track switching, speed control with pitch compensation), a
full SMPlayer-style menu layout, playlists, favorites, bookmarks, a video
equalizer, screenshots, unencrypted DVD playback (including interactive
menus), optional YouTube playback/download, OpenSubtitles search, casting
to a phone/tablet, OS media integration (Windows SMTC, Linux MPRIS2), and a
Windows installer.
