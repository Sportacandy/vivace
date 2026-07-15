# Opdrachtregelopties

Vivace kan vanuit een terminal worden gestart met een mediabestand en enkele
opties. Gebruik lange opties in GNU-stijl (korte aliassen staan tussen haakjes).

```
vivace [opties] [media]
```

- **media** — een bestandspad of URL om af te spelen. Alles wat Vivace normaal
  kan openen (lokaal bestand, map, `VIDEO_TS`, netwerkstream, YouTube-URL) werkt hier.

## Opties

- **`--help` (`-h`)** — de lijst met opties tonen en afsluiten.
- **`--version` (`-v`)** — de versie tonen en afsluiten.
- **`--fullscreen` (`-f`)** — in volledig scherm starten.
- **`--ontop` (`-t`)** — het venster boven andere vensters houden.
- **`--close-at-end` (`-c`)** — Vivace sluiten wanneer het afspelen eindigt.
- **`--sub <bestand>` (`-s`)** — het opgegeven ondertitelbestand laden.
- **`--start <tijd>` (`-b`)** — het afspelen bij `<tijd>` starten, opgegeven als
  `h:m:s`, `m:s` of een aantal seconden.
- **`--pos <x,y>`** — het venster op schermpositie `x,y` plaatsen.
- **`--size <b,h>`** — de venstergrootte instellen op `b`×`h`.

## Voorbeelden

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Opmerking (Windows):** Vivace is een grafische toepassing, dus `--help` /
`--version` schrijven naar de terminal van waaruit u het hebt gestart (de
shell-prompt kan eerst terugkeren, zodat de tekst erna kan verschijnen). Wanneer
de enkele-instantiemodus aan staat (*Voorkeuren ▸ Interface ▸ Instanties*),
past een nieuwe start met een mediabestand en deze opties ze toe op het reeds
draaiende venster.
