@echo off
setlocal enabledelayedexpansion

rem Packages the patched Qt Multimedia binaries (AV1 hwaccel-disable +
rem WSOLA pitch-compensation + subtitle-bitmap-display patches applied,
rem see patches\*.patch) into qt-patched-prebuilt-win64.zip, the asset
rem release.yml/nightly.yml download and swap into an official Qt
rem install on CI (see the "Swap in the patched qtmultimedia AV1 + Speed
rem Control fix (prebuilt)" step there).
rem
rem Run this after building qtmultimedia with the patches applied and
rem installing it over C:\Qt\6.11.1\msvc2022_64 (or pass that install's
rem root as %1).

set "QTDIR=%~1"
if "%QTDIR%"=="" set "QTDIR=C:\Qt\6.11.1\msvc2022_64"
set "OUTZIP=%~dp0..\qt-patched-prebuilt-win64.zip"

if not exist "%QTDIR%\bin\Qt6Multimedia.dll" (
    echo ERROR: "%QTDIR%" does not look like a Qt install ^(Qt6Multimedia.dll not found^).
    exit /b 1
)

rem Qt6Multimedia.dll / Qt6MultimediaQuick.dll: patched by
rem qtmultimedia-subtitle-bitmap.patch (qvideoframe.cpp,
rem qplatformvideosink.cpp, qvideosink.cpp, qsgvideonode_p.cpp).
rem ffmpegmediaplugin.dll: patched by all three patches (AV1 decoder
rem selection, WSOLA pitch compensation, and the subtitle decoder fix).
rem avcodec/avformat/avutil/swresample/swscale: the dav1d-enabled FFmpeg
rem build the AV1 patch requires, bundled since Qt's own official
rem FFmpeg build doesn't include libdav1d.
set "FILES=%QTDIR%\plugins\multimedia\ffmpegmediaplugin.dll;%QTDIR%\bin\avcodec-61.dll;%QTDIR%\bin\avformat-61.dll;%QTDIR%\bin\avutil-59.dll;%QTDIR%\bin\swresample-5.dll;%QTDIR%\bin\swscale-8.dll;%QTDIR%\bin\Qt6Multimedia.dll;%QTDIR%\bin\Qt6MultimediaQuick.dll"

for %%F in ("%FILES:;=" "%") do (
    if not exist %%F (
        echo ERROR: missing file: %%F
        exit /b 1
    )
)

if exist "%OUTZIP%" del /f /q "%OUTZIP%"

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "Compress-Archive -Path ('%FILES%' -split ';') -DestinationPath '%OUTZIP%'"
if errorlevel 1 (
    echo ERROR: Compress-Archive failed.
    exit /b 1
)

echo Created "%OUTZIP%"
endlocal
