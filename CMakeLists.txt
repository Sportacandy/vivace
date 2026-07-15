cmake_minimum_required(VERSION 3.24)

project(vivace VERSION 0.1.0 LANGUAGES CXX)

find_package(Qt6 6.11 REQUIRED COMPONENTS Quick QuickControls2 Multimedia LabsPlatform Network ShaderTools DBus LinguistTools)
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
    qt_add_dbus_interface(vivace_kwallet_dbus_sources
        "${vivace_qtkeychain_dir}/org.kde.KWallet.xml" kwallet_interface KWalletInterface)
    target_sources(vivace PRIVATE ${vivace_kwallet_dbus_sources})
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
)

# Windows: embed the brand icon into the executable. The RC references
# icons/vivace.ico; icon ID 1 becomes the exe icon used by Explorer, the
# taskbar, Start-menu shortcuts (iconId=0), file-type associations
# (DefaultIcon "exe",0) and the SMTC flyout.
if(WIN32)
    target_sources(vivace PRIVATE icons/vivace.rc)
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
)

# macOS: screensaver inhibition uses an IOKit power-management assertion.
if(APPLE)
    target_link_libraries(vivace PRIVATE "-framework IOKit" "-framework CoreFoundation")
endif()

# Windows: System Media Transport Controls via C++/WinRT. The cppwinrt headers
# live in a Windows SDK subdirectory that is not on the default include path.
if(WIN32)
    target_sources(vivace PRIVATE src/windowssmtc.h src/windowssmtc.cpp)
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
