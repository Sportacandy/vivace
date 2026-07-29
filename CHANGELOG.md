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

### Added
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
