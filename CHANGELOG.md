# Changelog

All notable changes to Vivace are documented here. Format loosely follows
[Keep a Changelog](https://keepachangelog.com/).

Vivace's `v0.1.1` tag has been re-cut several times without a version bump,
each time replacing the previous release's binaries in place — so unlike a
typical changelog, the `[0.1.1]` entry below has grown across those re-cuts
rather than being written once. Future releases that bump the version will
each get their own entry instead.

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
