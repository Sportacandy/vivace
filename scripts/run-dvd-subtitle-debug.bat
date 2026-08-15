@echo off
setlocal enabledelayedexpansion
rem Launch Vivace against a DVD folder with the DVD subtitle-track debug
rem tracing enabled, for tracking down "the first subtitle line displays
rem but following ones don't" (Bug 2 follow-up, 2026-08-15).
rem
rem Plain .bat, run directly by cmd.exe -- no Git Bash/MSYS involved, so
rem none of the MSYS DLL-loader / path-mangling issues apply here.
rem
rem Usage:
rem   scripts\run-dvd-subtitle-debug.bat ["path\to\folder containing VIDEO_TS"]
rem
rem With no argument, this looks under media\ for a subfolder containing
rem VIDEO_TS; if there is more than one (there are several test discs in
rem this project's media\ folder), it lists them and asks you to pass one
rem explicitly instead of guessing.
rem
rem Sets:
rem   VIVACE_DVD_LOG=1      -- trace DVD/menu/subtitle events to
rem                            %%TEMP%%\vivace_dvd.log (see playercontroller.cpp's
rem                            dvdLog()).
rem   VIVACE_DVD_NOMENU=1   -- skip disc-menu navigation and jump straight
rem                            into the main (longest) title. Set
rem                            SKIP_MENU=0 in the environment first to go
rem                            through the real menu instead.
rem
rem After reproducing (e.g. scrub the seek bar to a few spread-out points --
rem ~5/30/60/90/120 min -- and note where the subtitle text appears), close
rem Vivace; the trace log path is printed below before launch.

set "SCRIPT_DIR=%~dp0"
set "REPO_ROOT=%SCRIPT_DIR%.."
set "EXE=%REPO_ROOT%\build\RelWithDebInfo\vivace.exe"

if not exist "%EXE%" (
    echo vivace.exe not found at: %EXE%
    echo Build it first: cmake --build build --config RelWithDebInfo --target vivace
    exit /b 1
)

set "DVD_DIR=%~1"
if "%DVD_DIR%"=="" (
    set "MATCH_COUNT=0"
    set "DVD_DIR="
    for /d %%D in ("%REPO_ROOT%\media\*") do (
        if exist "%%D\VIDEO_TS" (
            set /a MATCH_COUNT+=1
            set "DVD_DIR=%%D"
            echo Found DVD folder: %%D
        )
    )
    if not "!MATCH_COUNT!"=="1" (
        echo.
        echo No folder given, and found !MATCH_COUNT! DVD folders under media\ ^(need exactly 1 to auto-pick^).
        echo Usage: %~nx0 "path\to\folder containing VIDEO_TS"
        exit /b 1
    )
)

if not exist "%DVD_DIR%\VIDEO_TS" (
    echo Not a DVD folder ^(no VIDEO_TS subfolder found^): %DVD_DIR%
    exit /b 1
)

set "VIVACE_DVD_LOG=1"
if not defined SKIP_MENU set "SKIP_MENU=1"
set "VIVACE_DVD_NOMENU=%SKIP_MENU%"

echo.
echo Launching: %EXE%
echo   DVD dir:           %DVD_DIR%
echo   VIVACE_DVD_LOG:    %VIVACE_DVD_LOG%
echo   VIVACE_DVD_NOMENU: %VIVACE_DVD_NOMENU%
echo.
echo Trace log will be appended to: %TEMP%\vivace_dvd.log
echo (existing content is kept -- old runs stay in the file too)
echo.

"%EXE%" "%DVD_DIR%"

endlocal
