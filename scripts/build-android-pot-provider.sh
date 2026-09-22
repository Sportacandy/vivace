#!/usr/bin/env bash
# Builds the Android-bundled assets for the YouTube "PO token provider"
# feature (see YoutubeResolver's own desktop-side installOrUpdatePotProvider()
# for the equivalent live-download/build flow on Windows/Linux/macOS) --
# recent YouTube videos increasingly require a PO (proof-of-origin) token
# just to play at all, and yt-dlp's own community-standard mechanism for
# generating one is the "BgUtils POT Provider" project
# (github.com/Brainicism/bgutil-ytdlp-pot-provider): a small yt-dlp plugin
# (registers with yt-dlp's POT Provider Framework) plus a Node/Deno script
# yt-dlp runs on demand to actually mint the token.
#
# Android needs a COMPLETELY DIFFERENT build strategy from desktop's live
# "download the source + run deno install/tsc on first use" flow -- there is
# no Deno on Android at all (see build-android-youtube-tools.sh's own
# comment on why nodejs-mobile, not Deno, is this project's bundled JS
# runtime there), and running a live `npm install`/`deno install` step
# on-device is not remotely realistic (no compiler toolchain for native
# addons, no writable place for a real package manager cache, no guarantee
# of network access at the moment it's needed). So this ALL happens at
# BUILD time instead, producing a pre-built, self-contained asset tree that
# PythonYoutubeResolver extracts once (mirroring extractedStdlibHomeDir()'s
# own versioned-marker extraction pattern) and points yt-dlp's
# `--extractor-args youtubepot-bgutilscript:script_path=...` /
# `--js-runtimes` mechanism at directly -- no on-device build step of any
# kind.
#
# REAL, EMPIRICALLY VALIDATED constraints discovered building this (see this
# project's own CLAUDE.md history for the full investigation):
#
# 1. The generation script (server/src/generate_once.ts) is TypeScript with
#    real npm dependencies (axios, bgutils-js, commander, jsdom, proxy-agent,
#    youtubei.js) -- NOT a bundler-produced single file upstream. It needs a
#    real build step. Bundling it with esbuild into ONE flat CommonJS file
#    works for every dependency EXCEPT jsdom itself, whose own code reads a
#    couple of real on-disk resource files (a default stylesheet, an XHR
#    sync-worker script) via `__dirname`-relative paths that a flattening
#    bundler breaks. Fix: bundle everything EXCEPT jsdom/canvas
#    (`--external:jsdom --external:canvas`) and ship jsdom's own real
#    node_modules subtree as loose files alongside the bundle, exactly where
#    Node's own module resolution will find it (a sibling `node_modules/`
#    of the bundle). `canvas` itself is never actually required at runtment
#    for THIS narrow use case (jsdom's own canvas support is optional --
#    confirmed empirically: the script runs and generates a real token with
#    only a harmless "canvas not installed" console message).
#
# 2. This project's Android JS runtime is nodejs-mobile v18.20.4 (see
#    build-android-youtube-tools.sh) -- but bgutil-ytdlp-pot-provider's own
#    package.json declares "engines": {"node": ">=22"}, and its yt-dlp
#    plugin (BgUtilScriptNodePTP._JSRT_MIN_VER = (22, 0, 0)) enforces this.
#    Confirmed EMPIRICALLY (not just from the stated engines field) this is
#    a REAL requirement, not a conservative floor: the LATEST jsdom (30.x,
#    pulled in by a plain `npm install jsdom`) depends, several levels deep,
#    on a package (`@exodus/bytes`) that is pure ESM with NO CommonJS
#    build at all -- loading it via a synchronous `require()` needs Node's
#    own `require(esm)` support, stable only since Node 22. A SEPARATE,
#    independent Node-22+-only language feature (`ArrayBuffer.prototype.
#    resizable`, used unconditionally at module-load time by
#    webidl-conversions@8) would ALSO crash on Node 18 even if the ESM
#    problem were papered over.
#
#    FIX: pin jsdom to v26 specifically for this Android build (NOT the
#    ^29.1.1 the real upstream project's own package.json declares) --
#    confirmed empirically that jsdom 24/25/26 have ZERO `@exodus/bytes`
#    anywhere in their own dependency trees (that migration happened in
#    jsdom 27+), and jsdom 26 pulls webidl-conversions@7 (no resizable-
#    ArrayBuffer check at all). This is a genuine, deliberate downgrade
#    from what upstream's own lockfile specifies, done ONLY for this
#    Android build (desktop's own live-built copy, via YoutubeResolver's
#    installOrUpdatePotProvider(), always gets upstream's real, current
#    jsdom via a real `deno install`/npm resolution against the
#    unmodified package.json -- this pin is Android-only).
#
# 3. jsdom 26's own `new JSDOM(html, { resources: {...} })` API requires a
#    real `ResourceLoader` INSTANCE for the `resources` option -- session_
#    manager.ts (as shipped upstream, written/tested against jsdom 29)
#    passes a plain `{ userAgent }` object, which jsdom 29+ accepts as a
#    convenience shorthand but jsdom 26 rejects outright ("resources must
#    be an instance of ResourceLoader"). FIX: a small, one-line source
#    patch (see patch_session_manager() below) importing `ResourceLoader`
#    and constructing one explicitly -- functionally identical either way,
#    the ONLY jsdom-26-vs-29 API difference this specific code path hits.
#
# ALL THREE of the above were verified end to end against the EXACT real
# runtime this produces for Android (a genuine Node v18.20.4 binary,
# downloaded directly from nodejs.org to match nodejs-mobile's own bundled
# version): --version reports correctly, a real (no-content-binding)
# visitor-data token generates correctly, and a real content-binding +
# innertube-context invocation (the exact shape yt-dlp's own plugin uses)
# also generates a real, correctly-shaped PO token -- not just "the script
# doesn't crash," a genuine working token response in every case.
#
# Usage: build-android-pot-provider.sh <output-dir>
#   output-dir: where the two asset trees are written --
#               <output-dir>/pot-plugin/yt_dlp_plugins/...  (the Python plugin)
#               <output-dir>/pot-provider/server/build/generate_once.js
#               <output-dir>/pot-provider/server/node_modules/...  (jsdom only)
#
# Needs: git, node + npm (>=18; used only to BUILD this, not to run it --
# any reasonably recent Node works for the build step itself, since esbuild/
# npm's own tooling doesn't share the runtime's own Node-22 dependency
# problem), and network access. Does NOT need the Android NDK at all --
# nothing here is native code; the whole output is plain JS/Python text.
#
# UNVERIFIED beyond the exact Node v18.20.4 + jsdom 26 combination checked
# above on this Windows dev machine -- needs the user's own on-device test
# once wired into PythonYoutubeResolver (a real Android nodejs-mobile
# process spawn, not just a desktop Node binary of the same version, could
# still behave differently in some unforeseen way).

set -euo pipefail

OUT_ROOT="${1:?usage: $0 <output-dir>}"
mkdir -p "$OUT_ROOT"

POT_PROVIDER_VERSION="2.0.0"  # matches this project's desktop-side pin point of reference; bump alongside it
JSDOM_ANDROID_VERSION="26"    # see the big comment above -- deliberately older than upstream's own ^29.1.1

WORK_DIR="$OUT_ROOT/_work"
mkdir -p "$WORK_DIR"

# --- Fetch the real bgutil-ytdlp-pot-provider source (same source desktop's
# own installOrUpdatePotProvider() downloads at runtime) -----------------
SRC_DIR="$WORK_DIR/bgutil-ytdlp-pot-provider"
if [ ! -d "$SRC_DIR" ]; then
    echo "Cloning bgutil-ytdlp-pot-provider v$POT_PROVIDER_VERSION..."
    git clone --branch "$POT_PROVIDER_VERSION" --depth 1 \
        https://github.com/Brainicism/bgutil-ytdlp-pot-provider.git "$SRC_DIR"
fi

# --- Stage 1: the yt-dlp plugin (pure Python, no build step needed) ------
PLUGIN_ASSET_DIR="$OUT_ROOT/pot-plugin/yt_dlp_plugins"
rm -rf "$OUT_ROOT/pot-plugin"
mkdir -p "$OUT_ROOT/pot-plugin"
cp -r "$SRC_DIR/plugin/yt_dlp_plugins" "$PLUGIN_ASSET_DIR"
echo "  -> $PLUGIN_ASSET_DIR"

# --- Stage 2: install the REAL runtime dependencies (respecting upstream's
# own package-lock.json, matching desktop's own `deno install --frozen`
# exactly) -- everything except jsdom, which gets a separate, deliberately
# older install in stage 3. -------------------------------------------------
SERVER_SRC="$SRC_DIR/server"
(
    cd "$SERVER_SRC"
    echo "Installing server/ dependencies (respecting package-lock.json)..."
    npm ci --no-audit --no-fund
)

# --- Stage 3: patch session_manager.ts for jsdom 26's stricter API (see
# the big comment above, point 3) -----------------------------------------
patch_session_manager() {
    local f="$SERVER_SRC/src/session_manager.ts"
    if grep -q 'new ResourceLoader(' "$f"; then
        return 0  # already patched (idempotent re-run)
    fi
    sed -i \
        -e 's/^import { JSDOM } from "jsdom";$/import { JSDOM, ResourceLoader } from "jsdom";/' \
        -e 's/resources: { userAgent: USER_AGENT },/resources: new ResourceLoader({ userAgent: USER_AGENT }),/' \
        "$f"
    if ! grep -q 'new ResourceLoader(' "$f"; then
        echo "ERROR: session_manager.ts patch did not apply -- upstream source" >&2
        echo "       may have changed shape; update patch_session_manager()." >&2
        exit 1
    fi
}
patch_session_manager

# --- Stage 4: install jsdom 26 specifically (overriding the lockfile's own
# ^29.1.1) into an ISOLATED, clean directory -- deliberately NOT installed
# into $SERVER_SRC itself, so `npm ci` above (which must stay faithful to
# upstream's real lockfile for every OTHER dependency) is never disturbed. --
JSDOM_DIR="$WORK_DIR/jsdom-android"
rm -rf "$JSDOM_DIR"
mkdir -p "$JSDOM_DIR"
(
    cd "$JSDOM_DIR"
    npm init -y >/dev/null
    echo "Installing jsdom@$JSDOM_ANDROID_VERSION in isolation (Android/Node-18 compatible)..."
    npm install --no-audit --no-fund "jsdom@$JSDOM_ANDROID_VERSION"
)
if grep -rl "@exodus/bytes" "$JSDOM_DIR/node_modules" --include="*.js" >/dev/null 2>&1; then
    echo "ERROR: jsdom@$JSDOM_ANDROID_VERSION's dependency tree now references" >&2
    echo "       @exodus/bytes (an ESM-only package needing Node 22+) -- the" >&2
    echo "       empirically-validated Node-18-compatible pin has changed" >&2
    echo "       upstream. Re-investigate (see the big comment above) before" >&2
    echo "       trusting this build." >&2
    exit 1
fi

# --- Stage 5: bundle generate_once.ts, external jsdom+canvas -------------
(
    cd "$SERVER_SRC"
    echo "Installing esbuild..."
    npm install --no-save --no-audit --no-fund esbuild
    echo "Bundling generate_once.ts..."
    node_modules/.bin/esbuild src/generate_once.ts \
        --bundle --platform=node --format=cjs \
        --external:canvas --external:jsdom \
        --outfile="$WORK_DIR/generate_once.js"
)

# --- Stage 6: assemble the final asset tree, matching the exact directory
# shape the plugin's own path math expects (see getpot_bgutil.py/
# getpot_bgutil_script.py: script_path -> up two levels -> server_home,
# then server_home/build/generate_once.js reconstructed -- so the file
# MUST live at <server_home>/build/generate_once.js, and jsdom's own
# node_modules MUST be a sibling of that build/ dir for plain Node
# require() resolution to find it by walking upward). -------------------
PROVIDER_ASSET_DIR="$OUT_ROOT/pot-provider/server"
rm -rf "$OUT_ROOT/pot-provider"
mkdir -p "$PROVIDER_ASSET_DIR/build" "$PROVIDER_ASSET_DIR/node_modules"
# The Node PO-token-provider variant's own hardcoded basename check
# (BgUtilScriptNodePTP._SCRIPT_BASENAME = 'generate_once.js') -- must be
# named exactly this, not e.g. generate_once.cjs, even though the content
# is plain CommonJS.
cp "$WORK_DIR/generate_once.js" "$PROVIDER_ASSET_DIR/build/generate_once.js"
cp -r "$JSDOM_DIR/node_modules/." "$PROVIDER_ASSET_DIR/node_modules/"
echo "  -> $PROVIDER_ASSET_DIR/build/generate_once.js"
echo "  -> $PROVIDER_ASSET_DIR/node_modules/ ($(find "$PROVIDER_ASSET_DIR/node_modules" -type f | wc -l) files)"

echo "Done. Assets under: $OUT_ROOT/{pot-plugin,pot-provider}/"
