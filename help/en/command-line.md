# Command-line options

Vivace can be started from a terminal with a media file and a few options. Use
GNU-style long options (short aliases are shown in parentheses).

```
vivace [options] [media]
```

- **media** — a file path or URL to play. Anything Vivace can open normally
  (local file, folder, `VIDEO_TS`, network stream, YouTube URL) works here.

## Options

- **`--help` (`-h`)** — show the list of options and exit.
- **`--version` (`-v`)** — show the version and exit.
- **`--fullscreen` (`-f`)** — start in fullscreen.
- **`--ontop` (`-t`)** — keep the window above other windows.
- **`--close-at-end` (`-c`)** — close Vivace when playback ends.
- **`--sub <file>` (`-s`)** — load the given subtitle file.
- **`--start <time>` (`-b`)** — start playback at `<time>`, given as `h:m:s`,
  `m:s`, or a number of seconds.
- **`--pos <x,y>`** — place the window at screen position `x,y`.
- **`--size <w,h>`** — set the window size to `w`×`h`.

## Examples

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Note (Windows):** Vivace is a GUI application, so `--help` / `--version` print
to the terminal you launched it from (the shell prompt may return first, so the
text can appear after it). When single-instance mode is on (*Preferences ▸
Interface ▸ Instances*), launching again with a media file and these options
applies them to the already-running window.
