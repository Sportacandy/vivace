#!/usr/bin/env bash
# Builds a minimal Qt 6.11.1 SDK from source (qtbase + qtshadertools +
# qtdeclarative + qttools) for Vivace's Linux-arm64-on-Debian-12-"bookworm"
# build. This exists ONLY because Qt's official prebuilt linux_gcc_arm64 kit
# requires glibc 2.39+ (confirmed against Qt's own docs,
# https://doc.qt.io/qt-6/linux-building.html) while Raspberry Pi OS -- the
# most common arm64 desktop Linux target, and the whole reason this script
# exists -- is currently based on Debian 12 "bookworm" (glibc 2.36). Building
# INSIDE a debian:bookworm container produces binaries that only need
# glibc 2.36, which real Raspberry Pi OS (and any other bookworm-or-newer
# glibc) can run. See CLAUDE.md's 2026-08-10 entries for the full
# investigation (Qt's ARM64 reference platform is Ubuntu on a Pi 5, NOT
# Raspberry Pi OS -- so this is genuinely unsupported territory, not a
# packaging oversight on Qt's part).
#
# Deliberately does NOT use Qt's own top-level `configure` wrapper script
# (which downloads/builds the "everywhere" source tree) -- two reasons:
# (1) that tarball is a multi-GB download bundling dozens of modules this
# project doesn't need, when only 4 individual module repos are actually
# required; (2) this project's own qtmultimedia patch-build scripts already
# established (and proved working, see their own comments) that a PLAIN
# `cmake -S/-B` invocation against each module's own repo, one at a time,
# against a growing CMAKE_PREFIX_PATH, avoids `configure`/`qt-configure-
# module`'s own argument-parsing quirks entirely. This script continues
# that same pattern for the base SDK instead of introducing a second,
# untested build mechanism.
#
# Usage: build-qt-from-source-bookworm.sh <INSTALL_PREFIX>
#   INSTALL_PREFIX: where to install the built Qt SDK, e.g. /opt/qt6.
#   Must be run inside a Debian 12 "bookworm" environment (or a container)
#   with the apt packages from https://doc.qt.io/qt-6/linux-requirements.html
#   already installed, PLUS build-essential/cmake/ninja-build/git/curl/
#   ca-certificates/perl/python3/pkg-config/libssl-dev/libwayland-dev.
#
# After this script, run scripts/build-patched-qtmultimedia-linux.sh
# <INSTALL_PREFIX> to add the patched qtmultimedia (AV1/speed-control/
# bitmap-subtitle fixes) on top -- that script is unmodified and arch-aware
# via `uname -m` already.

set -euo pipefail

PREFIX="${1:?usage: $0 <INSTALL_PREFIX>}"
QT_TAG="v6.11.1"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

# Order matters: each module's CMake config is found via CMAKE_PREFIX_PATH
# pointing at $PREFIX, which grows as each earlier module installs into it.
MODULES=(qtbase qtshadertools qtdeclarative qttools)

for module in "${MODULES[@]}"; do
    echo "== Cloning $module $QT_TAG =="
    git clone --branch "$QT_TAG" --depth 1 "https://github.com/qt/$module.git" "$WORK/$module"

    echo "== Configuring + building $module against $PREFIX =="
    CMAKE_ARGS=(
        -S "$WORK/$module" -B "$WORK/build-$module"
        -G Ninja
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_PREFIX="$PREFIX"
        -DQT_BUILD_EXAMPLES=OFF
        -DQT_BUILD_TESTS=OFF
    )
    # qtbase has no Qt dependency of its own (it IS the base); every module
    # after it needs to find qtbase's (and each other's) installed CMake
    # package config via CMAKE_PREFIX_PATH.
    if [ "$module" != "qtbase" ]; then
        CMAKE_ARGS+=(-DCMAKE_PREFIX_PATH="$PREFIX")
    fi
    cmake "${CMAKE_ARGS[@]}"
    cmake --build "$WORK/build-$module" --parallel
    cmake --install "$WORK/build-$module"

    rm -rf "$WORK/$module" "$WORK/build-$module"
done

echo "Done: $PREFIX now has qtbase + qtshadertools + qtdeclarative + qttools ($QT_TAG)."
