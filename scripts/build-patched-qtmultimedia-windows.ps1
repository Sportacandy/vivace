# Builds qtmultimedia from source (tag v6.11.1, all patches\*.patch applied)
# against an ALREADY-INSTALLED official Qt kit, in place -- overwriting that
# kit's own Qt6Multimedia.dll, Qt6MultimediaQuick.dll, and
# plugins\multimedia\ffmpegmediaplugin.dll. Also deploys the matching
# dav1d-enabled FFmpeg runtime DLLs these patched libraries link against.
#
# Building THIS WAY -- against the exact Qt kit that will actually run it,
# rather than a separately-built Qt tree whose binaries get copied over
# afterward -- is the whole point: it guarantees the patched Multimedia/
# MultimediaQuick are ABI-consistent with that same kit's own Gui/Quick/
# QuickTemplates2/..., since they are compiled against its own installed
# headers and CMake package config, not a different Qt build that merely
# shares a version NUMBER. See CLAUDE.md's 2026-08-06 entries and
# scripts/build-patched-qtmultimedia-linux.sh's header comment for the
# Linux-side story of why this approach replaced a prebuilt-asset swap.
# Windows hasn't shown the same symptom (the local Qt kit used to build the
# patches and CI's aqtinstall-fetched Qt happened to share the same
# MSVC-built official distribution channel), but there's no guarantee that
# holds forever, so this script brings Windows CI onto the same, more
# robust "build against the exact kit that will run it" approach too.
#
# Usage: build-patched-qtmultimedia-windows.ps1 -QtDir <QT_ROOT_DIR> [-RepoRoot <repo-root>]
#   QtDir:    an aqtinstall/install-qt-action (or equivalent) Qt 6.11.1
#             win64_msvc2022_64 kit that already has the qtmultimedia
#             module installed -- this REBUILDS qtmultimedia in place, it
#             does not add the module from nothing, and needs the kit's
#             dev tools (CMake package config, private headers) present.
#   RepoRoot: the Vivace checkout containing patches\*.patch (defaults to
#             this script's own parent directory's parent, i.e. the repo
#             this script lives in).

param(
    [Parameter(Mandatory = $true)][string]$QtDir,
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot ".."))
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path (Join-Path $QtDir "lib\cmake\Qt6"))) {
    throw "$QtDir\lib\cmake\Qt6 not found -- is this a Qt install with dev tools?"
}

$patchFiles = Get-ChildItem (Join-Path $RepoRoot "patches\qtmultimedia-*.patch")
if ($patchFiles.Count -eq 0) {
    throw "No patches\qtmultimedia-*.patch found under $RepoRoot"
}

$work = Join-Path $env:TEMP ("vivace-qtmm-" + [System.Guid]::NewGuid())
New-Item -ItemType Directory -Path $work | Out-Null

try {
    Write-Host "== Downloading dav1d-enabled FFmpeg (BtbN n7.1) =="
    $ffmpegZip = Join-Path $work "ffmpeg.zip"
    Invoke-WebRequest -Uri "https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-n7.1-latest-win64-gpl-shared-7.1.zip" -OutFile $ffmpegZip
    $ffmpegExtract = Join-Path $work "ffmpeg"
    Expand-Archive -Path $ffmpegZip -DestinationPath $ffmpegExtract
    # BtbN's zip extracts one level deep, e.g.
    # ffmpeg\ffmpeg-n7.1-latest-win64-gpl-shared-7.1\{bin,include,lib}.
    $ffmpegRoot = (Get-ChildItem -Path $ffmpegExtract -Directory | Select-Object -First 1).FullName
    if (-not $ffmpegRoot) {
        throw "Could not locate the extracted FFmpeg root under $ffmpegExtract"
    }

    Write-Host "== Cloning qtmultimedia v6.11.1 =="
    $qtmmSrc = Join-Path $work "qtmultimedia"
    git clone --branch v6.11.1 --depth 1 https://github.com/qt/qtmultimedia.git $qtmmSrc
    if ($LASTEXITCODE -ne 0) { throw "git clone failed" }

    Write-Host "== Applying patches =="
    foreach ($patch in $patchFiles) {
        Write-Host "  $($patch.Name)"
        git -C $qtmmSrc apply $patch.FullName
        if ($LASTEXITCODE -ne 0) { throw "git apply failed for $($patch.Name)" }
    }

    Write-Host "== Configuring + building qtmultimedia against $QtDir =="
    # Plain cmake against CMAKE_PREFIX_PATH, not qt-configure-module: the
    # latter is a convenience wrapper (qtbase's QtProcessConfigureArgs.cmake)
    # with its OWN argument parser that only forwards raw -D... flags to
    # the real cmake invocation after a literal "--" separator -- confirmed
    # by reading that script directly after CI hit "Unknown command line
    # option '-DFFMPEG_DIR=...'" (2026-08-06). Configuring qtmultimedia as
    # a plain standalone CMake project (find_package(Qt6 ...) resolves it
    # via CMAKE_PREFIX_PATH, same as any other out-of-tree Qt module build)
    # skips that wrapper and its parsing quirks entirely -- -D flags are
    # always unambiguous on a real cmake command line.
    #
    # -G Ninja (not the CMake default, the installed Visual Studio's
    # generator): Qt's own QtBuildHelpers.cmake explicitly warns "The
    # officially supported CMake generator for building Qt is Ninja /
    # Ninja Multi-Config" -- confirmed the hard way when the VS generator
    # set up RelWithDebInfo/Debug configurations (Qt's own default, not
    # Release), so `cmake --build . --config Release` failed with MSB8013
    # ("doesn't contain the Configuration and Platform combination of
    # Release|x64"). Ninja is a single-config generator, so
    # -DCMAKE_BUILD_TYPE=Release is used instead of --config.
    if (-not (Get-Command ninja.exe -ErrorAction SilentlyContinue)) {
        Write-Host "Ninja not found on PATH -- installing via choco"
        choco install ninja -y --no-progress | Out-Null
        if ($LASTEXITCODE -ne 0) { throw "choco install ninja failed" }
        $env:PATH = "$env:PATH;C:\ProgramData\chocolatey\bin"
    }

    # Ninja (unlike the VS generator, which resolves cl.exe through MSBuild's
    # own toolset settings regardless of PATH) just runs CMake's generic
    # compiler search across PATH -- and this runner also has MinGW's g++ on
    # PATH, which CMake picked up instead of MSVC, producing an install built
    # with GCC-style flags it doesn't understand (-Zc:..., -bigobj, ...)
    # against an MSVC-built Qt kit. MS C++ (cl.exe) must be the actual
    # compiler used here regardless of what else is on PATH, since the
    # installed Qt kit (win64_msvc2022_64) is itself MSVC-built and the
    # whole point of this script is ABI consistency with it. Locate the
    # Visual Studio C++ toolset via vswhere and import vcvarsall's
    # environment (PATH prepended with cl.exe's directory, INCLUDE/LIB set)
    # into this process, so CMake's compiler search finds cl.exe first.
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) {
        throw "vswhere.exe not found at $vswhere -- cannot locate the MSVC C++ toolset"
    }
    $vsInstallPath = & $vswhere -latest -prerelease -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath
    if (-not $vsInstallPath) {
        throw "vswhere found no Visual Studio install with the MSVC C++ toolset"
    }
    $vcvarsall = Join-Path $vsInstallPath "VC\Auxiliary\Build\vcvarsall.bat"
    if (-not (Test-Path $vcvarsall)) {
        throw "vcvarsall.bat not found under $vsInstallPath"
    }
    Write-Host "Importing MSVC x64 environment from $vcvarsall"
    $vcvarsOutput = cmd /c "`"$vcvarsall`" x64 && set"
    foreach ($line in $vcvarsOutput) {
        if ($line -match "^([^=]+)=(.*)$") {
            [System.Environment]::SetEnvironmentVariable($Matches[1], $Matches[2], "Process")
        }
    }
    $cl = Get-Command cl.exe -ErrorAction SilentlyContinue
    if (-not $cl) {
        throw "cl.exe still not found on PATH after importing the MSVC environment"
    }
    Write-Host "Using MSVC compiler: $($cl.Source)"

    $build = Join-Path $work "build"
    cmake -S $qtmmSrc -B $build `
        -G Ninja `
        "-DCMAKE_C_COMPILER=cl.exe" `
        "-DCMAKE_CXX_COMPILER=cl.exe" `
        "-DCMAKE_PREFIX_PATH=$QtDir" `
        "-DCMAKE_INSTALL_PREFIX=$QtDir" `
        "-DFFMPEG_DIR=$ffmpegRoot" `
        "-DQT_DEPLOY_FFMPEG=TRUE" `
        "-DCMAKE_BUILD_TYPE=Release"
    if ($LASTEXITCODE -ne 0) { throw "cmake configure failed" }

    cmake --build $build --parallel
    if ($LASTEXITCODE -ne 0) { throw "cmake --build failed" }

    cmake --install $build
    if ($LASTEXITCODE -ne 0) { throw "cmake --install failed" }

    Write-Host "== Deploying dav1d FFmpeg runtime DLLs into $QtDir\bin =="
    foreach ($dll in "avcodec-61.dll", "avformat-61.dll", "avutil-59.dll", "swresample-5.dll", "swscale-8.dll") {
        Copy-Item (Join-Path $ffmpegRoot "bin\$dll") (Join-Path $QtDir "bin\$dll") -Force
    }

    Write-Host "Done: $QtDir now has the patched qtmultimedia + dav1d FFmpeg installed."
} finally {
    Remove-Item -Recurse -Force $work -ErrorAction SilentlyContinue
}
