<#
    Regenerates the NSIS installer's brand bitmaps from the app logo, so they
    stay in sync with icons/. Run after changing the brand icon:

        pwsh packaging/windows/nsis/make-brand-images.ps1

    Produces (24-bit BMP, the sizes NSIS/MUI2 expects) next to vivace.nsi:
      vivace-wizard.bmp  164x314  Welcome/Finish page panel
      vivace-header.bmp  150x57   inner-page header (right-aligned logo)
#>
Add-Type -AssemblyName System.Drawing
$root = (Resolve-Path "$PSScriptRoot/../../..").Path
$logo = [System.Drawing.Image]::FromFile("$root/icons/app_256.png")

function New-Canvas([int]$w, [int]$h) {
    $bmp = New-Object System.Drawing.Bitmap($w, $h, [System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = 'AntiAlias'
    $g.InterpolationMode = 'HighQualityBicubic'
    $g.Clear([System.Drawing.Color]::White)
    return @($bmp, $g)
}

# Welcome/Finish panel: logo + wordmark on white.
$c = New-Canvas 164 314; $bmp = $c[0]; $g = $c[1]
$g.DrawImage($logo, 22, 48, 120, 120)
$fB = New-Object System.Drawing.Font("Segoe UI", 22, [System.Drawing.FontStyle]::Bold)
$fS = New-Object System.Drawing.Font("Segoe UI", 10)
$sf = New-Object System.Drawing.StringFormat; $sf.Alignment = 'Center'
$dark = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(40, 40, 40))
$gray = New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(120, 120, 120))
$g.DrawString("Vivace", $fB, $dark, (New-Object System.Drawing.RectangleF(0, 185, 164, 40)), $sf)
$g.DrawString("Media Player", $fS, $gray, (New-Object System.Drawing.RectangleF(0, 225, 164, 20)), $sf)
$g.Dispose(); $bmp.Save("$PSScriptRoot/vivace-wizard.bmp", [System.Drawing.Imaging.ImageFormat]::Bmp); $bmp.Dispose()

# Page header: small logo, right-aligned on white (MUI_HEADERIMAGE_RIGHT).
$c = New-Canvas 150 57; $bmp = $c[0]; $g = $c[1]
$g.DrawImage($logo, 100, 6, 44, 44)
$g.Dispose(); $bmp.Save("$PSScriptRoot/vivace-header.bmp", [System.Drawing.Imaging.ImageFormat]::Bmp); $bmp.Dispose()

$logo.Dispose()
Write-Host "Wrote vivace-wizard.bmp (164x314) and vivace-header.bmp (150x57)."
