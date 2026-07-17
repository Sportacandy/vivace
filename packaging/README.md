# Packaging

Vivace is deployed with **Qt's CMake deployment API**. On **Windows** it is
packaged with **NSIS** (`makensis`, the primary/default installer, as SMPlayer
uses) or the **Qt Installer Framework** (`binarycreator`); pick with
`-Installer NSIS|IFW`. Linux and macOS default to the Qt Installer Framework
too, or skip it with `--tarball` (Linux, a plain `.tar.gz`) / `--dmg` (macOS,
via `macdeployqt`) — which is what CI/releases actually use, since IFW has no
package-manager entry on either platform and building/installing it is its
own undertaking (see the Windows notes on IFW needing a fully static build).

`CMakeLists.txt` calls `qt_generate_deploy_qml_app_script()` + `install(SCRIPT …)`,
so `cmake --install` gathers the Qt runtime, QML modules, plugins and FFmpeg for
the current platform:

- **Windows** — drives `windeployqt` (this is the tool the [Windows deployment
  guide](https://doc.qt.io/qt-6/windows-deployment.html) recommends).
- **macOS** — drives `macdeployqt`, embedding Qt frameworks in `vivace.app`
  ([macOS deployment guide](https://doc.qt.io/qt-6/macos-deployment.html)).
- **Linux** — uses CMake's own runtime-dependency scanning. **There is no
  `linuxdeployqt` in Qt 6** ([Linux deployment
  guide](https://doc.qt.io/qt-6/linux-deployment.html)).

```
packaging/
  README.md
  windows/  nsis/vivace.nsi  config/config.xml  packages/org.vivaceplayer.vivace/meta/{package.xml,installscript.qs}  build_installer.ps1
  linux/    config/config.xml  packages/org.vivaceplayer.vivace/meta/{package.xml,installscript.qs}  build_installer.sh   vivace.desktop
  macos/    config/config.xml  packages/org.vivaceplayer.vivace/meta/{package.xml,installscript.qs}  build_installer.sh
```

Generated at build time (git-ignored): each component's `data/` (deploy output)
and `meta/license.txt` (copied from the repo `LICENSE`).

## Prerequisites

- The matching Qt 6.11.x kit (MSVC 2022 on Windows, gcc_64 on Linux, macos on
  macOS).
- **NSIS** (Windows default) — install from https://nsis.sourceforge.io;
  `makensis.exe` is auto-detected under `Program Files\NSIS` (or pass `-NsisDir`).
- **Qt Installer Framework** — only for `-Installer IFW` / the Linux and macOS
  default modes (skip it with `--tarball` / `--dmg`). Add it
  with the Qt Maintenance Tool: *Qt → Developer and Designer Tools → Qt Installer
  Framework*. `binarycreator` then lives under `…/Qt/Tools/QtInstallerFramework/<ver>/bin`.
- Linux only: **patchelf** (the CMake deploy uses it to fix rpaths).

## Build

```powershell
# Windows (PowerShell) — NSIS installer (default) -> VivaceSetup-Release.exe
packaging\windows\build_installer.ps1 -QtDir C:/Qt/6.11.1/msvc2022_64
# Windows — Qt IFW installer instead -> VivaceSetup-Release-IFW.exe
packaging\windows\build_installer.ps1 -Installer IFW -QtDir C:/Qt/6.11.1/msvc2022_64
```
```bash
# Linux — IFW installer, or a .tar.gz with --tarball
QT_DIR=~/Qt/6.11.1/gcc_64 packaging/linux/build_installer.sh
QT_DIR=~/Qt/6.11.1/gcc_64 packaging/linux/build_installer.sh --tarball

# macOS — IFW installer, or a .dmg with --dmg
QT_DIR=~/Qt/6.11.1/macos packaging/macos/build_installer.sh
QT_DIR=~/Qt/6.11.1/macos packaging/macos/build_installer.sh --dmg
```

Each produces `VivaceSetup-*` / `Vivace-linux-x86_64.tar.gz` / `Vivace-macos.dmg`
at the repo root.

## Installed layout

The Qt standard deploy layout: the executable + Qt libraries in `bin/` (Windows)
or `bin/` + `lib/` (Linux), with `plugins/`, `qml/`, `translations/` alongside
and a generated `qt.conf` (`Prefix = ..`). macOS produces a self-contained
`vivace.app`. The installer shortcuts / `RunProgram` point at `bin/vivace[.exe]`
(or the `.app`).

## Install location and admin rights

The Windows installer shows an **install-scope page** ("Install only for me" vs
"Install for all users of this computer") before the target-directory page:

- **Just me** (default): target `@HomeDir@/Vivace`, no administrator rights,
  shortcuts + uninstall entry in the current user's scope (`HKCU`).
- **All users**: target `C:\Program Files\Vivace`; selecting it requests
  elevation (UAC) at that point, and shortcuts go to the all-users Start Menu.

The scope page defaults to per-user so the installer starts **without** admin
and shows no UAC prompt. IFW auto-elevates at startup only when the *default*
target needs admin, which is why `@ApplicationsDir@` (Program Files) is
deliberately not the config default — the page opts into it instead. The
target-directory page still follows, pre-filled with the chosen default and
editable.

Mechanism: `packages/…/meta/installscope.ui` (two radio buttons) is registered
via `<UserInterfaces>` in `package.xml` and inserted before
`QInstaller.TargetDirectory` by `installscript.qs` (`installer.addWizardPage`);
the radio's `toggled` handler swaps `TargetDir` and calls
`installer.gainAdminRights()` for the all-users choice.

## Verified / untested

- **Windows deploy — verified**: `cmake --install` deploys a self-contained tree
  (Qt6Multimedia + `ffmpegmediaplugin` + `av*/sw*` DLLs + styles/platform
  plugins); the installed exe plays video with no Qt on `PATH`.
- **`binarycreator`** step is **untested here** (IFW not installed on the dev
  machine). Linux and macOS scripts are **untested** (no such machines here) but
  follow the official guides.

## Reinstalling / overwriting

The Windows `config/control.qs` control script handles a reinstall by
**uninstalling the previous installation first** — it runs that install's own
`maintenancetool.exe purge` and then installs fresh into the freed directory.

**When it runs** (this timing matters — an earlier version got it wrong):
- **GUI:** only after **explicit consent**. When the target already contains an
  installation, an *"Uninstall existing version"* page (`uninstallcommit.ui`,
  added by `installscript.qs`) is shown just before the *Ready to Install*
  summary: **Next** proceeds (the uninstall then runs as the Ready page appears,
  via `ReadyForInstallationPageCallback`, safely **before** any files are
  written); **Cancel** quits the installer without touching anything. The page
  auto-skips when there is no existing installation. Because the Ready page is
  reachable only through this consent page, the uninstall never happens without
  the user agreeing — and never when the folder page is merely shown. (The first
  version purged as soon as the folder page appeared, so a Cancel there destroyed
  the existing install.)
- **CLI/unattended:** from the control-script constructor, because the CLI raises
  the hard `TargetDirectoryInUse` error unless the directory is freed before
  validation. The **GUI does not** raise that error — it allows installing into a
  directory that already contains an installation, which is why the GUI can defer
  the uninstall to the Ready page.

Other key points:
- Runs the real uninstaller (`purge`), removing the files **and** the registry
  uninstall entries. It never deletes the directory directly — an earlier
  `rmdir /s /q` version orphaned the registry entries (Vivace appeared as several
  phantom installs that could not be uninstalled).
- The maintenance tool self-relaunches from a temp copy, finishing
  asynchronously. A PowerShell wrapper starts `purge` and polls until
  `maintenancetool.exe` is gone (120 s timeout); `installer.execute` blocks on it.
- **Benign failure mode:** if purge fails/times out, `maintenancetool.exe` is
  still present and the existing install is left intact — nothing is corrupted.
- Elevation: per-user (home) purges unelevated; all-users (Program Files) runs the
  purge through IFW's elevated operation executor (the install-scope page already
  called `gainAdminRights()`).

Manual alternative (always works): uninstall via Start Menu > Vivace, or run
`maintenancetool.exe` (default install dir `%USERPROFILE%\Vivace`), then run the
installer again.

Caveat: an unattended install to a **custom** directory (`installer --root
<dir>`) can't be auto-overwritten — `--root` is applied after the control
script's constructor. Uninstall first for such installs.

**Untested here** (needs an interactive installer build over an existing
install): that the GUI reaches the Ready page without a block, `purge` runs
headless when launched this way, the poll detects completion, and the Program
Files (elevated) path works.

## Follow-ups

- **Brand icon (paused):** embed it in the exe (Windows RC) and reference a
  `.ico` / `.icns` via `<InstallerApplicationIcon>`; the Linux/macOS scripts
  already stage `icons/app_256.png` as a placeholder.
- **SMTC "unknown app" — handled.** The Windows installer stamps the Start Menu
  shortcut's `System.AppUserModel.ID` = `VivacePlayer.Vivace` (matching the
  process AUMID from main.cpp) via `set-aumid.ps1`, run from `installscript.qs`
  after `CreateShortcut`. That's what makes the SMTC media flyout show "Vivace"
  (with the shortcut/exe icon) for an installed build. IFW's `CreateShortcut`
  can't set that property itself, hence the helper.
- **Version** is hard-coded in the `config.xml` / `package.xml` files; keep it in
  sync with `project(vivace VERSION …)` when bumping.
- **Qt6DBus** is still linked on all platforms; on Windows the deploy bundles
  `Qt6DBus.dll` harmlessly. Gating it to Linux is an optional cleanup.
