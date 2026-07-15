# Play

The **Play** menu controls playback and seeking.

## Transport

- **Play**, **Pause**, **Stop** — `Space` toggles play/pause; `Ctrl+Space`
  stops.
- **Frame step** (`.`) and **Frame back step** — advance or go back one frame.
- **Previous** / **Next** — move between playlist items.

## Seeking

Three seek step sizes are offered in both directions (**Rewind** / **Forward**),
shown with their current durations. Adjust the step sizes in
*Preferences ▸ Interface ▸ Seeking*. You can also seek with `←` / `→`, the seek
slider, or the mouse wheel over the video.

## Speed

- **Normal speed** (`Backspace`), **Halve speed**, **Double speed**.
- **Speed −10% / +10%** and finer ±4% / ±1% steps.
- **Pitch compensation** — keep voices natural at non-normal speeds.

Playback speed is per-file and resets to normal for each new file.

## A-B section

Loop part of a file:

- **Set A marker**, **Set B marker** — mark the start and end.
- **Repeat** — loop the A-B region (the section only repeats while this is on).
- **Clear A-B markers** — remove them. Markers reset for each new file.

**Jump to…** (go to a specific time) is planned for a later release.

## Cast

**Cast ▸ Smartphone/tablet…** starts a small built-in web server so a phone
or tablet on the same network can open a page and play the file Vivace is
currently playing, with a QR code to scan or the address to type in. The
port is fixed in *Preferences ▸ Network ▸ Cast* (not editable per session)
so you can allow it through your firewall/router once. Only the file
currently playing is served, and only while casting is on.
