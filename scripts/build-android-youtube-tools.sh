#!/usr/bin/env bash
# Builds the two external tools YouTube "Download & play" mode needs on
# Android: a minimal, static, remux-only ffmpeg CLI (merges yt-dlp's
# separately-downloaded video+audio streams via `-c copy`, no re-encoding,
# so no codec libraries beyond ffmpeg's own built-in muxers/demuxers/
# parsers/bsfs are needed) and a real "node" executable wrapping
# nodejs-mobile's libnode.so (needed so yt-dlp's own --js-runtimes
# mechanism, built around spawning a real node/deno/bun binary, works
# unmodified against a bundled tool -- see this project's own CLAUDE.md
# history, 2026-09-19, for the full "why not Deno" / "why not QuickJS"
# reasoning).
#
# Both are placed under the app's own native-library directory at package
# time (see CMakeLists.txt's VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS block),
# NOT app-private storage -- Android's noexec policy (the same one that
# forced the embedded-Python approach for yt-dlp itself) specifically
# targets app-private data directories (filesDir/cacheDir); a real
# executable ELF file bundled the normal way (named to end in ".so", the
# standard, widely-used trick) and placed in ApplicationInfo.
# nativeLibraryDir is legal to exec() there, since it's part of the signed,
# installed package -- exactly the same reasoning that already lets Vivace
# dlopen() libssl_python.so/libcrypto_python.so from that same directory
# for the embedded CPython interpreter.
#
# ffmpeg's own configure is a real autotools-style shell script (not
# CMake/Meson), so this is a plain bash script (matching this project's own
# established pattern for external-tool builds, e.g.
# build-patched-qtmultimedia-linux.sh) rather than a CMake
# ExternalProject_Add with inline shell commands.
#
# Usage: build-android-youtube-tools.sh <ANDROID_NDK_ROOT> <output-dir> [abi ...]
#   ANDROID_NDK_ROOT: e.g. C:/Android/Sdk/ndk/26.1.10909125 (Windows path OK
#                     under Git Bash -- converted internally as needed).
#   output-dir:       where per-ABI subdirectories (arm64-v8a, x86_64, ...)
#                     of the built/staged binaries are written.
#   abi:              one or more of arm64-v8a, x86_64 (default: arm64-v8a).
#
# UNVERIFIED beyond compiling/linking/ELF-header inspection on this
# Windows dev machine (no Android device/emulator here) -- needs the
# user's own device test, same division of labor as every other
# Android-native addition in this project's history.

set -euo pipefail

NDK_ROOT="${1:?usage: $0 <ANDROID_NDK_ROOT> <output-dir> [abi ...]}"
OUT_ROOT="${2:?usage: $0 <ANDROID_NDK_ROOT> <output-dir> [abi ...]}"
shift 2
ABIS=("$@")
if [ "${#ABIS[@]}" -eq 0 ]; then
    ABIS=("arm64-v8a")
fi

API=28  # matches this project's qtMinSdkVersion (see android/gradle.properties)
FFMPEG_TAG="n7.1"  # matches the version this project's other FFmpeg work already pins to
NODEJS_MOBILE_VERSION="v18.20.4"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORK_DIR="$OUT_ROOT/_work"
mkdir -p "$WORK_DIR" "$OUT_ROOT"

# ffmpeg's own configure needs a real TMPDIR it can create/exec files under
# -- a bare Windows-style TEMP/TMP (e.g. "C:\Temp") breaks its internal
# mktemp-style path handling under Git Bash (a real failure hit building
# this script: "Unable to create and execute files in C:\Temp... Sanity
# test failed"). Force a plain POSIX-style path.
export TMPDIR="$WORK_DIR/tmp"
export TEMP="$TMPDIR"
export TMP="$TMPDIR"
mkdir -p "$TMPDIR"

# ffmpeg's configure ALSO probes a HOST compiler (to build+run small
# feature-test programs during configure itself, and a couple of tiny
# host-side code generators) independently of the CROSS compiler -- if
# left to its own default ("cc"/"gcc" on PATH), configure fails with "Host
# compiler lacks C11 support" when only MSVC/NDK-cross-clang are on PATH.
# Needs a real, NATIVE-host-runnable C compiler (NOT the NDK's own
# Android-targeting clang -- test programs configure runs must execute on
# THIS machine). Auto-detect one; override by exporting HOST_CC yourself
# before running this script if none of these guesses fit your machine.
if [ -z "${HOST_CC:-}" ]; then
    for candidate in "/c/LLVM/bin/clang.exe" "$(command -v clang 2>/dev/null || true)" \
                      "$(command -v gcc 2>/dev/null || true)" "$(command -v cc 2>/dev/null || true)"; do
        if [ -n "$candidate" ] && [ -x "$candidate" ]; then
            HOST_CC="$candidate"
            break
        fi
    done
fi
if [ -z "${HOST_CC:-}" ]; then
    echo "ERROR: no native host C compiler found for ffmpeg's own configure-time" >&2
    echo "       feature tests. Set HOST_CC to one (e.g. a real Windows clang.exe" >&2
    echo "       or gcc), not the NDK's own Android-targeting cross compiler." >&2
    exit 1
fi
echo "Using host compiler for ffmpeg's configure-time probes: $HOST_CC"

TOOLCHAIN="$NDK_ROOT/toolchains/llvm/prebuilt/windows-x86_64"
if [ ! -d "$TOOLCHAIN" ]; then
    # Non-Windows NDK layout (Linux/macOS host) -- same NDK, different
    # prebuilt-toolchain subdirectory name.
    for candidate in "$NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64" \
                      "$NDK_ROOT/toolchains/llvm/prebuilt/darwin-x86_64"; do
        if [ -d "$candidate" ]; then
            TOOLCHAIN="$candidate"
            break
        fi
    done
fi
if [ ! -d "$TOOLCHAIN" ]; then
    echo "ERROR: no NDK LLVM toolchain found under $NDK_ROOT/toolchains/llvm/prebuilt/*" >&2
    exit 1
fi

# --- Fetch ffmpeg source once, shared across ABIs ---------------------------
FFMPEG_SRC="$WORK_DIR/ffmpeg-src"
if [ ! -d "$FFMPEG_SRC" ]; then
    echo "Cloning FFmpeg $FFMPEG_TAG..."
    git clone --branch "$FFMPEG_TAG" --depth 1 https://github.com/FFmpeg/FFmpeg.git "$FFMPEG_SRC"
fi

# --- Fetch nodejs-mobile's Android release once, shared across ABIs --------
NODEJS_MOBILE_DIR="$WORK_DIR/nodejs-mobile"
if [ ! -d "$NODEJS_MOBILE_DIR" ]; then
    echo "Downloading nodejs-mobile $NODEJS_MOBILE_VERSION (Android)..."
    mkdir -p "$NODEJS_MOBILE_DIR"
    curl -sL -o "$WORK_DIR/nodejs-mobile-android.zip" \
        "https://github.com/nodejs-mobile/nodejs-mobile/releases/download/${NODEJS_MOBILE_VERSION}/nodejs-mobile-${NODEJS_MOBILE_VERSION}-android.zip"
    unzip -o -q "$WORK_DIR/nodejs-mobile-android.zip" -d "$NODEJS_MOBILE_DIR"
fi

build_for_abi() {
    local abi="$1"
    local ffmpeg_arch ffmpeg_cpu target_triple
    case "$abi" in
        arm64-v8a)
            ffmpeg_arch="aarch64"; ffmpeg_cpu="armv8-a"; target_triple="aarch64-linux-android"
            ;;
        x86_64)
            ffmpeg_arch="x86_64"; ffmpeg_cpu="x86-64"; target_triple="x86_64-linux-android"
            ;;
        *)
            echo "ERROR: unsupported ABI '$abi' (supported: arm64-v8a, x86_64)" >&2
            return 1
            ;;
    esac

    echo "=== Building for $abi ($target_triple) ==="
    local cc="$TOOLCHAIN/bin/${target_triple}${API}-clang"
    local cxx="$TOOLCHAIN/bin/${target_triple}${API}-clang++"
    local ar="$TOOLCHAIN/bin/llvm-ar"
    local strip_tool="$TOOLCHAIN/bin/llvm-strip"
    local ranlib="$TOOLCHAIN/bin/llvm-ranlib"
    local nm_tool="$TOOLCHAIN/bin/llvm-nm"
    local sysroot="$TOOLCHAIN/sysroot"
    local abi_out="$OUT_ROOT/$abi"
    mkdir -p "$abi_out"

    # --- ffmpeg -------------------------------------------------------------
    # Minimal, remux-only: no encoders/decoders/filters beyond the handful
    # ffmpeg's own CLI hard-requires (fftools/ffmpeg.c depends on avfilter
    # even for a plain `-c copy` remux -- confirmed directly from
    # configure's own ffmpeg_deps/ffmpeg_select variables, a real bug hit
    # while first bringing this build up: --disable-avfilter silently
    # disabled the whole "ffmpeg" program despite --enable-ffmpeg, since
    # ffmpeg_deps="avcodec avfilter avformat threads"). Only the container
    # formats/codecs yt-dlp's own downloads actually use are enabled.
    local ffmpeg_build_dir="$WORK_DIR/ffmpeg-build-$abi"
    rm -rf "$ffmpeg_build_dir"
    mkdir -p "$ffmpeg_build_dir"
    cp -r "$FFMPEG_SRC"/. "$ffmpeg_build_dir/"
    (
        cd "$ffmpeg_build_dir"
        ./configure \
            --target-os=android \
            --arch="$ffmpeg_arch" \
            --cpu="$ffmpeg_cpu" \
            --enable-cross-compile \
            --cross-prefix= \
            --cc="$cc" \
            --cxx="$cxx" \
            --host-cc="$HOST_CC" \
            --ar="$ar" \
            --ranlib="$ranlib" \
            --strip="$strip_tool" \
            --nm="$nm_tool" \
            --sysroot="$sysroot" \
            --pkg-config=false \
            --enable-static \
            --disable-shared \
            --disable-doc \
            --disable-debug \
            --disable-avdevice \
            --disable-swscale \
            --disable-postproc \
            --disable-network \
            --disable-everything \
            --enable-ffmpeg \
            --enable-avfilter \
            --enable-filter=aformat,anull,atrim,crop,format,hflip,null,rotate,transpose,trim,vflip \
            --enable-protocol=file,pipe,concat \
            --enable-demuxer=mov,matroska,ogg,wav,aac,mp3 \
            --enable-muxer=mp4,webm,matroska,ipod \
            --enable-parser=aac,h264,hevc,vp8,vp9,av1,opus,vorbis,mpeg4video \
            --enable-bsf=aac_adtstoasc,h264_mp4toannexb,hevc_mp4toannexb,vp9_superframe,extract_extradata,null \
            --enable-small \
            --disable-symver
        make -j"$(nproc 2>/dev/null || echo 4)"
    )
    # "lib" prefix is not optional -- androiddeployqt rejects a
    # QT_ANDROID_EXTRA_LIBS entry outright ("must begin with \"lib\" and
    # end with the suffix \".so\"") if the filename doesn't start with it,
    # a real error hit wiring this into CMakeLists.txt. Renaming here is
    # safe: nothing looks this file up by NEEDED name, it's spawned
    # directly by whatever full path Vivace's own C++ resolves at runtime.
    cp "$ffmpeg_build_dir/ffmpeg" "$abi_out/libffmpeg.so"
    echo "  -> $abi_out/libffmpeg.so"

    # --- node wrapper ---------------------------------------------------
    local node_inc="$NODEJS_MOBILE_DIR/include/node"
    local node_lib_dir="$NODEJS_MOBILE_DIR/bin/$abi"
    if [ ! -f "$node_lib_dir/libnode.so" ]; then
        echo "ERROR: $node_lib_dir/libnode.so not found (nodejs-mobile has no $abi build?)" >&2
        return 1
    fi
    "$cxx" --sysroot="$sysroot" \
        -std=c++17 \
        -o "$abi_out/libvivace_node.so" \
        "$SCRIPT_DIR/../src/android/vivace_node_main.cc" \
        -I "$node_inc" \
        -L "$node_lib_dir" \
        -lnode \
        -Wl,-rpath,'$ORIGIN' \
        -fPIE -pie
    echo "  -> $abi_out/libvivace_node.so (wrapper)"

    # Kept as "libnode.so" EXACTLY (not renamed like libffmpeg.so/
    # libvivace_node.so above) -- the wrapper's own ELF dynamic section records a NEEDED entry
    # of literally "libnode.so" (baked in at link time), and RUNPATH=$ORIGIN
    # resolution looks for a file with that EXACT name next to the wrapper;
    # renaming it would silently break loading at runtime. ffmpeg/
    # vivace_node above are safe to rename freely since nothing looks them
    # up by NEEDED string -- they're spawned directly by path.
    cp "$node_lib_dir/libnode.so" "$abi_out/libnode.so"
    echo "  -> $abi_out/libnode.so"

    # Same reasoning: libc++_shared.so is needed (by exact name) by BOTH
    # the wrapper and libnode.so itself (confirmed via llvm-readelf -d on
    # both) -- NDK's own toolchain-provided C++ runtime, not guaranteed
    # present system-wide on Android, so it must be bundled too, under its
    # own real name.
    local libcxx="$sysroot/usr/lib/${target_triple}/libc++_shared.so"
    if [ ! -f "$libcxx" ]; then
        echo "ERROR: $libcxx not found" >&2
        return 1
    fi
    cp "$libcxx" "$abi_out/libc++_shared.so"
    echo "  -> $abi_out/libc++_shared.so"
}

for abi in "${ABIS[@]}"; do
    build_for_abi "$abi"
done

echo "Done. Per-ABI outputs under: $OUT_ROOT/<abi>/"
