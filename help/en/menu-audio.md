# Audio

The **Audio** menu controls sound.

- **Track** — choose among the file's embedded audio tracks. A preferred
  language can be auto-selected in *Preferences ▸ General*.
- **Mute** (`M`) — toggle sound on/off.
- **Volume −** / **Volume +** (`↓` / `↑`) — change the volume; the step size is
  set in *Preferences ▸ General*.
- **Delay −** / **Delay +** — shift audio earlier or later to fix lip-sync.
- **Set delay…** — enter an exact per-file audio delay.

## About audio delay

The delay applied to a file is the sum of a **per-device** delay (remembered
for each audio output device — useful for Bluetooth latency, set in
*Preferences ▸ General ▸ Audio*) and the **per-file** delay from this menu.
Vivace applies the offset by delaying the video to match, so negative delays
(audio should come earlier) work as expected.
