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
#             dev tools (qt-configure-module.bat, private headers) present.
#   RepoRoot: the Vivace checkout containing patches\*.patch (defaults to
#             this script's own parent directory's parent, i.e. the repo
#             this script lives in).

param(
    [Parameter(Mandatory = $true)][string]$QtDir,
    [string]$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot ".."))
)

$ErrorActionPreference = "Stop"

$qtConfigureModule = Join-Path $QtDir "bin\qt-configure-module.bat"
if (-not (Test-Path $qtConfigureModule)) {
    throw "qt-configure-module.bat not found under $QtDir\bin -- is this a Qt install with qtbase dev tools?"
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
    $build = Join-Path $work "build"
    New-Item -ItemType Directory -Path $build | Out-Null
    Push-Location $build
    try {
        & $qtConfigureModule $qtmmSrc `
            "-DFFMPEG_DIR=$ffmpegRoot" `
            "-DQT_DEPLOY_FFMPEG=TRUE" `
            "-DCMAKE_BUILD_TYPE=Release"
        if ($LASTEXITCODE -ne 0) { throw "qt-configure-module failed" }

        cmake --build . --config Release --parallel
        if ($LASTEXITCODE -ne 0) { throw "cmake --build failed" }

        cmake --install . --config Release
        if ($LASTEXITCODE -ne 0) { throw "cmake --install failed" }
    } finally {
        Pop-Location
    }

    Write-Host "== Deploying dav1d FFmpeg runtime DLLs into $QtDir\bin =="
    foreach ($dll in "avcodec-61.dll", "avformat-61.dll", "avutil-59.dll", "swresample-5.dll", "swscale-8.dll") {
        Copy-Item (Join-Path $ffmpegRoot "bin\$dll") (Join-Path $QtDir "bin\$dll") -Force
    }

    Write-Host "Done: $QtDir now has the patched qtmultimedia + dav1d FFmpeg installed."
} finally {
    Remove-Item -Recurse -Force $work -ErrorAction SilentlyContinue
}
