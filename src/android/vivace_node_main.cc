/*  Vivace — a fast, pure-Qt media player.
    Copyright (C) 2026 Hironori Komaba
    SPDX-License-Identifier: GPL-3.0-or-later

    Thin wrapper making nodejs-mobile's libnode.so behave as a real,
    standard "node" CLI executable. nodejs-mobile's own Android release
    (github.com/nodejs-mobile/nodejs-mobile) ships ONLY libnode.so plus
    headers -- an embedding library, meant to be linked into your own
    native code and driven via its C++ API, not a standalone binary you
    can spawn as a subprocess.

    This file is intentionally byte-for-byte equivalent to Node's own
    upstream src/node_main.cc UNIX branch (verified directly against
    https://github.com/nodejs/node/blob/v18.20.4/src/node_main.cc, not
    written from memory) -- node::Start() IS the real entry point Node's
    own main() calls on every platform, so linking this against
    nodejs-mobile's libnode.so and calling it the same way produces a
    fully node-compatible executable, with the exact same argv/exit-code
    semantics as a real "node" binary.

    This is what lets yt-dlp's existing, UNMODIFIED --js-runtimes
    mechanism (built around spawning a real node/deno/bun binary and
    piping JS to it over stdin/stdout) work against a bundled, Android-
    legal executable with zero protocol reverse-engineering -- see
    scripts/build-android-youtube-tools.sh for how this gets built and
    CMakeLists.txt's VIVACE_HAVE_YOUTUBE_DOWNLOAD_TOOLS block for how the
    result is bundled into the APK.
*/

#include "node.h"

int main(int argc, char *argv[])
{
    return node::Start(argc, argv);
}
