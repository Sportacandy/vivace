# Vivace

**Vivace** (pronounced *vee-VAH-cheh* — the musical tempo marking for "lively") is a fast,
pure-Qt media player: Qt Multimedia's FFmpeg backend for playback, QML for the UI.
No external player processes, no widgets.

Vivace is a ground-up successor to the ideas of SMPlayer, without the mplayer/mpv
process backends.

**[Download the latest release](https://github.com/Sportacandy/vivace/releases)**
— Windows installer, Linux tarball, and macOS `.dmg`. A rolling
[nightly build](https://github.com/Sportacandy/vivace/releases/tag/nightly)
is also published from the tip of `main` between tagged releases. See
[CHANGELOG.md](CHANGELOG.md) for what's changed in each release.

![Vivace screenshot](screenshot.png)

## Status

**v0.3.0** — macOS gets full parity with Windows/Linux: native menus
(Track, Recent files, Favorites/TV/Radio, DVD Titles, Chapters) that were
rendering blank now fill in correctly, the app carries its own icon and
identifies itself properly instead of a generic placeholder, and the
system menu bar reads "Vivace" instead of "vivace". The whole UI now
follows the system's light/dark color scheme (menu bar, list/table
dialogs, and the Keyboard-and-mouse shortcuts page). Fixed a bug where
reopening a file could leave the Audio/Subtitles Track menu showing the
wrong checkmark even though the right track was actually playing, and
fixed a crash (plus, on a custom-built Qt, actual on-screen display) for
image-based embedded subtitle tracks like DVD/PGS/DVB subs. Vivace is a
working daily-driver media player:
playback (mkv/mp4/mpeg2, seeking, embedded + external subtitles, audio/subtitle
track switching, speed control with pitch compensation), a full SMPlayer-style
menu layout (Open/Play/Video/Audio/Subtitles/Browse/View/Options/Help),
playlists, favorites, bookmarks, a video equalizer, screenshots, unencrypted
DVD playback (including interactive menus), optional YouTube playback/download
(via yt-dlp), OpenSubtitles search, casting to a phone/tablet over an embedded
web server, OS media integration (Windows SMTC, Linux MPRIS2), credentials
stored securely via the OS keychain, and Windows/Linux/macOS installers (the
Windows installer also registers Vivace as a file-type option in Windows'
Default Apps, so it appears as a candidate for opening media files right
after installing). UI translated into 24 languages, partial coverage
elsewhere.

Prebuilt packages: see [Releases](https://github.com/Sportacandy/vivace/releases) —
a Windows NSIS installer, a Linux `.tar.gz` (x86_64 and arm64 — the
latter for Raspberry Pi OS 64-bit and other arm64 desktop Linux), and a
macOS `.dmg`, plus a rolling nightly build from `main`.

## Requirements

To build from source:

- Qt 6.11 or later (Quick, Multimedia)
- CMake 3.24+
- A C++17 compiler (developed with MSVC 2022 on Windows)

To run the prebuilt **Linux** `.tar.gz`, `libxcb-cursor0` (some
distributions call it `xcb-cursor0`) may need to be installed, or Vivace
exits immediately with:

```
qt.qpa.plugin: From 6.5.0, xcb-cursor0 or libxcb-cursor0 is needed to load the Qt xcb platform plugin.
qt.qpa.plugin: Could not load the Qt platform plugin "xcb" in "" even though it was found.
```

This is a well-known Qt 6.5+ packaging gotcha, not a Vivace-specific bug:
the xcb platform plugin loads it at runtime rather than linking it
directly, so it can't be detected or bundled by the dependency-scanning
deploy step the Linux package is built with (there is no `linuxdeployqt`
for Qt 6 — see `packaging/linux/build_installer.sh`). It's a one-time
system install (`sudo apt install libxcb-cursor0` on Debian/Ubuntu, or
the equivalent package elsewhere).

## AV1 support

**Windows and Linux** prebuilt releases and the [nightly build](https://github.com/Sportacandy/vivace/releases/tag/nightly)
play AV1 video: CI swaps in a patched Qt Multimedia FFmpeg plugin plus a
`libdav1d`-enabled FFmpeg build right after installing Qt. This same swap
also carries the improved audio speed/pitch compensation described below,
since both fixes now ship in one combined bundle per platform. **macOS**
prebuilt releases, and Vivace built normally against a stock Qt on any
platform, get **neither**: AV1 files show "Unsupported media, a codec is
missing" and play audio only, and speed control falls back to Qt's stock
pitch-compensation behavior.

This isn't a Vivace-specific limitation: Qt's own official FFmpeg build
ships without `libdav1d`/AV1 decode support at all. Investigation found a
likely reason: AV1's *hardware*-accelerated decode path (e.g. D3D11VA on
Windows) hangs and leaks memory rather than failing cleanly on at least
some GPU/driver combinations, rather than this being a licensing choice —
enabling AV1 without very careful hwaccel-failure handling is a real
stability risk across the huge range of real-world hardware.

To get AV1 support elsewhere (Linux/macOS, or a Vivace you build yourself):

**Windows/Linux quick path**: run
[`scripts/build-patched-qtmultimedia-windows.ps1`](scripts/build-patched-qtmultimedia-windows.ps1) /
[`scripts/build-patched-qtmultimedia-linux.sh`](scripts/build-patched-qtmultimedia-linux.sh)
against your own Qt 6.11.1 install (`msvc2022_64` / `gcc_64`) — the same
scripts CI itself now uses. Each one downloads a dav1d-enabled FFmpeg,
clones `qtmultimedia` at the matching tag, applies all three patches
(AV1, speed/pitch compensation, bitmap subtitles), and builds + installs
it *against that exact Qt kit*, then deploys the matching FFmpeg runtime
libraries alongside it. Building against the same kit you'll actually run
is deliberate, not incidental: `Qt6Multimedia`/`Qt6MultimediaQuick` call
into `Qt6Gui`/`Qt6Quick` through Qt's *private* (no ABI-stability
guarantee) API, and an earlier prebuilt-binary-asset approach (built on
one machine, copied onto another) hit exactly that — a private struct
layout drift that a mere version-number match couldn't catch, crashing
some video with no code-level bug on either side. Building against your
own installed kit removes that risk entirely, since there's only ever one
build involved. No separate rebuild step needed afterward; just rebuild
Vivace itself against that same Qt install once the script finishes.

## Audio speed/pitch compensation

Vivace's speed control (`Play > Speed`) can preserve pitch when playing
faster or slower than 1x ("pitch compensation" in Preferences). With a
**stock Qt**, this uses Qt's built-in phase-vocoder algorithm for every
speed, which sounds clean when slowing down but produces audible vibrato/
echo when speeding up (e.g. 2x) on speech-heavy content.

**Windows and Linux** prebuilt releases and the nightly build get the fix
below via the same CI-built `qtmultimedia` described in "AV1 support"
above. **macOS** prebuilt releases, and Vivace built normally against a
stock Qt on any platform, use Qt's stock behavior — there is no
artifact-free option without a custom Qt build, for the same reason as
AV1: the fix requires source changes to `qtmultimedia` itself, not just
Vivace.

Investigation found the phase vocoder and the alternative WSOLA (time-
domain) approach have exactly opposite strengths: the phase vocoder holds
up well slowing down but degrades speeding up (denser frame overlap makes
phase-coherence harder), while WSOLA is clean speeding up but introduces a
buzz/seam artifact slowing down (it has to repeat audio to fill time,
without pitch-synchronized splicing). So the fix uses each algorithm only
in the direction it's strong: WSOLA (via the vendored
[SoundTouch](https://codeberg.org/soundtouch/soundtouch) library) above 1x,
Qt's original phase vocoder below 1x.

The `build-patched-qtmultimedia-*` scripts under "AV1 support" above apply
this patch too (along with the other two), so that's the simplest way to
get it. To apply just this one patch by hand instead: apply
[`patches/qtmultimedia-wsola-pitch-compensation.patch`](patches/qtmultimedia-wsola-pitch-compensation.patch)
to your `qtmultimedia` source checkout (vendors SoundTouch alongside Qt's
existing Signalsmith Stretch phase vocoder, then picks per playback
direction), rebuild `qtmultimedia`, and rebuild Vivace against that Qt.
(This patch is independent of the AV1 one above and can be applied to the
same checkout either alongside it or on its own.)

The vendored SoundTouch sources build on any platform/toolchain: they
auto-detect (via the standard `__cpp_exceptions` feature-test macro)
whether the surrounding build has C++ exceptions enabled, and fall back to
SoundTouch's own built-in `ST_NO_EXCEPTION_HANDLING` mode when it doesn't —
needed on Linux/GCC, where Qt Multimedia's FFmpeg plugin module is built
with `-fno-exceptions`, which would otherwise fail to compile SoundTouch's
normal `throw`-based error path.

## Bitmap-based embedded subtitle tracks (crash, and missing display)

Selecting certain embedded subtitle tracks from *Subtitles ▸ Track* can
crash Vivace outright, and even once that's fixed, Qt Multimedia has no
way to actually *show* that kind of subtitle. This affects **every**
build — Windows, Linux, macOS, custom or official — because the bug and
the missing feature are both inside Qt Multimedia's FFmpeg plugin itself,
not Vivace: some subtitle formats (DVD subtitles, PGS, DVB — anything
*image*-based rather than text) don't carry any text data, only an
already-decoded bitmap. Qt's subtitle decoder dereferences a null pointer
instead of checking for this before reading it (the crash), and even past
that, Qt's `QVideoSink`/`QVideoFrame` subtitle pipeline only ever carries
*text* — there was no API to deliver a bitmap image at all. There is no
way for Vivace to detect or avoid this ahead of time, since Qt's public
API doesn't expose a subtitle track's underlying codec.

This same patch also fixes a second, independent bug that affects **plain
text subtitles too** (embedded `mov_text`/SRT-style tracks, not just
bitmap ones): if you have a **negative** audio delay configured for your
output device (*Preferences ▸ Audio ▸ Global audio delay*, or a per-file
delay under *Audio ▸ Set delay…* — used e.g. to compensate for late
Bluetooth audio), Vivace routes video frames through an internal delay
buffer before displaying them, and a bug in Qt Multimedia's own
`QPlatformVideoSink` caused that re-routing to silently wipe out any
subtitle (bitmap *or* text) already attached to the frame. With no
negative delay configured, this never showed up — which is why it went
unnoticed until the bitmap-subtitle investigation below turned it up.

To fix all of this — the crash, the missing bitmap display, and the
delayed-audio subtitle-dropping bug — apply
[`patches/qtmultimedia-subtitle-bitmap.patch`](patches/qtmultimedia-subtitle-bitmap.patch)
to your `qtmultimedia` source checkout, rebuild `qtmultimedia`, and
rebuild Vivace against that Qt (or use the **Windows/Linux quick path**
scripts under "AV1 support" above — they apply this fix too, alongside
AV1 and speed/pitch compensation, in the same run). It extends Qt's
existing subtitle pipeline
(decoder → renderer → sink → video frame → Qt Quick scene graph) with a
parallel bitmap-image path alongside the existing text path, so a bitmap
subtitle is decoded into an image, carried through the same frames as the
text subtitle would be, and composited onto the video the same way Qt
already composites text subtitles — no Vivace-side code is needed or
included. Independent of the AV1 and speed/pitch-compensation patches
above — apply any combination of these to the same checkout.

Without this patch, avoid selecting a subtitle track that turns out to be
image-based; Vivace has no way to warn you before you select it, since it
can't see the codec either. And if you rely on a negative audio delay,
text subtitles will silently not appear either, on any platform.

**Verified working end to end on both Windows and Linux** (2026-08-06):
selecting a bitmap-type subtitle track no longer crashes Vivace, and the
subtitle actually renders — correctly positioned, updates line to line as
playback continues, and stays correctly scaled after resizing/maximizing
the window mid-playback — in all four combinations of {bitmap, text
(`mov_text`)} × {no delay configured, a negative per-device delay
configured}.

## Building

```
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:/Qt/6.11.1/msvc2022_64
cmake --build build --config Release
```

Or open `CMakeLists.txt` in Qt Creator and hit Run.

Usage: `vivace [file-or-url]`, or drag & drop a file onto the window.

## Packaging (installers)

Vivace deploys with Qt's CMake deployment API (`cmake --install`, which drives
`windeployqt` on Windows and `macdeployqt` on macOS, and uses CMake's own
dependency scanning on Linux — there is no `linuxdeployqt` in Qt 6). On
**Windows** you choose the installer backend with `-Installer`:

- **`NSIS`** (default) — `nsis/vivace.nsi` via `makensis`; the primary installer.
- **`IFW`** — the **Qt Installer Framework** (`binarycreator`).

macOS defaults to the Qt Installer Framework too, or pass `--dmg` for a plain
`.dmg` (via `macdeployqt`, no IFW needed — this is what CI/releases use).
Linux defaults to the Qt Installer Framework, or pass `--tarball` for a plain
`.tar.gz` of the deployed tree (no IFW needed — also what CI/releases use,
since IFW has no package manager entry and building/installing it is its own
undertaking; a tarball is enough for this stage).

Prerequisites, in addition to the build requirements above:

- **NSIS** (Windows default) — install from <https://nsis.sourceforge.io>;
  `makensis.exe` is auto-detected under `Program Files\NSIS` (or pass `-NsisDir`).
- **Qt Installer Framework** — only needed for `-Installer IFW` / the Linux
  and macOS default modes (skip it with `--tarball` / `--dmg`). Install
  a prebuilt copy with the Qt Maintenance Tool (*Qt → Developer and Designer
  Tools → Qt Installer Framework*), or build it from source for single-file
  installers — see [`packaging/README.md`](packaging/README.md). It must be a
  **fully static** build (verify `dumpbin /dependents binarycreator.exe` shows
  only system DLLs).
- Linux only: `patchelf` (used by the CMake deploy to fix rpaths).

Build the installer — pick the backend you want:

```powershell
# Windows (PowerShell) — NSIS (default)          -> VivaceSetup-Release.exe
packaging\windows\build_installer.ps1 -QtDir C:/Qt/6.11.1/msvc2022_64

# Windows — Qt Installer Framework instead        -> VivaceSetup-Release-IFW.exe
packaging\windows\build_installer.ps1 -Installer IFW -QtDir C:/Qt/6.11.1/msvc2022_64 `
    -IfwDir C:/Qt/Tools/QtInstallerFramework/<ver>/bin
```
```bash
# Linux — Qt Installer Framework installer, or a .tar.gz with --tarball
QT_DIR=~/Qt/6.11.1/gcc_64 packaging/linux/build_installer.sh
QT_DIR=~/Qt/6.11.1/gcc_64 packaging/linux/build_installer.sh --tarball

# macOS — Qt Installer Framework installer, or a .dmg with --dmg
QT_DIR=~/Qt/6.11.1/macos packaging/macos/build_installer.sh
QT_DIR=~/Qt/6.11.1/macos packaging/macos/build_installer.sh --dmg
```

`-NsisDir`, `-IfwDir` (and `IFW_DIR` on Unix) are auto-detected if omitted. Each
script configures a Release build, deploys the Qt runtime + QML + plugins +
FFmpeg into the installer's staging dir, and writes `VivaceSetup-*` /
`Vivace-linux-$(uname -m).tar.gz` (`x86_64` or `aarch64`, matching the
machine it's built on) / `Vivace-macos.dmg` to the repo root. See
[`packaging/README.md`](packaging/README.md) for the full details and follow-ups.

Keys: `Space` play/pause · `←/→` seek ±5 s · `↑/↓` volume · `M` mute · `F` fullscreen ·
`Ctrl+O` open.

## License

GPL-3.0-or-later. See [LICENSE](LICENSE).
