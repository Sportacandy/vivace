#!/usr/bin/env bash
# Builds qtmultimedia from source (tag v6.11.1, all patches/*.patch applied)
# against an ALREADY-INSTALLED official Qt kit, in place -- overwriting that
# kit's own libQt6Multimedia*.so*, libQt6MultimediaQuick*.so*, and
# plugins/multimedia/libffmpegmediaplugin.so. Also deploys the matching
# dav1d-enabled FFmpeg runtime .so's these patched libraries link against.
#
# Building THIS WAY -- against the exact Qt kit that will actually run it,
# rather than a separately-built Qt tree whose binaries get copied over
# afterward -- is the whole point: it guarantees the patched Multimedia/
# MultimediaQuick are ABI-consistent with that same kit's own Gui/Quick/
# QuickTemplates2/..., since they are compiled against its own installed
# headers and CMake package config, not a different Qt build that merely
# shares a version NUMBER. See CLAUDE.md's 2026-08-06 entries for two ways
# of doing this the OTHER way (a prebuilt asset built on a different
# machine; later, partially swapping in just libQt6Gui/libQt6Quick) that
# both went wrong on a real Ubuntu 24.04 install -- one with a silent
# private-ABI crash (garbage pointer inside QRhi::isTextureFormatSupported),
# the other with the app refusing to even start (a QQuickTemplates2 private
# symbol lookup error). This script replaces both of those approaches for
# CI (see release.yml/nightly.yml's Linux jobs) and can also be run by hand
# for local development.
#
# Usage: build-patched-qtmultimedia-linux.sh <QT_ROOT_DIR> [<repo-root>]
#   QT_ROOT_DIR: an aqtinstall/install-qt-action (or equivalent) Qt 6.11.1
#                linux_gcc_64 kit that already has the qtmultimedia module
#                installed -- this REBUILDS qtmultimedia in place, it does
#                not add the module from nothing, and needs the kit's dev
#                tools (CMake package config, private headers) present.
#   repo-root:   the Vivace checkout containing patches/*.patch (defaults
#                to this script's own parent directory's parent, i.e. the
#                repo this script lives in).

set -euo pipefail

QTDIR="${1:?usage: $0 <QT_ROOT_DIR> [<repo-root>]}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="${2:-$SCRIPT_DIR/..}"

if [ ! -d "$QTDIR/lib/cmake/Qt6" ]; then
    echo "ERROR: $QTDIR/lib/cmake/Qt6 not found -- is \"$QTDIR\" a Qt install with dev tools?" >&2
    exit 1
fi

shopt -s nullglob
PATCH_FILES=("$REPO_ROOT"/patches/qtmultimedia-*.patch)
shopt -u nullglob
if [ "${#PATCH_FILES[@]}" -eq 0 ]; then
    echo "ERROR: no patches/qtmultimedia-*.patch found under $REPO_ROOT" >&2
    exit 1
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

echo "== Downloading dav1d-enabled FFmpeg (BtbN n7.1, matches Qt's own FFmpeg soname family) =="
# BtbN publishes separate x86_64 ("linux64") and arm64 ("linuxarm64")
# archives, built from the same n7.1 tag -- soname versions (avcodec.so.61
# etc.) come from the FFmpeg source version, not the architecture, so both
# match Qt's own FFmpeg regardless of which one this script downloads.
case "$(uname -m)" in
    x86_64)  FFMPEG_ARCH="linux64" ;;
    aarch64) FFMPEG_ARCH="linuxarm64" ;;
    *) echo "ERROR: unsupported architecture $(uname -m)" >&2; exit 1 ;;
esac
curl -L -o "$WORK/ffmpeg.tar.xz" \
    "https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-n7.1-latest-${FFMPEG_ARCH}-gpl-shared-7.1.tar.xz"
mkdir "$WORK/ffmpeg"
tar -xJf "$WORK/ffmpeg.tar.xz" -C "$WORK/ffmpeg" --strip-components=1

echo "== Cloning qtmultimedia v6.11.1 =="
git clone --branch v6.11.1 --depth 1 https://github.com/qt/qtmultimedia.git "$WORK/qtmultimedia"

echo "== Applying patches =="
for p in "${PATCH_FILES[@]}"; do
    echo "  $(basename "$p")"
    git -C "$WORK/qtmultimedia" apply "$p"
done

echo "== Configuring + building qtmultimedia against $QTDIR =="
# Plain cmake against CMAKE_PREFIX_PATH, not qt-configure-module: the
# latter is a convenience wrapper (qtbase's QtProcessConfigureArgs.cmake)
# with its OWN argument parser that only forwards raw -D... flags to the
# real cmake invocation after a literal "--" separator -- confirmed by
# reading that script directly after CI hit "Unknown command line option
# '-DFFMPEG_DIR=...'" (2026-08-06). Configuring qtmultimedia as a plain
# standalone CMake project (find_package(Qt6 ...) resolves it via
# CMAKE_PREFIX_PATH, same as any other out-of-tree Qt module build) skips
# that wrapper and its parsing quirks entirely -- -D flags are always
# unambiguous on a real cmake command line.
# qvideowindow.cpp fails with "'QRhiVulkanInitParams': undeclared
# identifier" without a real fix here. Root cause (read qtbase's own
# src/gui/rhi/qrhi_platform.h directly): that struct is declared in the
# ALREADY-INSTALLED QtGui headers only under
# `#if QT_CONFIG(vulkan) && __has_include(<vulkan/vulkan.h>)`.
# QT_CONFIG(vulkan) here is QtGui's own value, permanently baked into its
# installed config header at official-Qt-release build time (not something
# qtmultimedia can toggle -- confirmed: qtmultimedia has no "vulkan"
# feature of its own at all, grep of its configure.cmake finds nothing, so
# an earlier -DFEATURE_vulkan=OFF attempt was silently ignored by CMake,
# "Manually-specified variables were not used by the project"). __has_include
# IS evaluated fresh, right now, against this build's own include path, and
# a machine with no Vulkan SDK installed fails that check, so the struct is
# never declared -- yet qvideowindow.cpp still tries to reference it, since
# ITS OWN QT_CONFIG(vulkan) check (the same baked-in QtGui value) says
# vulkan is available. Since Vivace doesn't use this optional Vulkan RHI
# init path and we never need it to actually run, just satisfying the
# compiler is enough: clone the tiny, header-only KhronosGroup/
# Vulkan-Headers repo and add its include dir to every C/C++ compile via
# CMAKE_C_FLAGS/CMAKE_CXX_FLAGS, so __has_include finally succeeds and the
# struct gets declared like it would on a machine with the real SDK.
echo "== Cloning Vulkan-Headers (compiler needs vulkan/vulkan.h to exist, never used at runtime) =="
git clone --depth 1 https://github.com/KhronosGroup/Vulkan-Headers.git "$WORK/Vulkan-Headers"
VULKAN_INCLUDE_FLAG="-I$WORK/Vulkan-Headers/include"

CMAKE_CONFIGURE_ARGS=(
    -S "$WORK/qtmultimedia" -B "$WORK/build"
    -G Ninja
    "-DCMAKE_C_FLAGS=$VULKAN_INCLUDE_FLAG"
    "-DCMAKE_CXX_FLAGS=$VULKAN_INCLUDE_FLAG"
    -DCMAKE_PREFIX_PATH="$QTDIR"
    -DCMAKE_INSTALL_PREFIX="$QTDIR"
    -DFFMPEG_DIR="$WORK/ffmpeg"
    -DQT_DEPLOY_FFMPEG=TRUE
    -DCMAKE_BUILD_TYPE=Release
    -DQT_SYNC_HEADERS_AT_CONFIGURE_TIME=ON
)
cmake "${CMAKE_CONFIGURE_ARGS[@]}"

# Configure TWICE (identical args, same build dir): a generated private
# config header like QtMultimedia/private/qtmultimedia-config_p.h is
# written via file(GENERATE), which -- unlike a plain configure_file() call
# made during ordinary CMakeLists.txt processing -- is deferred to the very
# end of a cmake invocation's own generate phase. If syncqt's
# QT_SYNC_HEADERS_AT_CONFIGURE_TIME pass (which builds each module's header
# alias during the EARLIER, ordinary configure phase, not generate) runs
# before that write happens, it still won't see the real file, for exactly
# the same reason a build-time ninja edge can miss it (qtbase's own
# src/tools/syncqt/main.cpp: silently skips creating the alias if the real
# file doesn't exist yet, relying on being re-run later -- which a single
# cmake invocation's own configure-time pass never gets to do for itself;
# confirmed on the Windows side of this same build, 2026-08-06: with only
# ONE configure, retrying the BUILD twice reproduced the exact same failing
# files both times). A second, separate cmake invocation against the SAME
# build dir starts with that first invocation's generate-phase output
# already on disk, so ITS configure-time syncqt pass can see the real file
# and create the alias correctly.
cmake "${CMAKE_CONFIGURE_ARGS[@]}"

# Retry once on failure anyway, as a harmless safety net for the unrelated,
# genuine from-scratch Ninja scheduling race described where this retry was
# first added (some parallel compiles can be scheduled before an otherwise-
# correct generated-header dependency finishes on a first pass, even though
# it succeeds moments later): a second invocation is a plain incremental
# Ninja build, so anything already built is skipped and only the
# previously-failed files are retried.
if ! cmake --build "$WORK/build" --parallel; then
    echo "First build pass failed (likely a generated-header race on a from-scratch Ninja build); retrying once..."
    cmake --build "$WORK/build" --parallel
fi
cmake --install "$WORK/build"

echo "== Deploying dav1d FFmpeg runtime libraries into $QTDIR/lib =="
for so in libavcodec.so.61 libavformat.so.61 libavutil.so.59 libswresample.so.5 libswscale.so.8; do
    cp -P "$WORK/ffmpeg/lib/$so" "$QTDIR/lib/$so"
done

echo "Done: $QTDIR now has the patched qtmultimedia + dav1d FFmpeg installed."
