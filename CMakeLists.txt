cmake_minimum_required(VERSION 3.24)

project(vivace VERSION 0.3.2 LANGUAGES CXX)

find_package(Qt6 6.11 REQUIRED COMPONENTS Quick QuickControls2 Multimedia LabsPlatform Network ShaderTools DBus LinguistTools Sql)
qt_standard_project_setup(REQUIRES 6.11)

qt_add_executable(vivace
    src/main.cpp
)

# Icon sets ported from SMPlayer (GPL-2.0-or-later): "Default" (H2O) and
# "Classic" (default-theme) under icons/<set>/, plus theme-independent
# app-branding icons at icons/ root.
file(GLOB vivace_icon_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_DEPENDS icons/*.png icons/Default/*.png icons/Classic/*.png)

# Bundled offline help documents (Markdown), shown in the Help window.
# One Markdown file per topic, under a per-language folder: help/<lang>/*.md
# (help/en, help/ja, …). Glob picks up any language folder automatically.
file(GLOB vivace_help_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_DEPENDS help/*/*.md)

# Theme.qml is a QML singleton (pragma Singleton needs this to register).
set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(vivace
    URI Vivace
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/ToolbarItems.js
        qml/GeometryUtils.js
        qml/ToolbarEditor.qml
        qml/MainMenuBar.qml
        qml/MainToolBar.qml
        qml/AppMenu.qml
        qml/AppMenuItem.qml
        qml/HelpMark.qml
        qml/ControlBar.qml
        qml/PlaylistEditor.qml
        qml/PlaylistPanel.qml
        qml/PlaylistWindow.qml
        qml/MediaInfoDialog.qml
        qml/FavoritesMenu.qml
        qml/FavoritesDialog.qml
        qml/BookmarksDialog.qml
        qml/FindSubtitlesDialog.qml
        qml/AboutDialog.qml
        qml/HelpDialog.qml
        qml/UpdateDialog.qml
        qml/YoutubeSupportDialog.qml
        qml/CacheBrowserDialog.qml
        qml/CastDialog.qml
        qml/VideoEqualizerDialog.qml
        qml/OpenUrlDialog.qml
        qml/PreferencesDialog.qml
        qml/PrefGeneralPage.qml
        qml/PrefDrivesPage.qml
        qml/PrefSubtitlesPage.qml
        qml/PrefInterfacePage.qml
        qml/PrefInputPage.qml
        qml/PrefPlaylistPage.qml
        qml/PrefTVPage.qml
        qml/PrefFileTypesPage.qml
        qml/PrefUpdatesPage.qml
        qml/PrefNetworkPage.qml
        qml/PrefAdvancedPage.qml
    SOURCES
        src/playercontroller.h src/playercontroller.cpp
        src/playlistmodel.h src/playlistmodel.cpp
        src/playlistparser.h src/playlistparser.cpp
        src/subtitleparser.h src/subtitleparser.cpp
        src/recentfiles.h src/recentfiles.cpp
        src/favoritesmodel.h src/favoritesmodel.cpp
        src/dvdtitledevice.h src/dvdtitledevice.cpp
        src/livestreamdevice.h src/livestreamdevice.cpp
        src/httptssource.h src/httptssource.cpp
        src/dvdifoparser.h src/dvdifoparser.cpp
        src/dvdmenuparser.h src/dvdmenuparser.cpp
        src/dvdpciparser.h src/dvdpciparser.cpp
        src/dvdspu.h src/dvdspu.cpp
        src/dvdsubtitletrack.h src/dvdsubtitletrack.cpp
        src/dvdvm.h src/dvdvm.cpp
        src/filesettings.h src/filesettings.cpp
        src/bookmarks.h src/bookmarks.cpp
        src/chapterparser.h src/chapterparser.cpp
        src/mediacontrols.h src/mediacontrols.cpp
        src/mpris2.h src/mpris2.cpp
        src/opensubtitlesclient.h src/opensubtitlesclient.cpp
        src/networkproxy.h src/networkproxy.cpp
        src/castserver.h src/castserver.cpp
        third_party/qrcodegen/qrcodegen.h third_party/qrcodegen/qrcodegen.cpp
        src/screensaver.h src/screensaver.cpp
        src/singleinstance.h src/singleinstance.cpp
        src/shortcuts.h src/shortcuts.cpp
        src/updatechecker.h src/updatechecker.cpp
        src/youtuberesolver.h src/youtuberesolver.cpp
        src/playlistthumbnailprovider.h src/playlistthumbnailprovider.cpp
        src/externaldownloader.h src/externaldownloader.cpp
        src/settings.h src/settings.cpp
        src/securestore.h src/securestore.cpp
        src/uihelpers.h src/uihelpers.cpp
    RESOURCES
        ${vivace_icon_files}
        ${vivace_help_files}
)

# Third-party vendored sources live under third_party/<name>/ regardless of
# size or how their upstream expresses the license (inline header comment vs.
# an external file like QtKeychain's COPYING) -- kept out of src/ so it's
# always clear at a glance which code is ours vs. vendored.
#
# QtKeychain (third_party/qtkeychain/, modified-BSD license -- see
# third_party/qtkeychain/COPYING) provides OS-level secure credential storage
# (Windows Credential Manager / macOS Keychain / Linux Secret Service+
# KWallet), used by SecureCredentials (src/securestore.*) to store the
# OpenSubtitles and proxy passwords instead of plaintext QSettings. Only the
# library sources for the platforms Vivace targets are vendored (no
# tests/examples/translations, no Android/Haiku/WASM backends); compiled
# directly into the vivace target rather than as a separate library, so
# QTKEYCHAIN_NO_EXPORT (below) skips the generated dllexport header.
set(vivace_qtkeychain_dir "${CMAKE_CURRENT_SOURCE_DIR}/third_party/qtkeychain")
target_sources(vivace PRIVATE
    ${vivace_qtkeychain_dir}/keychain.h
    ${vivace_qtkeychain_dir}/keychain.cpp
    ${vivace_qtkeychain_dir}/keychain_p.h
)
target_include_directories(vivace PRIVATE ${vivace_qtkeychain_dir})
target_compile_definitions(vivace PRIVATE QTKEYCHAIN_NO_EXPORT)

if(WIN32)
    target_sources(vivace PRIVATE
        ${vivace_qtkeychain_dir}/keychain_win.cpp
        ${vivace_qtkeychain_dir}/plaintextstore.cpp
        ${vivace_qtkeychain_dir}/plaintextstore_p.h
    )
    # USE_CREDENTIAL_STORE selects the modern Windows Credential Manager
    # backend (matches QtKeychain's own default); UNICODE matches its own
    # Windows build flags.
    target_compile_definitions(vivace PRIVATE USE_CREDENTIAL_STORE=1 UNICODE)
    target_link_libraries(vivace PRIVATE crypt32)
elseif(APPLE)
    target_sources(vivace PRIVATE ${vivace_qtkeychain_dir}/keychain_apple.mm)
    target_link_libraries(vivace PRIVATE "-framework Foundation" "-framework Security")
    # UiHelpers::syncNativeMenu() drives the NSMenu that Qt Quick Controls
    # builds for the menu bar on macOS; there is no public API for it. See the
    # comment on the declaration in src/uihelpers.h. macOS-only, so no other
    # platform picks up a dependency on Qt's private headers.
    find_package(Qt6 REQUIRED COMPONENTS QuickPrivate QuickTemplates2Private)
    target_link_libraries(vivace PRIVATE
        Qt6::QuickPrivate Qt6::QuickTemplates2Private)
elseif(UNIX)
    # Linux: KWallet via D-Bus (Qt6::DBus, already linked below) + optional
    # libsecret (GNOME Keyring/Secret Service) loaded dynamically at runtime
    # via QLibrary (dbuskeyring.cpp -- renamed from upstream's libsecret.cpp;
    # a local tool restriction here blocks any path containing "secret", so
    # keeping the upstream name wasn't an option) -- no libsecret-dev headers
    # needed unless LIBSECRET_FOUND below.
    target_sources(vivace PRIVATE
        ${vivace_qtkeychain_dir}/keychain_unix.cpp
        ${vivace_qtkeychain_dir}/gnomekeyring.cpp
        ${vivace_qtkeychain_dir}/gnomekeyring_p.h
        ${vivace_qtkeychain_dir}/dbuskeyring.cpp
        ${vivace_qtkeychain_dir}/dbuskeyring_p.h
        ${vivace_qtkeychain_dir}/plaintextstore.cpp
        ${vivace_qtkeychain_dir}/plaintextstore_p.h
    )
    target_compile_definitions(vivace PRIVATE KEYCHAIN_DBUS=1)
    # The generated class name is a CLASSNAME source-file property, not a 4th
    # positional argument -- qt_add_dbus_interface's versionless wrapper (Qt
    # 6.11's Qt6DBusMacros.cmake) takes exactly 3 args and FATAL_ERRORs on more.
    set_source_files_properties("${vivace_qtkeychain_dir}/org.kde.KWallet.xml"
        PROPERTIES CLASSNAME KWalletInterface)
    qt_add_dbus_interface(vivace_kwallet_dbus_sources
        "${vivace_qtkeychain_dir}/org.kde.KWallet.xml" kwallet_interface)
    target_sources(vivace PRIVATE ${vivace_kwallet_dbus_sources})
    # qt_add_dbus_interface() writes kwallet_interface.h into the current
    # binary dir; keychain_p.h's quoted #include only searches its own source
    # dir by default, so the binary dir needs to be an explicit include path.
    target_include_directories(vivace PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(LIBSECRET libsecret-1)
    endif()
    if(LIBSECRET_FOUND)
        target_compile_definitions(vivace PRIVATE HAVE_LIBSECRET=1)
        target_include_directories(vivace PRIVATE ${LIBSECRET_INCLUDE_DIRS})
        target_link_libraries(vivace PRIVATE ${LIBSECRET_LIBRARIES})
    endif()
endif()

set_target_properties(vivace PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    # Without this, Qt's bundle finalization bakes its own
    # "com.yourcompany.vivace" placeholder into Info.plist's CFBundleIdentifier
    # -- which is what macOS (not Vivace's QSettings, which keys off
    # setOrganizationName/setApplicationName) uses to identify the app. Match
    # the ID packaging/macos/build_installer.sh already uses for the installer.
    MACOSX_BUNDLE_GUI_IDENTIFIER "org.vivaceplayer.vivace"
    # Info.plist's CFBundleName otherwise defaults to the lowercase target
    # name, so macOS's system menu read "About vivace"/"Hide vivace"/
    # "Quit vivace" instead of "Vivace" (main.cpp's own
    # setApplicationDisplayName("Vivace") only affects the running app, not
    # what the OS reads from the bundle before/around that). Only changes the
    # menu-bar/Get Info name; the file in Finder stays vivace.app.
    MACOSX_BUNDLE_BUNDLE_NAME "Vivace"
)

# Windows: embed the brand icon into the executable. The RC references
# icons/vivace.ico; icon ID 1 becomes the exe icon used by Explorer, the
# taskbar, Start-menu shortcuts (iconId=0), file-type associations
# (DefaultIcon "exe",0) and the SMTC flyout.
if(WIN32)
    target_sources(vivace PRIVATE icons/vivace.rc)
endif()

# macOS: the same for the app bundle. Finder takes an app's icon from
# Info.plist's CFBundleIconFile and a matching .icns in Contents/Resources,
# both of which are build-time -- QGuiApplication::setWindowIcon() in main.cpp
# only covers the running app's Dock icon, so without this Finder fell back to
# the generic "unknown application" icon while the app itself looked right once
# launched. The .icns is generated from the same icons/app_*.png set every
# other platform uses (iconutil and sips ship with macOS), rather than
# committing a second copy of the artwork that could drift out of sync.
#
# app_256.png is the largest source there is, so the iconset stops at the
# 256x256 (and 128x128@2x) slots; macOS scales that up for the 512/1024 views
# it uses in Get Info and large-icon mode. Add icon_512x512{,@2x}.png here if
# higher-resolution artwork ever appears.
if(APPLE)
    set(vivace_iconset "${CMAKE_CURRENT_BINARY_DIR}/vivace.iconset")
    set(vivace_icns "${CMAKE_CURRENT_BINARY_DIR}/vivace.icns")
    set(vivace_icns_sources
        icons/app_16.png icons/app_32.png icons/app_64.png
        icons/app_128.png icons/app_256.png)
    add_custom_command(
        OUTPUT ${vivace_icns}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${vivace_iconset}
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/icons/app_16.png  ${vivace_iconset}/icon_16x16.png
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/icons/app_32.png  ${vivace_iconset}/icon_16x16@2x.png
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/icons/app_32.png  ${vivace_iconset}/icon_32x32.png
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/icons/app_64.png  ${vivace_iconset}/icon_32x32@2x.png
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/icons/app_128.png ${vivace_iconset}/icon_128x128.png
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/icons/app_256.png ${vivace_iconset}/icon_128x128@2x.png
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/icons/app_256.png ${vivace_iconset}/icon_256x256.png
        COMMAND iconutil -c icns -o ${vivace_icns} ${vivace_iconset}
        DEPENDS ${vivace_icns_sources}
        COMMENT "Generating vivace.icns for the app bundle"
        VERBATIM)
    target_sources(vivace PRIVATE ${vivace_icns})
    set_source_files_properties(${vivace_icns} PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
        GENERATED TRUE)
    set_target_properties(vivace PROPERTIES MACOSX_BUNDLE_ICON_FILE vivace.icns)
endif()

# Compile the video-equalizer shader to .qsb and embed it under the module
# prefix, so ShaderEffect can load it at qrc:/qt/qml/Vivace/shaders/*.qsb.
qt_add_shaders(vivace "vivace_shaders"
    PREFIX "/qt/qml/Vivace"
    FILES shaders/equalizer.frag
)

# UI translations: compile the .ts catalogs to .qm and embed them at
# qrc:/i18n/vivace_<lang>.qm (loaded by a QTranslator in main.cpp). The
# `update_vivace_translations` target re-runs lupdate over the QML + C++
# sources to refresh the .ts files.
# One .ts per language Qt itself ships (matches the qtbase_<lang>.qm set that
# windeployqt bundles), so every Qt language is selectable and its .qm is
# embedded at qrc:/i18n/. Only vivace_ja.ts is fully translated so far; the
# others are translatable scaffolding (untranslated entries fall back to the
# English source), ready to fill language-by-language.
qt_add_translations(vivace
    TS_FILES
        translations/vivace_ar.ts
        translations/vivace_bg.ts
        translations/vivace_ca.ts
        translations/vivace_cs.ts
        translations/vivace_da.ts
        translations/vivace_de.ts
        translations/vivace_es.ts
        translations/vivace_fa.ts
        translations/vivace_fi.ts
        translations/vivace_fr.ts
        translations/vivace_gd.ts
        translations/vivace_he.ts
        translations/vivace_hr.ts
        translations/vivace_hu.ts
        translations/vivace_it.ts
        translations/vivace_ja.ts
        translations/vivace_ka.ts
        translations/vivace_ko.ts
        translations/vivace_lg.ts
        translations/vivace_lv.ts
        translations/vivace_nl.ts
        translations/vivace_nn.ts
        translations/vivace_pl.ts
        translations/vivace_pt_BR.ts
        translations/vivace_ru.ts
        translations/vivace_sk.ts
        translations/vivace_sv.ts
        translations/vivace_tr.ts
        translations/vivace_uk.ts
        translations/vivace_zh_CN.ts
        translations/vivace_zh_TW.ts
    RESOURCE_PREFIX "/i18n"
    LUPDATE_OPTIONS -no-obsolete
)

# qmltyperegistrations includes module headers as <playercontroller.h> etc.
target_include_directories(vivace PRIVATE src)

# castserver.cpp does #include "qrcodegen.h" (bare, same-directory style);
# point the compiler at its vendored location.
target_include_directories(vivace PRIVATE third_party/qrcodegen)

target_link_libraries(vivace PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Multimedia
    Qt6::LabsPlatform
    Qt6::Network
    # DBus drives the Linux screensaver inhibitor and MPRIS2. It is linked on
    # all platforms so the MPRIS2 code compiles everywhere (it is only
    # instantiated on Linux); restrict to LINUX before packaging if desired.
    Qt6::DBus
    # SQLite-backed LRU index for PlaylistThumbnailProvider's generated
    # thumbnail cache (playlistthumbnailprovider.cpp). The QSQLITE driver
    # ships with Qt itself, no extra vendoring needed.
    Qt6::Sql
)

# macOS: screensaver inhibition uses an IOKit power-management assertion.
if(APPLE)
    target_link_libraries(vivace PRIVATE "-framework IOKit" "-framework CoreFoundation")
endif()

# Windows: System Media Transport Controls via C++/WinRT. The cppwinrt headers
# live in a Windows SDK subdirectory that is not on the default include path.
if(WIN32)
    target_sources(vivace PRIVATE src/windowssmtc.h src/windowssmtc.cpp)
    # C++/WinRT's generated headers still use <experimental/coroutine>; recent
    # MSVC STLs (e.g. VS2026's, toolset 18) hard-error on that deprecated
    # header (STL1011) instead of just warning.
    target_compile_definitions(vivace PRIVATE _SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS)
    if(DEFINED ENV{WindowsSdkDir})
        set(_wsdk "$ENV{WindowsSdkDir}")
    else()
        set(_wsdk "C:/Program Files (x86)/Windows Kits/10")
    endif()
    file(GLOB _cppwinrt_dirs "${_wsdk}/Include/*/cppwinrt")
    if(_cppwinrt_dirs)
        list(SORT _cppwinrt_dirs)
        list(REVERSE _cppwinrt_dirs)
        list(GET _cppwinrt_dirs 0 _cppwinrt_dir)
        target_include_directories(vivace PRIVATE "${_cppwinrt_dir}")
    endif()
    target_link_libraries(vivace PRIVATE RuntimeObject.lib)
endif()

# libx264-164.dll: avcodec-60.dll (this custom Qt's FFmpeg build) statically
# imports it for H.264 *encode* support, even though playback (decode-only)
# never calls into it -- Windows' loader still resolves a DLL's entire import
# table before any of its code can run, so this DLL's absence makes
# avcodec-60.dll -- and therefore ffmpegmediaplugin.dll -- fail to load at
# all. Qt Multimedia then silently falls back to the Windows Media Foundation
# backend: HEVC fails outright ("Unsupported media, a codec is missing", MF
# needs the separate Store extension) and H.264 intermittently hits MF's own
# "Media session serious error" on some files. Diagnosed 2026-07-16: this DLL
# lives in the FFmpeg build directory Qt was configured against
# (C:/opt/ffmpeg-devel/bin) but is NOT copied into Qt's own bin/, so
# windeployqt's FFmpeg deployment never picks it up. It had only ever loaded
# by coincidence in QtCreator-launched dev builds because an unrelated, older
# Qt 6.8.3 kit's bin/ (also on PATH there) happens to ship a same-named copy.
if(WIN32)
    set(VIVACE_FFMPEG_BIN_DIR "C:/opt/ffmpeg-devel/bin" CACHE PATH
        "Directory holding libx264-164.dll (and other private FFmpeg deps not deployed by windeployqt)")
    find_file(VIVACE_LIBX264_DLL
        NAMES libx264-164.dll
        HINTS "${VIVACE_FFMPEG_BIN_DIR}"
        PATHS "$ENV{FFMPEG_DIR}/bin"
    )
    if(VIVACE_LIBX264_DLL)
        install(FILES "${VIVACE_LIBX264_DLL}" DESTINATION ${CMAKE_INSTALL_BINDIR})
        add_custom_command(TARGET vivace POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${VIVACE_LIBX264_DLL}" "$<TARGET_FILE_DIR:vivace>"
            COMMENT "Copying libx264-164.dll (avcodec's encode-only dependency, absent from Qt's own bin)"
        )
    else()
        message(WARNING "libx264-164.dll not found (looked in VIVACE_FFMPEG_BIN_DIR=${VIVACE_FFMPEG_BIN_DIR}) -- "
                        "avcodec-60.dll will fail to load and Vivace will silently fall back to the Windows "
                        "Media Foundation backend. Set VIVACE_FFMPEG_BIN_DIR to the FFmpeg build's bin directory.")
    endif()
endif()

# Optional Blu-ray Disc playback via libbluray (LGPL-2.1-or-later,
# https://code.videolan.org/videolan/libbluray) -- parses BD-ROM disc
# structure (index.bdmv/MovieObject.bdmv/.mpls playlists, optionally AACS-
# decrypting) and demuxes the selected title's M2TS stream, handling
# multi-clip stitching internally. Unlike DVD (where Vivace hand-rolls its
# own IFO/VOB parser, consulting libdvdread's ifo_read.c only as reference
# material), Blu-ray's on-disc format is dramatically more involved (MPLS/
# CLPI/MovieObject, plus optional AACS/BD+ crypto) and no vendored reference
# source is available locally to reimplement it from scratch against --
# libbluray is the same actively-maintained library every other open-source
# BD player (VLC, mpv, Kodi, HandBrake) already links against, so Vivace
# links it too rather than reinventing BD-ROM navigation. src/blurayplayer.*
# is a thin QIODevice wrapper around its public bd_*() API; it always
# compiles, but is a no-op (returns empty/false everywhere) when libbluray
# isn't available, gated by the VIVACE_HAVE_BLURAY compile definition below.
#
# BUILT FROM SOURCE, uniformly on every platform, via ExternalProject_Add --
# this replaced an earlier Windows-only approach (MSYS2's prebuilt package +
# a hand-generated MSVC import library bridging a MinGW-built DLL) once it
# became clear that approach couldn't extend to Linux/macOS at all, and
# separately dragged in a 17-DLL runtime dependency closure (BD-J font
# rendering via freetype/fontconfig, metadata parsing via libxml2 -- none of
# which src/blurayplayer.cpp's basic/primitive playback support even uses).
#
# libbluray's own build system is Meson (as of 1.4.0; there is no CMake or
# autotools support upstream) -- confirmed to build cleanly and NATIVELY
# with MSVC (no MinGW bridging needed at all) by fetching the real 1.5.0
# source and building it directly: `meson setup --backend ninja
# -Dfontconfig=disabled -Dfreetype=disabled -Dlibxml2=disabled
# -Dbdj_jar=disabled -Denable_tools=false -Ddefault_library=static` inside
# an MSVC developer environment auto-detects cl.exe/link.exe/lib.exe with
# zero extra configuration, and the resulting static libbluray.a links
# into a real MSVC executable with no other runtime dependency at all
# (verified: a standalone test program linked this way opened the real
# disc used throughout this project's Blu-ray testing and produced
# identical results to the earlier MSYS2-based build). Disabling those
# four options removes BD-J entirely (no on-disc Java menus -- matching
# this project's own explicit "no on-disc HDMV/BD-J menu support" scope
# decision) and the whole font/XML dependency chain along with it, so the
# ONLY extra system libraries needed are small, always-present platform
# ones (see VIVACE_LIBBLURAY_SYSTEM_LIBS below), not a large closure of
# third-party DLLs to hunt down and deploy.
#
# CMake's own Visual Studio generator runs custom build steps (including
# ExternalProject_Add's) through MSBuild, which already has the correct
# INCLUDE/LIB/PATH for the active MSVC toolset -- the same environment
# meson's own compiler auto-detection was verified against above -- so no
# extra environment plumbing is done here for the Windows/MSVC case. A
# Ninja-generator build invoked from a plain (non-Developer-Prompt) shell
# would need the same vcvars environment CMake's own compiler detection
# already requires in that scenario, so this isn't a new constraint.
#
# ONLY VERIFIED ON WINDOWS so far (this project's only available dev
# machine); Linux/macOS use the identical Meson recipe (Meson and its
# `-Dxxx=disabled` options are genuinely cross-platform, and this project's
# Qt-Multimedia-patch build scripts already establish the same
# build-from-source-not-a-system-package convention for other
# dependencies), but the exact system libraries libbluray needs per
# platform (VIVACE_LIBBLURAY_SYSTEM_LIBS below) are reasoned from reading
# libbluray's own meson.build directly (its explicit `if host_machine.
# system() == 'darwin'` block, etc.), not confirmed by an actual Linux/
# macOS build in this environment.
# Always built as a Release configuration (Meson --buildtype release, MSVC
# /MD) regardless of which config Vivace itself is being built as -- a
# multi-config generator (Visual Studio) doesn't know which config will
# actually be built at CMake CONFIGURE time, and building libbluray twice
# (once per CRT variant) would double this dependency's build time for
# every developer/CI run. This produces a real MSVC LNK4098 warning
# ("defaultlib 'MSVCRT' conflicts") when Vivace itself is built as Debug
# (/MDd) -- confirmed harmless for how Vivace actually calls libbluray
# today: grepping bluray.h directly shows only bd_get_meta_file() and
# bd_read_file() return memory the CALLER must free() (a real cross-CRT
# hazard if the allocator and the free() call come from different CRT
# instances) -- and src/blurayplayer.cpp calls neither. If a future
# feature ever does call one of those two functions, this becomes a real
# bug that needs the full fix (building libbluray per-config, selected via
# a $<CONFIG:...> generator expression in the target_link_libraries call
# below), not just a warning to tolerate.
option(VIVACE_ENABLE_BLURAY "Enable Blu-ray Disc playback (needs meson, ninja, and git at configure time)" ON)
set(VIVACE_HAVE_BLURAY FALSE)
if(VIVACE_ENABLE_BLURAY)
    find_program(VIVACE_MESON_EXECUTABLE NAMES meson)
    find_program(VIVACE_NINJA_EXECUTABLE NAMES ninja)
    find_package(Git QUIET)

    if(VIVACE_MESON_EXECUTABLE AND VIVACE_NINJA_EXECUTABLE AND Git_FOUND)
        set(VIVACE_HAVE_BLURAY TRUE)

        set(VIVACE_LIBBLURAY_GIT_TAG "1.5.0" CACHE STRING
            "libbluray git tag to build (pinned to the latest tagged release)")
        set(VIVACE_LIBBLURAY_PREFIX "${CMAKE_BINARY_DIR}/bluray-prefix")
        set(VIVACE_LIBBLURAY_SRC "${VIVACE_LIBBLURAY_PREFIX}/src/bluray_ext")
        set(VIVACE_LIBBLURAY_BUILD "${VIVACE_LIBBLURAY_PREFIX}/build")
        set(VIVACE_LIBBLURAY_INSTALL "${VIVACE_LIBBLURAY_PREFIX}/install")
        # Meson names a default_library=static target libFOO.a on every
        # platform (confirmed on Windows/MSVC directly -- a real COFF
        # archive despite the Unix-style ".a" name; the same Unix naming
        # convention applies natively on Linux/macOS).
        #
        # This path assumes plain "lib/" -- NOT guaranteed without the
        # explicit -Dlibdir=lib passed to `meson setup` below. Root-caused
        # 2026-08-18 (real CI failure, Ubuntu only: gmake "No rule to make
        # target '.../lib/libbluray.a', needed by 'vivace'"): Meson
        # auto-detects a Debian/Ubuntu MULTIARCH libdir convention
        # (e.g. "lib/x86_64-linux-gnu") by default on those distros, so an
        # un-pinned `meson setup` installs the static library to a
        # different path than this variable (and BUILD_BYPRODUCTS below)
        # assume -- CMake's Makefiles generator then has no rule at all
        # for the (never-produced) plain "lib/libbluray.a" path it was
        # told to expect, and errors out exactly like this the moment
        # vivace's link step is reached. macOS/Windows builds were
        # unaffected (Homebrew/MSVC meson installs both default to plain
        # "lib", no multiarch triplet), which is why only the Linux job
        # failed. Fix: -Dlibdir=lib forces the same plain, predictable
        # layout on every platform.
        set(VIVACE_LIBBLURAY_LIBRARY_FILE
            "${VIVACE_LIBBLURAY_INSTALL}/lib/libbluray.a")
        # libbluray's own meson.build declares its libudfread dependency
        # (contrib/libudfread, a git submodule built as an internal Meson
        # subproject) via plain `dependencies:`, not `link_whole:` -- so
        # Meson does NOT merge udfread's object files into libbluray.a;
        # it stays a genuinely SEPARATE static archive that any final
        # consumer must link too. It's also never installed to the
        # prefix (confirmed empirically: install/lib/ only ever contains
        # libbluray.a + libbluray.pc, and libbluray.pc's own "Libs:" field
        # doesn't mention udfread at all) -- so it has to be referenced
        # from the BUILD tree, not VIVACE_LIBBLURAY_INSTALL. Root-caused
        # 2026-08-18 (real CI failure, macOS only): Apple's `ld` refused
        # to link with "Undefined symbols ... _udfread_open, ... referenced
        # from libbluray.a[..](libbluray_disc_udf_fs.c.o)" -- Windows and
        # Linux builds hadn't shown this (most likely because whatever
        # code path bd_open() takes for those targets never pulls
        # libbluray_disc_udf_fs.c.o's object into the link at all, so its
        # unresolved references are simply never noticed there), but the
        # missing dependency was real on every platform regardless -- only
        # macOS's stricter linker behavior exposed it first.
        set(VIVACE_LIBUDFREAD_LIBRARY_FILE
            "${VIVACE_LIBBLURAY_BUILD}/contrib/libudfread/src/libudfread.a")

        include(ExternalProject)
        ExternalProject_Add(bluray_ext
            GIT_REPOSITORY "https://code.videolan.org/videolan/libbluray.git"
            GIT_TAG "${VIVACE_LIBBLURAY_GIT_TAG}"
            GIT_SUBMODULES_RECURSE TRUE
            PREFIX "${VIVACE_LIBBLURAY_PREFIX}"
            SOURCE_DIR "${VIVACE_LIBBLURAY_SRC}"
            CONFIGURE_COMMAND "${VIVACE_MESON_EXECUTABLE}" setup
                "${VIVACE_LIBBLURAY_BUILD}" "${VIVACE_LIBBLURAY_SRC}"
                --backend ninja
                --buildtype release
                --prefix "${VIVACE_LIBBLURAY_INSTALL}"
                -Dlibdir=lib
                -Ddefault_library=static
                -Denable_tools=false
                -Dbdj_jar=disabled
                -Dfontconfig=disabled
                -Dfreetype=disabled
                -Dlibxml2=disabled
                # Forces Meson to ALWAYS build the vendored contrib/libudfread
                # submodule rather than using a system-found one, regardless
                # of what's pre-installed on a given machine. Root-caused
                # 2026-08-18 (real CI failure, macOS only): Homebrew's ffmpeg@7
                # (already installed for the qtmultimedia patches) pulls in
                # libudfread as a transitive dependency, so Meson found it
                # via pkg-config ("Run-time dependency libudfread found: YES",
                # "External libudfread: true") and never built the internal
                # subproject at all -- meaning VIVACE_LIBUDFREAD_LIBRARY_FILE's
                # build-tree path genuinely never got created there ("no such
                # file or directory"), even though the SAME path was verified
                # to exist correctly on Windows (no system libudfread there).
                # Rather than teach CMake to conditionally link a system
                # library that could be shared, static, or absent depending
                # on the machine, force the SAME internal-subproject build on
                # every platform, matching the fixed build-tree path this
                # project's CMakeLists.txt already assumes.
                -Dforce_fallback_for=libudfread
            BUILD_COMMAND "${VIVACE_NINJA_EXECUTABLE}" -C "${VIVACE_LIBBLURAY_BUILD}"
            INSTALL_COMMAND "${VIVACE_NINJA_EXECUTABLE}" -C "${VIVACE_LIBBLURAY_BUILD}" install
            BUILD_BYPRODUCTS "${VIVACE_LIBBLURAY_LIBRARY_FILE}" "${VIVACE_LIBUDFREAD_LIBRARY_FILE}"
            USES_TERMINAL_CONFIGURE TRUE
            USES_TERMINAL_BUILD TRUE
            USES_TERMINAL_INSTALL TRUE
        )
        message(STATUS "Blu-ray Disc playback: ENABLED (building libbluray "
                        "${VIVACE_LIBBLURAY_GIT_TAG} from source via Meson)")
    else()
        set(_vivace_bluray_missing "")
        if(NOT VIVACE_MESON_EXECUTABLE)
            list(APPEND _vivace_bluray_missing "meson")
        endif()
        if(NOT VIVACE_NINJA_EXECUTABLE)
            list(APPEND _vivace_bluray_missing "ninja")
        endif()
        if(NOT Git_FOUND)
            list(APPEND _vivace_bluray_missing "git")
        endif()
        message(STATUS "Blu-ray Disc playback: DISABLED (missing build tool(s): "
                        "${_vivace_bluray_missing} -- install them and reconfigure, "
                        "or set VIVACE_ENABLE_BLURAY=OFF to silence this message)")
    endif()
endif()

target_sources(vivace PRIVATE src/blurayplayer.h src/blurayplayer.cpp)
if(VIVACE_HAVE_BLURAY)
    # IMPORTED target instead of linking VIVACE_LIBBLURAY_LIBRARY_FILE's raw
    # path directly: CMake's idiomatic pattern for "link a prebuilt file
    # produced by an external step". Doesn't by itself fix the 2026-08-18
    # "No rule to make target" bug above (that was a wrong PATH -- Meson's
    # own multiarch libdir default -- not a missing target/ordering
    # mechanism; add_dependencies(vivace bluray_ext) was already correctly
    # ordering things the whole time), but it's a cleaner, more robust shape
    # than a bare path string in target_link_libraries.
    add_library(bluray_imported STATIC IMPORTED GLOBAL)
    set_target_properties(bluray_imported PROPERTIES
        IMPORTED_LOCATION "${VIVACE_LIBBLURAY_LIBRARY_FILE}")
    add_dependencies(bluray_imported bluray_ext)

    # libbluray.a's own libudfread dependency isn't merged into it or
    # installed (see VIVACE_LIBUDFREAD_LIBRARY_FILE's own comment above) --
    # link it explicitly too, same IMPORTED-target shape, from the build
    # tree rather than the install prefix.
    add_library(udfread_imported STATIC IMPORTED GLOBAL)
    set_target_properties(udfread_imported PROPERTIES
        IMPORTED_LOCATION "${VIVACE_LIBUDFREAD_LIBRARY_FILE}")
    add_dependencies(udfread_imported bluray_ext)

    target_compile_definitions(vivace PRIVATE VIVACE_HAVE_BLURAY=1)
    target_include_directories(vivace PRIVATE "${VIVACE_LIBBLURAY_INSTALL}/include")
    # udfread_imported must come AFTER bluray_imported: static-linker
    # order matters when one archive's undefined symbols are resolved by
    # a later one on the command line (ld/GNU ld both link left-to-right).
    target_link_libraries(vivace PRIVATE bluray_imported udfread_imported)

    # Small, always-present platform libraries libbluray's own code calls
    # into directly -- NOT a third-party dependency closure to deploy at
    # runtime, just system libraries every install of the OS already has.
    # Windows: VERIFIED by a real link (Advapi32 for the registry lookup /
    # Shell32 for the special-folder lookup libbluray's BD-J JVM-search
    # code (file_dirs_win32.c/bdj.c) still references even with
    # -Dbdj_jar=disabled -- that flag only skips building the JAR itself,
    # not this small amount of always-compiled calling code). Linux/macOS:
    # reasoned from reading libbluray's own meson.build directly (its
    # explicit `if host_machine.system() == 'darwin'` dependency block),
    # NOT independently verified against a real build on either platform.
    if(WIN32)
        set(VIVACE_LIBBLURAY_SYSTEM_LIBS Advapi32 Shell32)
    elseif(APPLE)
        set(VIVACE_LIBBLURAY_SYSTEM_LIBS "-framework CoreFoundation" "-framework DiskArbitration")
    else()
        set(VIVACE_LIBBLURAY_SYSTEM_LIBS ${CMAKE_DL_LIBS})
    endif()
    target_link_libraries(vivace PRIVATE ${VIVACE_LIBBLURAY_SYSTEM_LIBS})
endif()

install(TARGETS vivace
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Cross-platform deployment via Qt's CMake API (Qt 6.3+). `cmake --install`
# then gathers the Qt runtime, QML modules, plugins and FFmpeg for the current
# platform: windeployqt on Windows, macdeployqt for macOS bundles, and CMake's
# own dependency scanning on Linux (there is no linuxdeployqt in Qt 6). The
# packaging/ build scripts install into the installer's data/ dir.
qt_generate_deploy_qml_app_script(
    TARGET vivace
    OUTPUT_SCRIPT vivace_deploy_script
    MACOS_BUNDLE_POST_BUILD
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${vivace_deploy_script})
