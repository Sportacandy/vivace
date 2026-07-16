# Vivace

**Vivace** (pronounced *vee-VAH-cheh* — the musical tempo marking for "lively") is a fast,
pure-Qt media player: Qt Multimedia's FFmpeg backend for playback, QML for the UI.
No external player processes, no widgets.

Vivace is a ground-up successor to the ideas of SMPlayer, without the mplayer/mpv
process backends.

**[Download the latest release](https://github.com/Sportacandy/vivace/releases)**
— prebuilt Windows installer (Linux/macOS: build from source, see below).

![Vivace screenshot](screenshot.png)

## Status

**v0.1.0 — first public release.** Vivace is a working daily-driver media player:
playback (mkv/mp4/mpeg2, seeking, embedded + external subtitles, audio/subtitle
track switching, speed control with pitch compensation), a full SMPlayer-style
menu layout (Open/Play/Video/Audio/Subtitles/Browse/View/Options/Help),
playlists, favorites, bookmarks, a video equalizer, screenshots, unencrypted
DVD playback (including interactive menus), optional YouTube playback/download
(via yt-dlp), OpenSubtitles search, casting to a phone/tablet over an embedded
web server, OS media integration (Windows SMTC, Linux MPRIS2), credentials
stored securely via the OS keychain, and Windows/Linux/macOS installers. UI
translated into 24 languages, partial coverage elsewhere.

Prebuilt Windows installers: see [Releases](https://github.com/Sportacandy/vivace/releases).

## Requirements

- Qt 6.11 or later (Quick, Multimedia)
- CMake 3.24+
- A C++17 compiler (developed with MSVC 2022 on Windows)

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

Linux and macOS use the Qt Installer Framework.

Prerequisites, in addition to the build requirements above:

- **NSIS** (Windows default) — install from <https://nsis.sourceforge.io>;
  `makensis.exe` is auto-detected under `Program Files\NSIS` (or pass `-NsisDir`).
- **Qt Installer Framework** — for `-Installer IFW` and for Linux/macOS. Install
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
# Linux
QT_DIR=~/Qt/6.11.1/gcc_64 packaging/linux/build_installer.sh

# macOS — Qt Installer Framework installer, or a .dmg with --dmg
QT_DIR=~/Qt/6.11.1/macos packaging/macos/build_installer.sh
QT_DIR=~/Qt/6.11.1/macos packaging/macos/build_installer.sh --dmg
```

`-NsisDir`, `-IfwDir` (and `IFW_DIR` on Unix) are auto-detected if omitted. Each
script configures a Release build, deploys the Qt runtime + QML + plugins +
FFmpeg into the installer's staging dir, and writes `VivaceSetup-*` (or
`Vivace-macos.dmg`) to the repo root. See
[`packaging/README.md`](packaging/README.md) for the full details and follow-ups.

Keys: `Space` play/pause · `←/→` seek ±5 s · `↑/↓` volume · `M` mute · `F` fullscreen ·
`Ctrl+O` open.

## License

GPL-3.0-or-later. See [LICENSE](LICENSE).
