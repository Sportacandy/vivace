<#
    Builds a Vivace Windows installer.

    Two installer backends are supported; pick with -Installer:
      * NSIS  (default, primary) — nsis/vivace.nsi via makensis.
      * IFW              — Qt Installer Framework via binarycreator.

    Both share the same payload: the Qt-deployed tree produced by
    `cmake --install` (which runs the deploy script from
    qt_generate_deploy_qml_app_script() in CMakeLists.txt; on Windows that
    drives windeployqt — Qt libs, QML modules, plugins, FFmpeg, VC runtime —
    into the standard bin/ + plugins/ + qml/ layout).

    Prerequisites:
      * Qt 6.11.x MSVC 2022 kit.
      * NSIS (makensis) for the default backend — https://nsis.sourceforge.io
        (auto-detected under Program Files; or pass -NsisDir).
      * Qt Installer Framework (binarycreator) for -Installer IFW — install via
        the Qt Maintenance Tool (auto-detected under C:/Qt/Tools; or -IfwDir).

    Usage:
      pwsh packaging/windows/build_installer.ps1                 # NSIS (default)
      pwsh packaging/windows/build_installer.ps1 -Installer IFW
          [-QtDir C:/Qt/6.11.1/msvc2022_64] [-NsisDir ...] [-IfwDir ...]
#>
param(
    [ValidateSet("NSIS", "IFW")][string]$Installer = "NSIS",
    [string]$QtDir      = "C:/Qt/6.11.1/msvc2022_64",
    [string]$NsisDir    = "",
    [string]$IfwDir     = "",
    [string]$Config     = "Release",
    [string]$Version    = "0.1.0",  # keep in sync with project() VERSION / config.xml
    [string]$Generator  = ""        # e.g. "Visual Studio 17 2022"; empty = let CMake auto-pick
)
$ErrorActionPreference = "Stop"

# $ErrorActionPreference only catches PowerShell errors, not a native tool's
# non-zero exit — so check $LASTEXITCODE after each external command.
function Assert-LastExit([string]$what) {
    if ($LASTEXITCODE -ne 0) { throw "$what failed (exit $LASTEXITCODE)." }
}

$root  = (Resolve-Path "$PSScriptRoot/../..").Path
$build = Join-Path $root "build-release"
$pkg   = Join-Path $PSScriptRoot "packages/org.vivaceplayer.vivace"
$data  = Join-Path $pkg "data"
$env:PATH = "$QtDir/bin;$env:PATH"   # let the deploy step find windeployqt

Write-Host "== Configure + build ($Config) =="
# No -G pinned by default: this script runs both on local dev machines (VS2022
# here) and in CI (windows-latest, VS2026 as of 2026-07) -- hardcoding either
# version breaks the other. Omitting -G lets CMake auto-pick the newest
# installed Visual Studio generator; pass -Generator to force a specific one.
if ($Generator -ne "") {
    cmake -S $root -B $build -G $Generator -A x64 -DCMAKE_PREFIX_PATH=$QtDir
} else {
    cmake -S $root -B $build -A x64 -DCMAKE_PREFIX_PATH=$QtDir
}
Assert-LastExit "cmake configure"
cmake --build $build --config $Config
Assert-LastExit "cmake build"

Write-Host "== Deploy (cmake --install -> windeployqt) =="
if (Test-Path $data) { Remove-Item -Recurse -Force $data }
cmake --install $build --config $Config --prefix $data
Assert-LastExit "cmake --install (deploy)"

if ($Installer -eq "NSIS") {
    if (-not $NsisDir) {
        foreach ($p in @("${env:ProgramFiles(x86)}/NSIS", "$env:ProgramFiles/NSIS")) {
            if (Test-Path "$p/makensis.exe") { $NsisDir = $p; break }
        }
        if (-not $NsisDir) { throw "NSIS (makensis) not found. Install NSIS or pass -NsisDir." }
    }
    $out = Join-Path $root "VivaceSetup-$Config.exe"
    Write-Host "== makensis =="
    & "$NsisDir/makensis.exe" `
        "/DVERSION=$Version" `
        "/DDEPLOY=$data" `
        "/DOUTFILE=$out" `
        "/DICON=$root/icons/vivace.ico" `
        "/DLICENSE=$root/LICENSE" `
        "$PSScriptRoot/nsis/vivace.nsi"
    Assert-LastExit "makensis"
    Write-Host "Installer written to: $out"
}
else {
    # IFW stamps the AUMID via set-aumid.ps1 (NSIS does it natively instead).
    Copy-Item "$PSScriptRoot/set-aumid.ps1" "$data/set-aumid.ps1" -Force

    Write-Host "== License =="
    Copy-Item "$root/LICENSE" "$pkg/meta/license.txt" -Force

    # Brand icons for <InstallerApplicationIcon> (.ico, the installer/
    # maintenancetool exe icon) and <InstallerWindowIcon> (.png, wizard title
    # bar); base name "vivace" in the config dir. Single source: icons/.
    Write-Host "== Installer icons =="
    Copy-Item "$root/icons/vivace.ico"  "$PSScriptRoot/config/vivace.ico" -Force
    Copy-Item "$root/icons/app_256.png" "$PSScriptRoot/config/vivace.png" -Force

    if (-not $IfwDir) {
        $ifw = Get-ChildItem "C:/Qt/Tools/QtInstallerFramework" -Directory -ErrorAction SilentlyContinue |
               Sort-Object Name -Descending | Select-Object -First 1
        if (-not $ifw) { throw "Qt Installer Framework not found. Install it via the Qt Maintenance Tool or pass -IfwDir." }
        $IfwDir = Join-Path $ifw.FullName "bin"
    }

    $out = Join-Path $root "VivaceSetup-$Config-IFW.exe"
    Write-Host "== binarycreator =="
    & "$IfwDir/binarycreator.exe" -c "$PSScriptRoot/config/config.xml" `
        -p "$PSScriptRoot/packages" $out
    Assert-LastExit "binarycreator"
    Write-Host "Installer written to: $out"
}
