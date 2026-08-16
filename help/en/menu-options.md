# Options

The **Options** menu holds preferences and interface configuration.

- **Preferences…** (`Ctrl+P`) — the main settings dialog: General, Interface,
  Subtitles, Keyboard and mouse, Playlist, Drives, TV, File types, Updates,
  Network and Advanced. Changes apply instantly; **Cancel** reverts them.
  - **Network** has OpenSubtitles, YouTube, Proxy and Cast tabs; **Proxy**
    configures an optional HTTP or SOCKS5 proxy applied application-wide
    (OpenSubtitles search, the update check, and — HTTP only — media
    playback and yt-dlp); **Cast** sets the fixed port *Play ▸ Cast ▸
    Smartphone/tablet* listens on. The OpenSubtitles account password and
    the proxy password are stored securely in your operating system's
    credential manager, not in Vivace's own settings.
- **Show icon in system tray** — keep Vivace reachable from the tray.
- **Toolbars**
  - **Toolbar** / **Control bar** — show or hide each bar.
  - **Edit main toolbar…** / **Edit control bar…** — choose which buttons
    appear and their order and icon size.
- **Status bar**
  - **Show status bar**, and what it displays: **Video info**, **Audio info**,
    **Format info**, **Bitrate info**, **Frame counter**, **Display total
    time**, **Display remaining time**, and **Show the current time with
    milliseconds**.

**Tip:** the overall layout (Basic / Mini / MPC) is chosen under
*Preferences ▸ Interface*.

## Installing and updating yt-dlp

*Preferences ▸ Network ▸ YouTube* has a **Use managed yt-dlp** checkbox that
controls how Vivace gets and maintains the `yt-dlp` program it uses to play
YouTube links:

- **On** (the default) — Vivace can install `yt-dlp` for you and keep it
  current. The **yt-dlp path:** field is fixed to Vivace's own copy and
  can't be edited directly; use the **Install / Update yt-dlp…** button
  (next to the checkbox) whenever you want to fetch the latest official
  release. The **Update yt-dlp automatically:** setting also becomes
  available, letting Vivace run this update on its own — **Never**, before
  **Every time yt-dlp runs**, or once a **day**/**week**. An automatic
  update runs right before a YouTube URL is actually resolved or downloaded,
  so the first play after it becomes due takes a little longer; if the
  update itself fails (e.g. no network), Vivace quietly proceeds with
  whatever version is already installed rather than blocking playback.
- **Off** — for a yt-dlp you manage yourself (e.g. installed via `pip` or
  your OS package manager). The **yt-dlp path:** field becomes editable so
  you can point it at that copy, and **Update yt-dlp automatically** is
  disabled — Vivace never installs or updates a yt-dlp it doesn't manage.
  The **Install / Update yt-dlp…** button is also disabled in this mode.

## Exporting cookies for YouTube downloads

The **Cookies file:** field (*Preferences ▸ Network ▸ YouTube*) lets the
**Download & play** and **external tool** YouTube modes act as if you were
signed in — needed for age-restricted, members-only, or otherwise
account-gated videos, and it's what unlocks full HD/4K downloads. It expects
a plain-text `cookies.txt` file in the classic Netscape cookie-jar format
(the same format yt-dlp's own `--cookies` option reads); Vivace does not read
cookies directly out of a browser's profile.

**To create one:**

1. Sign in to youtube.com in your everyday browser, using the account whose
   access you want to use.
2. Install a cookie-export browser extension that writes the Netscape
   format — for Chrome, Edge or Brave, search your browser's extension store
   for something like "Get cookies.txt"; for Firefox, search for
   "cookies.txt". Any extension that clearly states it exports the classic
   Netscape `cookies.txt` format will work.
3. With youtube.com open in a tab, use the extension to export cookies for
   that site, and save the result as a `.txt` file somewhere on disk.
4. In Vivace, open *Preferences ▸ Network ▸ YouTube* and use **Browse…** next
   to **Cookies file:** to select that file.

**Keep in mind:**

- A `cookies.txt` file is effectively a saved login session — anyone who has
  the file can act as your YouTube account until the cookies expire or you
  sign out. Store it somewhere private and don't share it.
- Cookies are used only by the **download** path (Download & play / external
  tool). Vivace deliberately never sends cookies in **streaming** mode — a
  signed-in stream URL is bound to that session in a way Vivace's plain video
  player cannot open, so streaming stays anonymous even if a cookies file is
  configured.
- Cookies expire. If downloads that previously worked start failing, or fall
  back to a lower-quality/public result, export a fresh `cookies.txt`.

## Installing ffmpeg for YouTube downloads

**Download & play** mode needs `ffmpeg` to merge the separate video and audio
streams yt-dlp downloads into one playable file — YouTube rarely offers HD as
a single combined stream, so a video track and an audio track are downloaded
separately and then merged. The **ffmpeg location:** field (*Preferences ▸
Network ▸ YouTube ▸ Download & play*) tells yt-dlp where to find it; leave it
empty to use `ffmpeg` from your system PATH instead.

**To install ffmpeg:**

1. **Windows** — the easiest option is a package manager:
   `winget install ffmpeg` (or `scoop install ffmpeg` / `choco install
   ffmpeg`). Alternatively, download a prebuilt archive from
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) or
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) and unzip it
   somewhere.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — install it from your distribution's package manager, e.g.
   `sudo apt install ffmpeg` (Debian/Ubuntu), `sudo dnf install ffmpeg`
   (Fedora), or `sudo pacman -S ffmpeg` (Arch).
4. If you added ffmpeg to your system PATH, leave **ffmpeg location:** empty.
   Otherwise, paste the path to the *folder* containing the `ffmpeg`
   executable (not the executable itself) into that field.
5. Restart Vivace (or just retry a download) after installing.

**Keep in mind:**

- This is a dependency of **yt-dlp**, like Deno below — Vivace only ever runs
  it as an external process.
- **Streaming** mode never needs ffmpeg, since it plays a single
  already-combined stream; only **Download & play** does, because that mode
  fetches separate video and audio and merges them locally.
- If a download fails with a merge-related error, check the ffmpeg location
  first — it's the most common cause, besides a missing or outdated Deno.

## Installing Deno for YouTube downloads

yt-dlp itself — not just Vivace — uses a separate external JavaScript
runtime to solve challenges YouTube presents before handing out a video's
real download URL. Per yt-dlp's own documentation, running without one is
"deprecated" but does not outright fail: format availability is simply
reduced, and **severely so for a signed-in (cookie) request** — exactly
the kind of request **Download & play** mode makes to unlock HD,
members-only, and age-restricted videos. **Streaming** mode never sends
cookies (see "Exporting cookies for YouTube downloads" above), so it isn't
the severe case and works fine without Deno in most cases. This is why the
**Deno path:** field lives under *Preferences ▸ Network ▸ YouTube ▸
Download & play*, not as a general YouTube setting. yt-dlp supports
several JS runtimes; Deno is the one it looks for by default.

**To install Deno:**

1. Follow the official installation instructions at
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   for your operating system (an installer script, or a package manager like
   winget/scoop/Homebrew/apt, depending on platform).
2. Make sure the `deno` executable ends up on your system PATH — the
   installers above normally do this for you. On Windows, be sure to get
   `deno`, not `denort` (a different, related executable that won't work
   here).
3. If you'd rather not modify PATH, leave it where it is and instead paste
   its full path into **Deno path:** (*Preferences ▸ Network ▸ YouTube ▸
   Download & play*).
4. Restart Vivace (or just retry a download) after installing.

**Keep in mind:**

- This is a dependency of **yt-dlp**, not of Vivace directly — Vivace only
  ever runs yt-dlp as an external process and never invokes Deno itself.
- yt-dlp requires a reasonably recent Deno version (2.3.0 or later at the
  time of writing). If downloads still show reduced quality/format errors
  after installing, check `deno --version` and update it if it's older.
- This requirement comes from changes on YouTube's/yt-dlp's side, not from
  Vivace — the same **Deno path:** field exists for exactly this reason and
  needs no further configuration once Deno itself is installed and
  reachable.

## Bitmap subtitle smoothing

*Preferences ▸ Subtitles ▸ Bitmap subtitles* has a **Smoothing:** setting
(0–3, default 1) for subtitles that are rendered as images rather than
text — DVD subpicture, PGS, and DVB tracks. This covers both a real DVD
disc's own subtitles and an embedded subtitle track of the same kind in
an ordinary video file (e.g. an .mp4 with a `dvd_subtitle`-codec track).
These formats are pre-rendered bitmap images baked in at native,
standard-definition resolution when the source was authored — their
edges can look jagged once scaled up to a modern window size. Vivace can
apply a slight blur to soften those edges:

- **0** — off; shows the original subtitle bitmap exactly as authored.
- **1** (the default) — softens the roughest edges while keeping the text
  at essentially full brightness.
- **2** / **3** — progressively more blur.

This setting only affects bitmap-based subtitles — it has no effect on
Vivace's own external subtitle renderer (SRT/VTT/ASS) or on ordinary
text-based subtitle tracks, both of which are unrelated rendering paths.
