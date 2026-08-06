#!/usr/bin/env bash
# Packages the patched Qt Multimedia binaries (AV1 hwaccel-disable +
# WSOLA pitch-compensation + subtitle-bitmap-display patches applied,
# see patches/*.patch) into qt-patched-prebuilt-linux64.tar.gz, the
# asset release.yml/nightly.yml download and swap into an official Qt
# install on CI (see the "Swap in the patched qtmultimedia AV1 + Speed
# Control fix (prebuilt)" step there).
#
# Run this after building qtmultimedia with the patches applied and
# installing it over the gcc_64 kit (or pass that install's root as $1).
# IMPORTANT (2026-08-06): build qtmultimedia against the *exact* official
# aqtinstall-fetched Qt 6.11.1 linux_gcc_64 kit that release.yml/
# nightly.yml install (same version/arch, same aqtinstall fork/commit
# pinned there) -- not a separately-built full custom Qt tree. Qt6Gui/
# Qt6Quick/Qt6QuickTemplates2/etc. are NOT swapped by this script or by
# CI (only Multimedia/MultimediaQuick/the plugin/FFmpeg are), so the
# patched Multimedia/MultimediaQuick must already be ABI-compatible with
# whatever official Gui/Quick/QuickTemplates2/... is already installed
# on the target machine. A same-day attempt to instead bundle just
# libQt6Gui.so*/libQt6Quick.so* alongside the patched Multimedia
# (reasoning: QRhi/QVideoTextureHelper/etc. are private, unstable APIs
# Multimedia calls into Gui/Quick) was tried and made things WORSE on a
# real Ubuntu 24.04 test: the target's still-official
# libQt6QuickTemplates2.so.6 (unswapped) then failed to find a private
# virtual-thunk symbol in the newly-swapped-in libQt6Quick.so, and the
# app wouldn't even start (a plain "symbol lookup error", not a runtime
# crash). Swapping part of the Gui/Quick/Declarative stack just moves the
# private-ABI mismatch to a different pair of modules -- the only
# structurally sound fix is building Multimedia against the SAME Gui/
# Quick/QuickTemplates2/... binaries it will actually run against, not
# bundling a partial, inconsistent subset of them.
#
# Windows equivalent: scripts/zip-patched-mmplugin.bat (same file set,
# .zip instead of .tar.gz).

set -euo pipefail

QTDIR="${1:-/opt/Qt/6.11.1/gcc_64}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTTAR="$SCRIPT_DIR/../qt-patched-prebuilt-linux64.tar.gz"

if ! compgen -G "$QTDIR/lib/libQt6Multimedia.so*" > /dev/null; then
    echo "ERROR: \"$QTDIR\" does not look like a Qt install (libQt6Multimedia.so not found)." >&2
    exit 1
fi

# libffmpegmediaplugin.so: patched by all three patches (AV1 decoder
# selection, WSOLA pitch compensation, and the subtitle decoder fix).
# libavcodec/libavformat/libavutil/libswresample/libswscale: the
# dav1d-enabled FFmpeg build the AV1 patch requires (soname-61 family,
# matching Qt's own official FFmpeg build's ABI so it's a drop-in
# replacement -- see the AV1 patch notes for why this specific line was
# chosen).
PLUGIN_FILES=(libffmpegmediaplugin.so)
FFMPEG_SONAMES=(
    libavcodec.so.61
    libavformat.so.61
    libavutil.so.59
    libswresample.so.5
    libswscale.so.8
)

for f in "${PLUGIN_FILES[@]}"; do
    if [ ! -f "$QTDIR/plugins/multimedia/$f" ]; then
        echo "ERROR: missing file: $QTDIR/plugins/multimedia/$f" >&2
        exit 1
    fi
done

for f in "${FFMPEG_SONAMES[@]}"; do
    if [ ! -f "$QTDIR/lib/$f" ]; then
        echo "ERROR: missing file: $QTDIR/lib/$f" >&2
        exit 1
    fi
done

# libQt6Multimedia.so* / libQt6MultimediaQuick.so*: patched by
# qtmultimedia-subtitle-bitmap.patch (qvideoframe.cpp,
# qplatformvideosink.cpp, qvideosink.cpp live in the Multimedia module;
# qsgvideonode_p.cpp lives in MultimediaQuick). Globbed rather than a
# single exact filename because Qt installs each of these as a small
# family of symlinks (libQt6Multimedia.so -> .so.6 -> .so.6.11.1, ...) --
# grabbing the whole family keeps that symlink structure intact instead
# of guessing which single name the CI Qt tree's loader actually wants.
# Deliberately NOT joined by libQt6Gui.so*/libQt6Quick.so* -- see the
# 2026-08-06 note at the top of this file for why a same-day attempt to
# do that made things worse, not better.
shopt -s nullglob
QT_MODULE_FILES=("$QTDIR"/lib/libQt6Multimedia.so* "$QTDIR"/lib/libQt6MultimediaQuick.so*)
shopt -u nullglob

if [ "${#QT_MODULE_FILES[@]}" -eq 0 ]; then
    echo "ERROR: no libQt6Multimedia*.so files found under $QTDIR/lib" >&2
    exit 1
fi

QT_MODULE_BASENAMES=()
for f in "${QT_MODULE_FILES[@]}"; do
    QT_MODULE_BASENAMES+=("$(basename "$f")")
done

# Assembled in a staging directory rather than tarred straight out of the Qt
# install, for its per-file symlink handling, which a single tar call cannot
# express:
#
# - FFmpeg: those five names are SONAMEs, and in an FFmpeg install every one
#   of them is a symlink to the real, fully-versioned library
#   (libavcodec.so.61 -> libavcodec.so.61.19.101 -- and the suffix differs per
#   library even within one install, e.g. libavformat.so.61 ->
#   libavformat.so.61.7.103). tar stores a symlink as a symlink, so archiving
#   the SONAMEs directly produced a tarball holding five dangling links and
#   not one byte of library body. Staged dereferenced instead, so each SONAME
#   is a regular library body -- matching what the README's "Linux quick
#   path" tells people to copy (it names only these five, never the versioned
#   bodies). Costs nothing in size, and the runtime loader only ever looks a
#   library up by SONAME.
#
# - Qt modules: staged with -d, keeping the .so -> .so.6 -> .so.6.11.1 link
#   family intact. Dereferencing these would be wrong twice over: three full
#   copies of the same library, and extracting over a Qt install would turn
#   .so.6 into a regular file while leaving the install's old .so.6.11.1 body
#   in place with .so still pointing at it.
#
# The archive itself is flat (members at the root, no wrapping directory) --
# deliberately, to match scripts/zip-patched-mmplugin.bat's .zip exactly (same
# file set, same layout, just .tar.gz instead of .zip) and the README's copy
# instructions, which name individual files with no subdirectory step. Staged
# with hard links where the filesystem allows it, so this does not copy
# ~130 MB around just to repackage it.
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT
PKGDIR="$STAGE/pkg"
mkdir -p "$PKGDIR"

# stage <src> <dst> [extra cp flags]; -d keeps a symlink as a symlink, no -d
# resolves it to the body it points at.
stage() {
    local src="$1" dst="$2"; shift 2
    cp -l "$@" "$src" "$dst" 2>/dev/null || cp "$@" "$src" "$dst"
}

for f in "${PLUGIN_FILES[@]}"; do
    stage "$QTDIR/plugins/multimedia/$f" "$PKGDIR/$f"
done
for f in "${FFMPEG_SONAMES[@]}"; do
    stage "$QTDIR/lib/$f" "$PKGDIR/$f"          # dereferenced: SONAME -> body
done
for f in "${QT_MODULE_BASENAMES[@]}"; do
    stage "$QTDIR/lib/$f" "$PKGDIR/$f" -d       # symlink family preserved
done

# The FFmpeg members must be library bodies, not links: that is the layout the
# published asset has and the README's copy instructions assume.
for f in "${FFMPEG_SONAMES[@]}"; do
    if [ -L "$PKGDIR/$f" ] || [ ! -s "$PKGDIR/$f" ]; then
        echo "ERROR: $f was staged as a symlink or is empty, not a library body" >&2
        exit 1
    fi
done

# No dangling links, i.e. every symlink resolves to something inside the
# package. This is exactly how this script used to fail, and it failed
# silently: tar is perfectly happy to archive a dangling link, and the
# resulting tarball still looks plausible until something tries to load
# from it.
if find "$PKGDIR" -xtype l | grep -q .; then
    echo "ERROR: package contains symlinks with no target inside it:" >&2
    find "$PKGDIR" -xtype l -printf '  %f -> %l\n' >&2
    exit 1
fi

tar -czf "$STAGE/out.tar.gz" -C "$PKGDIR" \
    "${PLUGIN_FILES[@]}" "${FFMPEG_SONAMES[@]}" "${QT_MODULE_BASENAMES[@]}"

# Only publish once the checks above have passed, so a failed run never leaves
# a half-valid tarball where the previous good one used to be.
mv "$STAGE/out.tar.gz" "$OUTTAR"

echo "Created $OUTTAR"
tar -tzvf "$OUTTAR"
