cmake_minimum_required(VERSION 3.24)

project(vivace VERSION 0.5.0 LANGUAGES CXX C)

find_package(Qt6 6.11 REQUIRED COMPONENTS Quick QuickControls2 Multimedia LabsPlatform Network ShaderTools LinguistTools Sql)
qt_standard_project_setup(REQUIRES 6.11)

# DBus drives the Linux screensaver inhibitor, MPRIS2, and the Linux KWallet/
# Secret-Service credential-store backend (see the target_link_libraries and
# elseif(UNIX) blocks below). Android has no D-Bus session bus, and neither
# the official nor a self-built Qt kit for Android even provides the DBus
# module -- confirmed 2026-09-05 (a plain `find_package(Qt6 ... DBus)`
# unconditionally REQUIRED failed configure outright: "Failed to find
# required Qt component 'DBus'... Expected Config file...does NOT exist").
# Every other platform Vivace targets already ships it, so this stays
# narrowly scoped to Android rather than changing anything already verified
# working elsewhere. VIVACE_HAVE_DBUS gates the DBus-only sources below
# (mpris2.*, the KWallet keychain backend).
if(NOT ANDROID)
    find_package(Qt6 6.11 REQUIRED COMPONENTS DBus)
    set(VIVACE_HAVE_DBUS TRUE)
else()
    set(VIVACE_HAVE_DBUS FALSE)
endif()

# Android package source dir override hook -- see its use on the vivace
# target's QT_ANDROID_PACKAGE_SOURCE_DIR property below for the full
# rationale. Defaults to this repo's own android/ dir, so the normal
# open-source build is unaffected.
set(VIVACE_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android" CACHE PATH
    "Android package source dir (manifest, icons, ...) passed to androiddeployqt")

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

# Optional Android seed-data hook for a downstream/private build (e.g. a
# family-only variant maintained in a separate project that references this
# repo as a git submodule, since Android's app-storage sandboxing means the
# ONLY way to pre-populate a fresh install's data is to bake it into the
# APK at build time -- there is no way for a separate installer/app to
# write into another app's private storage the way a Windows installer can
# write into %APPDATA%). Each variable, when set, points at a file in the
# SAME JSON-tree format FavoritesModel already reads/writes for tv.json/
# radio.json/favorites.json; PlayerController's constructor copies it into
# place on a fresh install only (never overwriting a real, already-existing,
# possibly user-edited list -- see the Android branch there). All three
# default to empty, a complete no-op for the normal open-source build:
# nothing is embedded (the fixed alias below is simply absent from the
# resource system when unset), nothing changes.
set(VIVACE_TV_SEED_FILE "" CACHE FILEPATH
    "Optional tv.json-format file to seed a fresh Android install's TV channel list from")
set(VIVACE_RADIO_SEED_FILE "" CACHE FILEPATH
    "Optional radio.json-format file to seed a fresh Android install's radio station list from")
set(VIVACE_FAVORITES_SEED_FILE "" CACHE FILEPATH
    "Optional favorites.json-format file to seed a fresh Android install's Favorites list from")

set(vivace_seed_resources "")
if(ANDROID)
    # Only actually embedded on Android -- that is the one platform where
    # this mechanism is needed at all (see the rationale above), so there is
    # no reason to waste desktop-binary size on it there even if someone set
    # one of these variables for an unrelated reason.
    foreach(_vivace_seed_pair "VIVACE_TV_SEED_FILE:tv" "VIVACE_RADIO_SEED_FILE:radio"
                              "VIVACE_FAVORITES_SEED_FILE:favorites")
        string(REPLACE ":" ";" _vivace_seed_pair "${_vivace_seed_pair}")
        list(GET _vivace_seed_pair 0 _vivace_seed_var)
        list(GET _vivace_seed_pair 1 _vivace_seed_name)
        if(${_vivace_seed_var})
            # Fixed, predictable alias (":/qt/qml/Vivace/seed/<name>.json",
            # verified directly against a real build's generated resource
            # registration) independent of where the real file physically
            # lives, so PlayerController can reference it by a constant path
            # regardless of who supplies it.
            set_source_files_properties("${${_vivace_seed_var}}" PROPERTIES
                QT_RESOURCE_ALIAS "seed/${_vivace_seed_name}.json")
            list(APPEND vivace_seed_resources "${${_vivace_seed_var}}")
        endif()
    endforeach()
endif()

qt_add_qml_module(vivace
    URI Vivace
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/ToolbarItems.js
        qml/GeometryUtils.js
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
        qml/PrefToolsPage.qml
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
        src/androidshareintent.h src/androidshareintent.cpp
    RESOURCES
        ${vivace_icon_files}
        ${vivace_help_files}
        ${vivace_seed_resources}
)

# MPRIS2 (src/mpris2.*) unconditionally includes QtDBus headers -- it's only
# ever instantiated on Linux (see mediacontrols.cpp), but needs Qt6::DBus to
# even compile. Excluded entirely on Android, which has neither a D-Bus
# session bus nor (as of this writing) the Qt DBus module available at all.
if(VIVACE_HAVE_DBUS)
    target_sources(vivace PRIVATE src/mpris2.h src/mpris2.cpp)
endif()

# Third-party vendored sources live under third_party/<name>/ regardless of
# size or how their upstream expresses the license (inline header comment vs.
# an external file like QtKeychain's COPYING) -- kept out of src/ so it's
# always clear at a glance which code is ours vs. vendored.
#
# QtKeychain (third_party/qtkeychain/, modified-BSD license -- see
# third_party/qtkeychain/COPYING) provides OS-level secure credential storage
# (Windows Credential Manager / macOS Keychain / Linux Secret Service+
# KWallet), used by SecureCredentials (src/securestore.*) to store the
# OpenSubtitles and proxy passwords instead of plaintext QSettings (Android:
# the AndroidKeyStore, added 2026-09-12 -- see the ANDROID branch below).
# Only the library sources for the platforms Vivace targets are vendored (no
# tests/examples/translations, no Haiku/WASM backends); compiled directly
# into the vivace target rather than as a separate library, so
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
elseif(ANDROID)
    # No D-Bus (see VIVACE_HAVE_DBUS above), so neither the real KWallet/
    # Secret-Service backend (elseif(UNIX) below) nor its own Android
    # equivalent applies here -- but upstream QtKeychain DOES ship a real
    # Android backend (JNI + the Android Keystore, keychain_android.cpp +
    # androidkeystore.cpp/_p.h), just never vendored when this project only
    # targeted desktop platforms. Added 2026-09-12, fetched from the exact
    # tag (0.17.0) already vendored for every other platform, so it lines up
    # perfectly with the already-vendored keychain_p.h/plaintextstore_p.h --
    # no source changes were needed. It generates an RSA-2048 keypair per
    # service+key alias inside the AndroidKeyStore (a hardware-backed
    # keystore on most devices) and uses it in a hybrid RSA+AES-GCM scheme
    # (the RSA key wraps a random AES-256 key, which does the actual
    # encryption -- RSA alone can't handle payloads over its own modulus
    # size) to encrypt the value before it's written through the SAME
    # PlainTextStore/QSettings storage every other platform's own backend
    # uses as its final at-rest location -- so only the VALUE stored there
    # is opaque ciphertext, unlike a real plaintext fallback. Uses only
    # QJniObject/QJniEnvironment (public QtCore API, since Qt 6) -- no
    # Qt6::CorePrivate dependency, unlike androidshareintent.h's.
    # UNVERIFIED: fetched and wired to mirror upstream's own CMakeLists.txt
    # ANDROID branch exactly, not yet compiled against a real Android NDK
    # toolchain in this environment.
    target_sources(vivace PRIVATE
        ${vivace_qtkeychain_dir}/keychain_android.cpp
        ${vivace_qtkeychain_dir}/androidkeystore.cpp
        ${vivace_qtkeychain_dir}/androidkeystore_p.h
        ${vivace_qtkeychain_dir}/plaintextstore.cpp
        ${vivace_qtkeychain_dir}/plaintextstore_p.h
    )
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
    # Android: without this, Qt's own CMake Android integration
    # (Qt6AndroidGradleHelpers.cmake, inside the Qt-for-Android kit itself,
    # not this repo) silently defaults the app's package/applicationId to
    # "org.qtproject.example.vivace" (found 2026-09-07 -- the user never
    # set this and was surprised to see it) -- a generic placeholder meant
    # to be overridden, not a real identifier. Deliberately NOT the same
    # "org.vivaceplayer.vivace" this project uses for its macOS bundle ID
    # and desktop packaging -- the user asked for their own personal
    # domain-based identifier here instead.
    QT_ANDROID_PACKAGE_NAME "org.vivaceplayer.vivace"
    # Points androiddeployqt at this project's own custom Android package
    # source dir (manifest, launcher icon, splash screen, etc.) so it merges
    # those on top of Qt's own default template instead of using only that
    # bare template. Found 2026-09-12 misplaced on the unrelated
    # bluray_imported IMPORTED target below (a copy-paste mistake -- that
    # target has nothing to do with Android packaging, so androiddeployqt
    # never saw this property on the real vivace target at all): the built
    # APK's res/ contained only Qt's own auto-generated files
    # (values/libs.xml, xml/qtprovider_paths.xml) and the whole
    # android/res/mipmap-*, drawable*, values/colors.xml etc. tree was
    # silently absent -- explaining why the launcher icon (and splash
    # screen) never took effect despite the resource files themselves being
    # correct and present in the source tree.
    #
    # VIVACE_ANDROID_PACKAGE_SOURCE_DIR (cache variable, defaults to this
    # repo's own android/ dir) lets an outer project override which
    # android/ folder gets used -- e.g. a private/downstream build (see
    # VIVACE_TV_SEED_FILE above) that references this repo as a git
    # submodule and needs its OWN manifest (a distinct applicationId/app
    # label) so it can install alongside an official Vivace release on the
    # same device rather than colliding with or overwriting it.
    QT_ANDROID_PACKAGE_SOURCE_DIR "${VIVACE_ANDROID_PACKAGE_SOURCE_DIR}"
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
    # SQLite-backed LRU index for PlaylistThumbnailProvider's generated
    # thumbnail cache (playlistthumbnailprovider.cpp). The QSQLITE driver
    # ships with Qt itself, no extra vendoring needed.
    Qt6::Sql
)

# DBus drives the Linux screensaver inhibitor and MPRIS2. It was linked on
# every platform (so the MPRIS2 code compiles everywhere, even though it's
# only instantiated on Linux -- see mediacontrols.cpp) until Android exposed
# that this was never actually safe to assume: Android has no DBus module at
# all (see VIVACE_HAVE_DBUS, set above). screensaver.cpp/mpris2.cpp are both
# now excluded from compiling their DBus-touching code on Android, so this
# can just skip linking there instead of being unconditional.
if(VIVACE_HAVE_DBUS)
    target_link_libraries(vivace PRIVATE Qt6::DBus)
endif()

# Android's Share-intent handling (src/androidshareintent.*) needs
# QtAndroidPrivate::NewIntentListener, a private QtCore header. The
# Qt6::CorePrivate target exists as soon as Qt6Core is found, but its
# INTERFACE_INCLUDE_DIRECTORIES (the actual path to QtCore/private/*_p.h)
# is only populated once the Qt6CorePrivate package is separately found --
# which only happens when it's explicitly requested as a find_package
# COMPONENT (see the macOS QuickPrivate/QuickTemplates2Private case just
# above, which already does this correctly); Core itself is only ever
# pulled in here as an implicit dependency of Quick/Multimedia/etc., never
# named directly, so this find_package call is required, not optional.
if(ANDROID)
    find_package(Qt6 REQUIRED COMPONENTS CorePrivate)
    target_link_libraries(vivace PRIVATE Qt6::CorePrivate)
endif()

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

    # avfilter-10.dll / postproc-58.dll: the SAME class of gap as libx264-164.dll
    # above, for the SAME reason (windeployqt's FFmpeg deployment doesn't pick
    # them up), just for two DIFFERENT, NEWER files -- the deinterlace patch's
    # own avfilter graph (playbackengine/qffmpegdeinterlacer.cpp) links
    # FFmpeg::avfilter directly into ffmpegmediaplugin.dll, and avfilter itself
    # needs postproc. Root-caused 2026-08-18 (real user report: "almost all MP4
    # media aborts playback... Media session serious error", the EXACT libx264
    # symptom text above, but for a nightly/release install this time, not a
    # local dev build) -- confirmed directly on a real installed nightly build:
    # avcodec-61.dll/avformat-61.dll/avutil-59.dll/swresample-5.dll/swscale-8.dll
    # (ffmpegmediaplugin.dll's own long-standing DIRECT imports, present since
    # well before the deinterlace patch) were all correctly deployed by
    # windeployqt, but avfilter-10.dll/postproc-58.dll (added only recently,
    # once the deinterlace patch started linking FFmpeg::avfilter) were BOTH
    # completely missing from the installed bin/ -- windeployqt's dependency
    # scan evidently doesn't generalize to every new FFmpeg-adjacent DLL name on
    # its own, matching this file's own libx264 precedent exactly.
    # scripts/build-patched-qtmultimedia-windows.ps1 already deploys both files
    # into the Qt kit's OWN bin/ as part of building the patched qtmultimedia,
    # so that's searched too, alongside VIVACE_FFMPEG_BIN_DIR/FFMPEG_DIR.
    foreach(_vivace_extra_ffmpeg_dll avfilter-10.dll postproc-58.dll)
        string(MAKE_C_IDENTIFIER "VIVACE_${_vivace_extra_ffmpeg_dll}" _vivace_extra_ffmpeg_dll_var)
        find_file(${_vivace_extra_ffmpeg_dll_var}
            NAMES "${_vivace_extra_ffmpeg_dll}"
            HINTS "${VIVACE_FFMPEG_BIN_DIR}" ${CMAKE_PREFIX_PATH}
            PATHS "$ENV{FFMPEG_DIR}/bin" "$ENV{QT_ROOT_DIR}/bin"
            PATH_SUFFIXES bin
        )
        if(${_vivace_extra_ffmpeg_dll_var})
            install(FILES "${${_vivace_extra_ffmpeg_dll_var}}" DESTINATION ${CMAKE_INSTALL_BINDIR})
            add_custom_command(TARGET vivace POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${${_vivace_extra_ffmpeg_dll_var}}" "$<TARGET_FILE_DIR:vivace>"
                COMMENT "Copying ${_vivace_extra_ffmpeg_dll} (deinterlace's avfilter dependency, absent from Qt's own windeployqt bundle)"
            )
        else()
            message(WARNING "${_vivace_extra_ffmpeg_dll} not found (looked in VIVACE_FFMPEG_BIN_DIR="
                            "${VIVACE_FFMPEG_BIN_DIR}, CMAKE_PREFIX_PATH, FFMPEG_DIR, QT_ROOT_DIR) -- "
                            "ffmpegmediaplugin.dll will fail to load and Vivace will silently fall back to the "
                            "Windows Media Foundation backend. Set VIVACE_FFMPEG_BIN_DIR to the FFmpeg build's "
                            "bin directory (the same one scripts/build-patched-qtmultimedia-windows.ps1 deploys "
                            "this file into).")
        endif()
    endforeach()
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

        # On Android, meson's own compiler auto-detection has no idea it's
        # supposed to cross-compile -- run as a plain CONFIGURE_COMMAND with
        # no cross-file, it just finds whatever compiler is on the HOST
        # PATH (confirmed 2026-09-05, real build failure: ld.lld refused
        # every libbluray.a member with "is neither ET_REL nor LLVM
        # bitcode" -- native Windows COFF .obj files, not aarch64-android
        # ELF, linked into an otherwise-correctly-cross-compiled vivace).
        # A Meson "cross file" is the standard fix: it tells Meson which
        # compiler/binutils to actually invoke and what target machine to
        # describe itself as, instead of auto-detecting the host.
        # Built entirely from CMake's OWN already-resolved Android
        # toolchain variables (the exact ones the outer, correctly-
        # cross-compiling CMake/Ninja build already uses for `vivace`
        # itself) rather than re-deriving NDK paths independently, so it
        # can't drift from whatever toolchain/API-level/ABI combination
        # the user's own CMake configure actually resolved to.
        # CONFIRMED 2026-09-20 by a real Android CI run: the cross-file's
        # own `c = ['${CMAKE_C_COMPILER}']` line rendered as `c = ['']`,
        # since this project's top-level project() only ever enabled
        # LANGUAGES CXX -- CMake never runs C-compiler detection (and
        # never populates CMAKE_C_COMPILER at all) unless C is actually
        # enabled as a project language, regardless of what a cross-
        # compiling toolchain file itself could resolve if asked. Fixed
        # by adding C to the top-level project() languages (harmless
        # everywhere else -- no .c sources are needed for CMake to just
        # detect a C compiler alongside the C++ one).
        set(VIVACE_LIBBLURAY_MESON_EXTRA_ARGS "")
        if(ANDROID)
            if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
                set(_vivace_meson_cpu_family "aarch64")
                set(_vivace_meson_cpu "aarch64")
            elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
                set(_vivace_meson_cpu_family "arm")
                set(_vivace_meson_cpu "armv7")
            elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
                set(_vivace_meson_cpu_family "x86_64")
                set(_vivace_meson_cpu "x86_64")
            elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86")
                set(_vivace_meson_cpu_family "x86")
                set(_vivace_meson_cpu "i686")
            else()
                message(WARNING "libbluray Android cross-file: unrecognized "
                                "CMAKE_ANDROID_ARCH_ABI '${CMAKE_ANDROID_ARCH_ABI}', "
                                "defaulting to aarch64")
                set(_vivace_meson_cpu_family "aarch64")
                set(_vivace_meson_cpu "aarch64")
            endif()
            # CMAKE_<LANG>_COMPILER_TARGET is the documented CMake variable
            # for "the --target= a Clang-driver cross-compile uses"; falls
            # back to reconstructing it from the ABI + API level (the same
            # two pieces the target triple embeds, e.g. "aarch64-none-
            # linux-android28") on the off chance it isn't set for some
            # NDK/CMake version combination.
            set(_vivace_meson_target "${CMAKE_CXX_COMPILER_TARGET}")
            if(NOT _vivace_meson_target)
                if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
                    set(_vivace_meson_target "aarch64-none-linux-android${CMAKE_SYSTEM_VERSION}")
                elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
                    set(_vivace_meson_target "armv7a-none-linux-androideabi${CMAKE_SYSTEM_VERSION}")
                elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
                    set(_vivace_meson_target "x86_64-none-linux-android${CMAKE_SYSTEM_VERSION}")
                elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86")
                    set(_vivace_meson_target "i686-none-linux-android${CMAKE_SYSTEM_VERSION}")
                endif()
            endif()
            set(VIVACE_LIBBLURAY_MESON_CROSS_FILE
                "${VIVACE_LIBBLURAY_PREFIX}/android-cross-${CMAKE_ANDROID_ARCH_ABI}.ini")
            file(WRITE "${VIVACE_LIBBLURAY_MESON_CROSS_FILE}" "\
[binaries]
c = ['${CMAKE_C_COMPILER}']
cpp = ['${CMAKE_CXX_COMPILER}']
ar = '${CMAKE_AR}'
strip = '${CMAKE_STRIP}'

[built-in options]
c_args = ['--target=${_vivace_meson_target}', '--sysroot=${CMAKE_SYSROOT}']
c_link_args = ['--target=${_vivace_meson_target}', '--sysroot=${CMAKE_SYSROOT}']
cpp_args = ['--target=${_vivace_meson_target}', '--sysroot=${CMAKE_SYSROOT}']
cpp_link_args = ['--target=${_vivace_meson_target}', '--sysroot=${CMAKE_SYSROOT}']

[host_machine]
system = 'android'
cpu_family = '${_vivace_meson_cpu_family}'
cpu = '${_vivace_meson_cpu}'
endian = 'little'
")
            list(APPEND VIVACE_LIBBLURAY_MESON_EXTRA_ARGS
                --cross-file "${VIVACE_LIBBLURAY_MESON_CROSS_FILE}")
        endif()

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
                ${VIVACE_LIBBLURAY_MESON_EXTRA_ARGS}
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

# Embedded CPython (Android only) -- runs yt-dlp's own official pure-Python
# release for YouTube URL resolution. Every other platform's YoutubeResolver
# spawns a downloaded yt-dlp/ffmpeg BINARY as a subprocess -- Android forbids
# executing a native binary that wasn't shipped inside the signed APK.
# Embedding an interpreter INSIDE the app and only ever interpreting
# downloaded .py/.pyz DATA through it sidesteps that restriction (Google's
# own Device and Network Abuse policy explicitly allows this: "code that
# runs in a virtual machine or an interpreter... Python, Lua, etc. loaded at
# run time must not allow potential violations of Google Play policies" --
# i.e. permitted for a legitimate declared feature like this one).
#
# libpython.so is fetched as a prebuilt release asset from
# https://github.com/Sportacandy/cpython-android-prebuilt (a separate repo,
# so other projects can reuse it too), which cross-compiles CPython via its
# own official Android build support (PEP 738). That build script only runs
# on Linux/macOS (confirmed directly from CPython's own Android/README.md --
# Windows can't build it at all), so this project consumes it as a published
# release rather than building it as part of Vivace's own configure step --
# the same reasoning as using BtbN's prebuilt FFmpeg
# (scripts/build-patched-qtmultimedia-*.sh) instead of building FFmpeg from
# source too.
option(VIVACE_ENABLE_PYTHON_YOUTUBE
    "Enable embedded-Python YouTube URL resolution (Android only)" ON)
set(VIVACE_HAVE_PYTHON_YOUTUBE FALSE)
if(WIN32 AND VIVACE_ENABLE_PYTHON_YOUTUBE)
    # TEMPORARY, desktop-testing-only (2026-09-19, user request): the
    # embedded-Python YouTube resolver (PythonYoutubeResolver) was built
    # Android-only, but desktop's own subprocess-based YoutubeResolver has
    # turned out to be unreliable against current YouTube/yt-dlp (streaming
    # mode succeeds on only a limited set of URLs, and not even
    # consistently for those). This branch links against a REAL,
    # already-installed desktop Python (found via CMake's own standard
    # FindPython3 module -- e.g. C:/Python3 on this dev machine) purely so
    # PythonYoutubeResolver can be exercised interactively on Windows to
    # compare its success rate against the subprocess-based resolver, with
    # NO Android-specific asset extraction/packaging involved. REVERT this
    # whole branch (and every other block guarded by
    # VIVACE_CPYTHON_DESKTOP_HOME below) once the comparison is done -- this
    # was never meant to ship real desktop support this way; a real desktop
    # build would need to bundle its own Python runtime, not silently
    # depend on whatever full Python install happens to be on this machine.
    # Interpreter (not just Development) is needed too -- Python3_EXECUTABLE
    # is only ever set by the Interpreter component; requesting Development
    # alone left it empty, and VIVACE_CPYTHON_DESKTOP_HOME along with it (a
    # real bug caught by inspecting this very message() line's own output).
    find_package(Python3 COMPONENTS Interpreter Development)
    if(Python3_FOUND)
        set(VIVACE_HAVE_PYTHON_YOUTUBE TRUE)
        set(VIVACE_CPYTHON_ABI_VERSION "${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
        get_filename_component(VIVACE_CPYTHON_DESKTOP_HOME
            "${Python3_EXECUTABLE}" DIRECTORY)
        message(STATUS "Embedded-Python YouTube resolution: ENABLED "
                        "(TEMPORARY desktop test build, Python "
                        "${Python3_VERSION} at ${VIVACE_CPYTHON_DESKTOP_HOME})")
    else()
        message(WARNING "Embedded-Python YouTube resolution: no Python3 "
                         "development package found for the desktop test "
                         "build -- disabling.")
    endif()
elseif(ANDROID AND VIVACE_ENABLE_PYTHON_YOUTUBE)
    set(VIVACE_CPYTHON_VERSION "3.14.7" CACHE STRING
        "cpython-android-prebuilt release tag to fetch (must match a real \
published tag at https://github.com/Sportacandy/cpython-android-prebuilt/releases)")

    # cpython-android-prebuilt names its release assets by CPython's own GNU
    # host triple, not the Gradle-style ABI name CMAKE_ANDROID_ARCH_ABI uses.
    # SHA256 pins are of the actual v3.14.7 release assets (computed directly
    # from a real download, not guessed) -- only valid for that exact
    # version; bump both together if VIVACE_CPYTHON_VERSION is ever changed,
    # or configure will fail its hash check (which is the point: it means
    # the pin is stale, not that something is actually wrong).
    if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
        set(_vivace_cpython_host "aarch64-linux-android")
        set(_vivace_cpython_sha256 "492cdd80ebab2a1374d0749167ebed06465aa0d08430c30de183f2f113d20451")
    elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
        set(_vivace_cpython_host "x86_64-linux-android")
        set(_vivace_cpython_sha256 "27a2ac462bd7e4c62d7e382cea30dc4ab61b8369035ed734f036dde97b84756a")
    else()
        message(WARNING "Embedded-Python YouTube resolution: no prebuilt CPython "
                         "published for CMAKE_ANDROID_ARCH_ABI "
                         "'${CMAKE_ANDROID_ARCH_ABI}' (cpython-android-prebuilt "
                         "only builds arm64-v8a and x86_64) -- disabling.")
    endif()

    if(_vivace_cpython_host)
        set(VIVACE_HAVE_PYTHON_YOUTUBE TRUE)
        # "3.14", not the full "3.14.7" -- the tarball's own internal paths
        # (lib/libpython3.14.so, include/python3.14/) use major.minor only.
        string(REGEX MATCH "^[0-9]+\\.[0-9]+" VIVACE_CPYTHON_ABI_VERSION
            "${VIVACE_CPYTHON_VERSION}")

        set(VIVACE_CPYTHON_PREFIX "${CMAKE_BINARY_DIR}/cpython-android-prefix")
        # SOURCE_DIR must be a dedicated subdirectory under src/, NOT src/
        # itself -- ExternalProject's own default stamp dir is
        # <prefix>/src/<name>-stamp (a SIBLING of SOURCE_DIR under src/), and
        # its extraction step does `file(REMOVE_RECURSE ${SOURCE_DIR})`
        # before unpacking. Setting SOURCE_DIR to the bare "src" (a real
        # first-attempt mistake here, root-caused 2026-09-14 against an
        # actual failed Android build) wipes out the stamp directory as a
        # side effect of that REMOVE_RECURSE, since it's nested inside
        # SOURCE_DIR in that case -- explaining the exact observed symptom:
        # download/verify/extract all genuinely succeeded, but the final
        # `cmake -E touch <stamp>/..-download` always failed because its own
        # parent directory had just been deleted. bluray_ext avoids this
        # correctly (SOURCE_DIR = .../src/bluray_ext, not bare .../src).
        set(VIVACE_CPYTHON_SRC "${VIVACE_CPYTHON_PREFIX}/src/cpython_android_ext")
        # Real layout verified directly against a downloaded release asset
        # (tar -tzf), not assumed: everything sits under prefix/, and the
        # tarball also bundles CPython's own Android/README.md, android.py,
        # android-env.sh, and testbed/ sample app alongside it -- harmless
        # extras, just not what SOURCE_DIR's own name might suggest.
        set(VIVACE_CPYTHON_LIB
            "${VIVACE_CPYTHON_SRC}/prefix/lib/libpython${VIVACE_CPYTHON_ABI_VERSION}.so")
        # CPython's own Android build vendors its OWN copies of OpenSSL/
        # SQLite under "_python"-suffixed names (libssl_python.so etc,
        # alongside plain libssl.so/libcrypto.so/libsqlite3.so of the same
        # size -- confirmed identical byte-for-byte, just duplicated under
        # both names) specifically so its own _ssl/_hashlib/_sqlite3
        # extension modules never collide with Android's own system
        # OpenSSL (an incompatible ABI/version on most devices). These are
        # real, separate runtime dependencies of the extracted stdlib's
        # lib-dynload/*.so extension modules -- confirmed via a real
        # on-device failure, 2026-09-15: "dlopen failed: library
        # libssl_python.so not found ... needed by .../_ssl...so in
        # namespace classloader-namespace" -- the "classloader-namespace" a
        # dlopen() from app-PRIVATE storage (extractedStdlibHomeDir() at
        # runtime) gets loaded under does not search app-private storage
        # for its own NEEDED dependencies. See QT_ANDROID_EXTRA_LIBS below.
        set(VIVACE_CPYTHON_SSL_LIB "${VIVACE_CPYTHON_SRC}/prefix/lib/libssl_python.so")
        set(VIVACE_CPYTHON_CRYPTO_LIB "${VIVACE_CPYTHON_SRC}/prefix/lib/libcrypto_python.so")
        set(VIVACE_CPYTHON_SQLITE_LIB "${VIVACE_CPYTHON_SRC}/prefix/lib/libsqlite3_python.so")

        include(ExternalProject)
        ExternalProject_Add(cpython_android_ext
            URL "https://github.com/Sportacandy/cpython-android-prebuilt/releases/download/v${VIVACE_CPYTHON_VERSION}/python-${VIVACE_CPYTHON_VERSION}-${_vivace_cpython_host}.tar.gz"
            URL_HASH "SHA256=${_vivace_cpython_sha256}"
            PREFIX "${VIVACE_CPYTHON_PREFIX}"
            DOWNLOAD_DIR "${VIVACE_CPYTHON_PREFIX}/download"
            SOURCE_DIR "${VIVACE_CPYTHON_SRC}"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
            BUILD_BYPRODUCTS "${VIVACE_CPYTHON_LIB}" "${VIVACE_CPYTHON_SSL_LIB}"
                "${VIVACE_CPYTHON_CRYPTO_LIB}" "${VIVACE_CPYTHON_SQLITE_LIB}"
            USES_TERMINAL_DOWNLOAD TRUE
        )

        # The stdlib ships as a plain directory of ~2700 .py files in the
        # release tarball. FIRST ATTEMPT (reverted 2026-09-15, a real bug
        # found on a real device): zipped them into ONE file and pointed
        # config.module_search_paths at it directly (PyConfig_InitIsolatedConfig
        # + module_search_paths_set=1, no config.home) -- this consistently
        # failed on a real Android device with "Py_InitializeFromConfig
        # failed: Failed to import encodings module". Root-caused by reading
        # CPython's own official Android CI harness, bundled inside this
        # exact release tarball at Android/testbed/ (app/src/main/java/org/
        # python/testbed/MainActivity.kt + app/src/main/c/main_activity.c) --
        # it does NOT use a zip or module_search_paths at all: it extracts
        # the WHOLE stdlib as a real directory tree onto disk and points
        # PyConfig.home (via PyConfig_InitPythonConfig, not isolated) at the
        # directory ABOVE it, letting CPython's own normal getpath.c
        # calculation find lib/python<ver>/ underneath -- the same
        # PYTHONHOME convention a real desktop install uses. Since that
        # harness is CPython's own tested, working recipe for this exact
        # build (and the zip-based shortcut is not something it exercises
        # at all), Vivace now mirrors it instead of the zip shortcut: copy
        # the whole stdlib directory tree into Android assets (a real
        # recursive directory, not a single zip member) via
        # `cmake -E copy_directory`, and PythonYoutubeResolver's own
        # ensurePythonInitialized() extracts + points config.home at it the
        # same way at runtime (see pythonyoutuberesolver.cpp).
        #
        # Written directly into the checked-in android/assets/ dir (not a
        # build-tree path) because QT_ANDROID_PACKAGE_SOURCE_DIR/
        # androiddeployqt need a real, static directory tree to merge into
        # the Gradle project at packaging time -- there's no mechanism to
        # inject one generated directory from the build tree into that merge
        # otherwise. The copied tree is gitignored (see .gitignore), same as
        # any other generated build artifact. A stamp file (rather than the
        # directory itself) is the custom command's OUTPUT, since
        # copy_directory has no single output file CMake could track.
        set(VIVACE_CPYTHON_STDLIB_DIR
            "${VIVACE_CPYTHON_SRC}/prefix/lib/python${VIVACE_CPYTHON_ABI_VERSION}")
        set(VIVACE_CPYTHON_STDLIB_ASSETS_DIR
            "${VIVACE_ANDROID_PACKAGE_SOURCE_DIR}/assets/python-stdlib")
        set(VIVACE_CPYTHON_STDLIB_STAMP
            "${CMAKE_BINARY_DIR}/cpython-android-stdlib.stamp")
        add_custom_command(
            OUTPUT "${VIVACE_CPYTHON_STDLIB_STAMP}"
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "${VIVACE_ANDROID_PACKAGE_SOURCE_DIR}/assets"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${VIVACE_CPYTHON_STDLIB_DIR}" "${VIVACE_CPYTHON_STDLIB_ASSETS_DIR}"
            COMMAND ${CMAKE_COMMAND} -E touch "${VIVACE_CPYTHON_STDLIB_STAMP}"
            DEPENDS cpython_android_ext
            COMMENT "Copying the embedded CPython stdlib into Android assets"
            VERBATIM
        )
        add_custom_target(cpython_android_stdlib_assets ALL
            DEPENDS "${VIVACE_CPYTHON_STDLIB_STAMP}")

        # A CA certificate bundle for the embedded interpreter's own OpenSSL
        # (libssl_python.so/libcrypto_python.so) to verify HTTPS connections
        # against -- real bug found 2026-09-15: yt-dlp's real network call to
        # YouTube's API failed with "[SSL: CERTIFICATE_VERIFY_FAILED]...
        # unable to get local issuer certificate". Root cause: unlike every
        # desktop platform (which has a real OS-provided cert store at a
        # well-known path OpenSSL's own default-verify-paths logic already
        # knows to check), Android has no such POSIX-visible store at all --
        # the system trust store is exposed only through Java APIs, invisible
        # to a plain C library. FIX: bundle curl.se's own canonical
        # redistribution of Mozilla's CA root list (the SAME data the
        # `certifi` PyPI package ships, just fetched directly rather than
        # depending on a pip install) as an asset, and set the SSL_CERT_FILE
        # environment variable (read directly by OpenSSL's own
        # X509_STORE_set_default_paths(), independent of anything Python's
        # own config.use_environment governs) to point at it once extracted
        # (see ensureCaCertBundle() in pythonyoutuberesolver.cpp).
        # file(DOWNLOAD) at CONFIGURE time (not an ExternalProject_Add step)
        # since this doesn't depend on cpython_android_ext at all -- guarded
        # by NOT EXISTS so a normal reconfigure doesn't re-fetch it every time.
        set(VIVACE_CACERT_ASSET "${VIVACE_ANDROID_PACKAGE_SOURCE_DIR}/assets/cacert.pem")
        if(NOT EXISTS "${VIVACE_CACERT_ASSET}")
            file(DOWNLOAD "https://curl.se/ca/cacert.pem" "${VIVACE_CACERT_ASSET}"
                STATUS _vivace_cacert_status)
            list(GET _vivace_cacert_status 0 _vivace_cacert_status_code)
            if(NOT _vivace_cacert_status_code EQUAL 0)
                file(REMOVE "${VIVACE_CACERT_ASSET}")
                message(WARNING "Could not download the CA certificate bundle for "
                                 "embedded-Python YouTube resolution "
                                 "(${_vivace_cacert_status}) -- HTTPS requests from "
                                 "that path will fail with a certificate-verify error "
                                 "until android/assets/cacert.pem is provided some "
                                 "other way.")
            endif()
        endif()

        message(STATUS "Embedded-Python YouTube resolution: ENABLED "
                        "(CPython ${VIVACE_CPYTHON_VERSION} for ${_vivace_cpython_host})")
    endif()
endif()

target_sources(vivace PRIVATE src/pythonyoutuberesolver.h src/pythonyoutuberesolver.cpp)
if(VIVACE_HAVE_PYTHON_YOUTUBE AND ANDROID)
    add_library(cpython_android_imported SHARED IMPORTED GLOBAL)
    set_target_properties(cpython_android_imported PROPERTIES
        IMPORTED_LOCATION "${VIVACE_CPYTHON_LIB}")
    add_dependencies(cpython_android_imported cpython_android_ext)

    target_compile_definitions(vivace PRIVATE
        VIVACE_HAVE_PYTHON_YOUTUBE=1
        VIVACE_CPYTHON_ABI_VERSION="${VIVACE_CPYTHON_ABI_VERSION}"
        # Tags the on-device extracted-stdlib marker file (see
        # extractedStdlibHomeDir()) with the exact bundled version, so a
        # STALE extraction left over in app-private storage from an
        # earlier install of a DIFFERENT build (e.g. one from before some
        # stdlib-affecting fix) is correctly invalidated and re-extracted,
        # rather than silently reused forever just because a marker file
        # happens to already exist there -- real bug found 2026-09-15,
        # "ModuleNotFoundError: No module named 'zipfile._path'" on a
        # device that had run an earlier, differently-built APK before.
        VIVACE_CPYTHON_VERSION="${VIVACE_CPYTHON_VERSION}")
    target_include_directories(vivace PRIVATE
        "${VIVACE_CPYTHON_SRC}/prefix/include/python${VIVACE_CPYTHON_ABI_VERSION}")
    target_link_libraries(vivace PRIVATE cpython_android_imported)

    # QT_ANDROID_EXTRA_LIBS is Qt's own documented mechanism for "copy this
    # prebuilt .so into the APK's libs folder and load it at startup" --
    # linking it above satisfies the LINKER, but does nothing to make
    # androiddeployqt actually ship the file inside the APK on its own.
    # Per Qt's own docs for this property: a prebuilt produced by an
    # ExternalProject_Add in this same build tree needs an explicit
    # add_dependencies(), or there's no guarantee the .so exists yet by the
    # time the APK deployment step runs.
    # cpython_android_stdlib_assets must finish before androiddeployqt reads
    # android/assets/ -- vivace itself is a reasonable proxy dependency for
    # "before packaging", matching how add_dependencies(vivace
    # cpython_android_ext) is used just above for the same reason.
    add_dependencies(vivace cpython_android_ext cpython_android_stdlib_assets)
    # libssl_python.so/libcrypto_python.so/libsqlite3_python.so are NOT
    # link-time dependencies of vivace's own .so (nothing in Vivace's own
    # C++ calls OpenSSL/SQLite APIs directly) -- so, unlike libpython3.14.so
    # above, ordinary ELF NEEDED-dependency resolution would never load them
    # at all. QT_ANDROID_EXTRA_LIBS's own documented behavior ("copied into
    # your application's libs folder AND LOADED ON START-UP... used to
    # enable OpenSSL") is exactly what's needed here: get them resident in
    # the process, under the app's normal/default linker namespace, before
    # anything else runs -- including the embedded interpreter's own later
    # dlopen() of _ssl.cpython-...so from app-private storage (which runs
    # under a DIFFERENT, more restricted linker namespace that can't find
    # its own dependencies there on its own; see the comment above
    # VIVACE_CPYTHON_SSL_LIB for the real on-device error this fixes).
    set_property(TARGET vivace APPEND PROPERTY
        QT_ANDROID_EXTRA_LIBS "${VIVACE_CPYTHON_LIB}" "${VIVACE_CPYTHON_SSL_LIB}"
            "${VIVACE_CPYTHON_CRYPTO_LIB}" "${VIVACE_CPYTHON_SQLITE_LIB}")
elseif(VIVACE_HAVE_PYTHON_YOUTUBE)
    # TEMPORARY desktop-test branch -- see the VIVACE_ENABLE_PYTHON_YOUTUBE
    # block above for why this exists. No asset extraction/packaging at
    # all: Python3::Python (from find_package(Python3 COMPONENTS
    # Development) above) already carries the correct include dirs and
    # import library for the real, already-installed Python this links
    # against; VIVACE_CPYTHON_DESKTOP_HOME tells pythonyoutuberesolver.cpp
    # to point PyConfig.home directly at that real install instead of
    # extracting anything from Android assets.
    target_compile_definitions(vivace PRIVATE
        VIVACE_HAVE_PYTHON_YOUTUBE=1
        VIVACE_CPYTHON_ABI_VERSION="${VIVACE_CPYTHON_ABI_VERSION}"
        VIVACE_CPYTHON_DESKTOP_HOME="${VIVACE_CPYTHON_DESKTOP_HOME}")
    target_link_libraries(vivace PRIVATE Python3::Python)
endif()

# YouTube "Download & play" mode on Android: needs a real ffmpeg (to merge
# yt-dlp's separately-downloaded video+audio streams -- QMediaPlayer can't
# play separate DASH streams) and a real JS runtime (yt-dlp's own
# --js-runtimes mechanism, needed for YouTube's signature/PoToken
# challenges on many videos). Both are built from source (see
# scripts/build-android-youtube-tools.sh for the full "why" -- a minimal
# static ffmpeg with no --disable-programs, since fftools/ffmpeg.c itself
# needs avfilter even for a plain -c copy remux; a thin node_main.cc-style
# wrapper around nodejs-mobile's libnode.so, since it ships no standalone
# "node" binary of its own) and bundled the SAME way libssl_python.so etc.
# already are for the embedded CPython work above -- named to end in
# ".so" and deployed via QT_ANDROID_EXTRA_LIBS, landing in the app's own
# nativeLibraryDir (part of the signed, installed package -- legal to
# exec() there, unlike app-private storage).
option(VIVACE_ENABLE_YOUTUBE_DOWNLOAD_TOOLS
    "Bundle a minimal ffmpeg + Node.js for YouTube Download & play mode (Android only)" ON)
set(VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS FALSE)
if(ANDROID AND VIVACE_HAVE_PYTHON_YOUTUBE AND VIVACE_ENABLE_YOUTUBE_DOWNLOAD_TOOLS)
    # Real bug found 2026-09-20 (a genuine Qt Creator build failure): a
    # plain find_program(bash) can match C:/Windows/System32/bash.exe --
    # present on the PATH of some Windows installs regardless of whether
    # Git for Windows is also installed -- which is not a suitable
    # interpreter for this script (it operates on a different filesystem
    # view than the plain Windows paths this custom command passes as
    # arguments, so the script is reported missing even though it exists).
    # HINTS are searched before the plain system PATH by find_program's own
    # documented search order, so listing Git for Windows' common install
    # locations here reliably prefers a real, working bash over that one
    # whenever Git for Windows is installed at one of them.
    find_program(VIVACE_BASH_EXECUTABLE bash
        HINTS
            "$ENV{ProgramFiles}/Git/bin"
            "$ENV{ProgramFiles}/Git/usr/bin"
            "C:/Program Files (x86)/Git/bin"
            "C:/Program Files (x86)/Git/usr/bin"
            "C:/Git/bin"
            "C:/Git/usr/bin"
    )
    # Defensive fallback: if none of the HINTS matched (Git for Windows
    # installed somewhere else entirely) and the plain PATH search found
    # the unsuitable System32 one anyway, reject it explicitly rather than
    # silently trying -- and failing -- to use it.
    if(VIVACE_BASH_EXECUTABLE)
        string(TOLOWER "${VIVACE_BASH_EXECUTABLE}" _vivace_bash_lower)
        if(_vivace_bash_lower MATCHES "system32")
            message(WARNING "YouTube Download & play tools: find_program(bash) "
                             "resolved to '${VIVACE_BASH_EXECUTABLE}', which cannot "
                             "run this script. Install Git for Windows, or set "
                             "-DVIVACE_BASH_EXECUTABLE=<path to a real bash.exe> "
                             "explicitly. Disabling for now.")
            unset(VIVACE_BASH_EXECUTABLE CACHE)
        endif()
    endif()
    if(NOT VIVACE_BASH_EXECUTABLE)
        message(WARNING "YouTube Download & play tools: no usable bash found to run "
                         "scripts/build-android-youtube-tools.sh -- disabling.")
    elseif(NOT DEFINED ANDROID_NDK)
        message(WARNING "YouTube Download & play tools: ANDROID_NDK not set by "
                         "the NDK's own toolchain file -- disabling.")
    else()
        set(VIVACE_YOUTUBE_TOOLS_OUT "${CMAKE_BINARY_DIR}/android-youtube-tools")
        set(VIVACE_YOUTUBE_TOOLS_ABI "${CMAKE_ANDROID_ARCH_ABI}")
        set(VIVACE_YOUTUBE_TOOLS_STAMP "${CMAKE_BINARY_DIR}/android-youtube-tools.stamp")
        add_custom_command(
            OUTPUT "${VIVACE_YOUTUBE_TOOLS_STAMP}"
            COMMAND "${VIVACE_BASH_EXECUTABLE}"
                "${CMAKE_SOURCE_DIR}/scripts/build-android-youtube-tools.sh"
                "${ANDROID_NDK}" "${VIVACE_YOUTUBE_TOOLS_OUT}" "${VIVACE_YOUTUBE_TOOLS_ABI}"
            COMMAND ${CMAKE_COMMAND} -E touch "${VIVACE_YOUTUBE_TOOLS_STAMP}"
            DEPENDS "${CMAKE_SOURCE_DIR}/scripts/build-android-youtube-tools.sh"
                    "${CMAKE_SOURCE_DIR}/src/android/vivace_node_main.cc"
            COMMENT "Building ffmpeg + Node.js for YouTube Download & play mode (${VIVACE_YOUTUBE_TOOLS_ABI})"
            USES_TERMINAL
            VERBATIM
        )
        add_custom_target(vivace_youtube_tools ALL DEPENDS "${VIVACE_YOUTUBE_TOOLS_STAMP}")
        set(VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS TRUE)
        message(STATUS "YouTube Download & play tools: ENABLED (building ffmpeg + "
                        "Node.js for ${VIVACE_YOUTUBE_TOOLS_ABI})")
    endif()
endif()

if(VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS)
    set(_vivace_yt_tools_dir "${VIVACE_YOUTUBE_TOOLS_OUT}/${VIVACE_YOUTUBE_TOOLS_ABI}")
    add_dependencies(vivace vivace_youtube_tools)
    # QT_ANDROID_EXTRA_LIBS: same documented "copied into the libs folder
    # AND LOADED ON START-UP" mechanism already used for the CPython/
    # OpenSSL libraries above. libffmpeg.so/libvivace_node.so are real
    # EXECUTABLES (not shared libraries) despite the extension AND the
    # "lib" prefix -- both are required by androiddeployqt itself (a real
    # error hit wiring this up: "must begin with \"lib\" and end with the
    # suffix \".so\""), but Qt's own deployment step doesn't care what's
    # actually INSIDE the file, it just copies whatever is listed here
    # into the APK's lib/<abi>/ directory, which is exactly the
    # nativeLibraryDir location a real exec() is legal from. libnode.so/
    # libc++_shared.so MUST keep those exact names (not renamed further)
    # -- see build-android-youtube-tools.sh's own comment on why.
    set_property(TARGET vivace APPEND PROPERTY
        QT_ANDROID_EXTRA_LIBS
            "${_vivace_yt_tools_dir}/libffmpeg.so"
            "${_vivace_yt_tools_dir}/libvivace_node.so"
            "${_vivace_yt_tools_dir}/libnode.so"
            "${_vivace_yt_tools_dir}/libc++_shared.so")
    target_compile_definitions(vivace PRIVATE VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS=1)
endif()

# TEMPORARY desktop-test convenience (2026-09-19, user request): a plain
# cache-variable pair letting a developer point Vivace at LOCAL,
# already-built copies of these same tools (e.g. this project's own
# scripts/build-android-youtube-tools.sh output, or just a system ffmpeg/
# node on PATH) to exercise the SAME "bundled external tool" code path on
# desktop without needing a real Android device for every iteration. Not
# meant to ship real desktop behavior -- desktop already has its own,
# separate, user-configurable ffmpeg/Deno paths via YoutubeResolver's own
# Settings (ytdlPath, ffmpegLocation, denoLocation).
set(VIVACE_YOUTUBE_DOWNLOAD_TOOLS_DESKTOP_FFMPEG "" CACHE FILEPATH
    "Dev/debug only: path to a local ffmpeg executable, used to test the Android-style bundled-tool code path on desktop")
set(VIVACE_YOUTUBE_DOWNLOAD_TOOLS_DESKTOP_NODE "" CACHE FILEPATH
    "Dev/debug only: path to a local node executable, used to test the Android-style bundled-tool code path on desktop")
if(NOT ANDROID AND VIVACE_YOUTUBE_DOWNLOAD_TOOLS_DESKTOP_FFMPEG AND VIVACE_YOUTUBE_DOWNLOAD_TOOLS_DESKTOP_NODE)
    set(VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS TRUE)
    target_compile_definitions(vivace PRIVATE
        VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS=1
        VIVACE_YOUTUBE_TOOLS_DESKTOP_FFMPEG="${VIVACE_YOUTUBE_DOWNLOAD_TOOLS_DESKTOP_FFMPEG}"
        VIVACE_YOUTUBE_TOOLS_DESKTOP_NODE="${VIVACE_YOUTUBE_DOWNLOAD_TOOLS_DESKTOP_NODE}")
    message(STATUS "YouTube Download & play tools: ENABLED (TEMPORARY desktop "
                    "test build, ffmpeg=${VIVACE_YOUTUBE_DOWNLOAD_TOOLS_DESKTOP_FFMPEG} "
                    "node=${VIVACE_YOUTUBE_DOWNLOAD_TOOLS_DESKTOP_NODE})")
endif()

install(TARGETS vivace
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    # On Android, qt_add_executable() produces a MODULE_LIBRARY (a shared
    # object the Java activity wrapper loads via JNI, not a true executable),
    # so install(TARGETS) needs a LIBRARY DESTINATION or configure fails
    # outright ("no LIBRARY DESTINATION for module target"). A no-op on
    # Windows/macOS/Linux -- each DESTINATION clause only applies to the
    # target's actual output type, and none of those produce a MODULE_LIBRARY.
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
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
