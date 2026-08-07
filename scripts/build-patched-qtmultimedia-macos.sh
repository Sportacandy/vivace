#!/usr/bin/env bash
# Builds qtmultimedia from source (tag v6.11.1, all patches/*.patch applied)
# against an ALREADY-INSTALLED official Qt kit, in place -- overwriting that
# kit's own QtMultimedia.framework, QtMultimediaQuick.framework, and
# plugins/multimedia/libffmpegmediaplugin.dylib. Also deploys the matching
# dav1d-enabled FFmpeg runtime .dylib's these patched libraries link against.
#
# Building THIS WAY -- against the exact Qt kit that will actually run it,
# rather than a separately-built Qt tree whose binaries get copied over
# afterward -- is the whole point: it guarantees the patched Multimedia/
# MultimediaQuick are ABI-consistent with that same kit's own Gui/Quick/
# QuickTemplates2/..., since they are compiled against its own installed
# headers and CMake package config, not a different Qt build that merely
# shares a version NUMBER. See scripts/build-patched-qtmultimedia-linux.sh's
# header comment and CLAUDE.md's 2026-08-06 entries for why this approach
# replaced a prebuilt-asset swap on Linux/Windows; this script brings macOS
# onto the same footing (previously macOS CI only swapped in an AV1-capable
# FFmpeg, with none of the three patches actually applied -- see README.md's
# "macOS is still unwired" notes, now stale once this is verified working).
#
# UNVERIFIED as of 2026-08-07: written by porting the Linux/Windows scripts'
# already-hard-won fixes (double cmake configure, Vulkan-Headers stub) to
# macOS's toolchain, but never actually run -- there is no macOS machine in
# this development environment. Expect at least one more round of fixes
# once this is tried for real in CI.
#
# Usage: build-patched-qtmultimedia-macos.sh <QT_ROOT_DIR> [<repo-root>]
#   QT_ROOT_DIR: an aqtinstall/install-qt-action (or equivalent) Qt 6.11.1
#                clang_64 kit that already has the qtmultimedia module
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

if ! command -v ninja >/dev/null 2>&1; then
    echo "== Ninja not found -- installing via brew =="
    brew install ninja
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

# Homebrew's ffmpeg@7 is the AV1 (libdav1d)-capable FFmpeg this project
# already uses for macOS (see the now-superseded "Swap in an AV1-capable
# FFmpeg" step this script replaces) -- its prefix has the same
# include/+lib/ layout FFMPEG_DIR expects, matching the BtbN archives used
# on Windows/Linux.
echo "== Installing dav1d-enabled FFmpeg (Homebrew ffmpeg@7) =="
brew install ffmpeg@7
FFMPEG7="$(brew --prefix ffmpeg@7)"

echo "== Cloning qtmultimedia v6.11.1 =="
git clone --branch v6.11.1 --depth 1 https://github.com/qt/qtmultimedia.git "$WORK/qtmultimedia"

echo "== Applying patches =="
for p in "${PATCH_FILES[@]}"; do
    echo "  $(basename "$p")"
    git -C "$WORK/qtmultimedia" apply "$p"
done

echo "== Configuring + building qtmultimedia against $QTDIR =="
# Plain cmake against CMAKE_PREFIX_PATH, not qt-configure-module -- see
# build-patched-qtmultimedia-linux.sh's own comment for why (qt-configure-
# module's own argument parser only forwards raw -D... flags to the real
# cmake invocation after a literal "--" separator, confirmed on Windows CI
# 2026-08-06).
#
# qvideowindow.cpp can fail with "'QRhiVulkanInitParams': undeclared
# identifier" without a real fix here (hit on Windows CI, 2026-08-06; see
# that script's comment for the full root-cause explanation -- QtGui's own
# QT_CONFIG(vulkan) flag is baked in from official-build time and can't be
# toggled from qtmultimedia's side, but qtbase's own qrhi_platform.h ALSO
# requires __has_include(<vulkan/vulkan.h>) to succeed, freshly, right now).
# macOS's own RHI backend is Metal, not Vulkan, so QtGui's official mac
# build most likely has QT_CONFIG(vulkan) off already and this may turn out
# to be unnecessary here -- included defensively anyway since it's cheap
# and harmless if unneeded.
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
    -DFFMPEG_DIR="$FFMPEG7"
    -DQT_DEPLOY_FFMPEG=TRUE
    -DCMAKE_BUILD_TYPE=Release
    -DQT_SYNC_HEADERS_AT_CONFIGURE_TIME=ON
)
cmake "${CMAKE_CONFIGURE_ARGS[@]}"

# Configure TWICE (identical args, same build dir): a generated private
# config header like QtMultimedia/private/qtmultimedia-config_p.h is
# written via file(GENERATE), deferred to the end of a cmake invocation's
# own generate phase -- after QT_SYNC_HEADERS_AT_CONFIGURE_TIME's own
# syncqt pass (which runs during the earlier configure phase, not generate)
# would already have looked for it and found nothing. Confirmed on Windows
# CI, 2026-08-06 -- see that script's comment for the full explanation. A
# second, separate cmake invocation against the SAME build dir starts with
# the first one's generate-phase output already on disk.
cmake "${CMAKE_CONFIGURE_ARGS[@]}"

# Retry once on failure anyway, as a harmless safety net for a genuine
# from-scratch Ninja scheduling race (some parallel compiles can be
# scheduled before an otherwise-correct generated-header dependency
# finishes on a first pass, even though it succeeds moments later): a
# second invocation is a plain incremental Ninja build, so anything already
# built is skipped and only the previously-failed files are retried.
if ! cmake --build "$WORK/build" --parallel; then
    echo "First build pass failed (likely a generated-header race on a from-scratch Ninja build); retrying once..."
    cmake --build "$WORK/build" --parallel
fi
cmake --install "$WORK/build"

echo "== Deploying dav1d FFmpeg runtime libraries into $QTDIR/lib =="
for dylib in libavcodec.61.dylib libavformat.61.dylib libavutil.59.dylib libswresample.5.dylib libswscale.8.dylib; do
    cp "$FFMPEG7/lib/$dylib" "$QTDIR/lib/$dylib"
done

echo "Done: $QTDIR now has the patched qtmultimedia + dav1d FFmpeg installed."
