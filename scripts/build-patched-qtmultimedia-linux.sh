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
curl -L -o "$WORK/ffmpeg.tar.xz" \
    "https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-n7.1-latest-linux64-gpl-shared-7.1.tar.xz"
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
cmake -S "$WORK/qtmultimedia" -B "$WORK/build" \
    -G Ninja \
    -DCMAKE_PREFIX_PATH="$QTDIR" \
    -DCMAKE_INSTALL_PREFIX="$QTDIR" \
    -DFFMPEG_DIR="$WORK/ffmpeg" \
    -DQT_DEPLOY_FFMPEG=TRUE \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "$WORK/build" --parallel
cmake --install "$WORK/build"

echo "== Deploying dav1d FFmpeg runtime libraries into $QTDIR/lib =="
for so in libavcodec.so.61 libavformat.so.61 libavutil.so.59 libswresample.so.5 libswscale.so.8; do
    cp -P "$WORK/ffmpeg/lib/$so" "$QTDIR/lib/$so"
done

echo "Done: $QTDIR now has the patched qtmultimedia + dav1d FFmpeg installed."
