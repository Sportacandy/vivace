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
