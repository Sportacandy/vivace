# Kommandolinjetilvalg

Vivace kan startes fra en terminal med en mediefil og nogle få tilvalg. Brug lange
tilvalg i GNU-stil (korte aliasser vises i parentes).

```
vivace [tilvalg] [medie]
```

- **medie** — en filsti eller URL, der skal afspilles. Alt, hvad Vivace normalt
  kan åbne (lokal fil, mappe, `VIDEO_TS`, netværksstream, YouTube-URL), fungerer her.

## Tilvalg

- **`--help` (`-h`)** — vis listen over tilvalg og afslut.
- **`--version` (`-v`)** — vis versionen og afslut.
- **`--fullscreen` (`-f`)** — start i fuldskærm.
- **`--ontop` (`-t`)** — hold vinduet over andre vinduer.
- **`--close-at-end` (`-c`)** — luk Vivace, når afspilningen slutter.
- **`--sub <fil>` (`-s`)** — indlæs den angivne undertekstfil.
- **`--start <tid>` (`-b`)** — start afspilningen ved `<tid>`, angivet som
  `h:m:s`, `m:s` eller et antal sekunder.
- **`--pos <x,y>`** — placér vinduet ved skærmpositionen `x,y`.
- **`--size <b,h>`** — angiv vinduesstørrelsen til `b`×`h`.

## Eksempler

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Bemærk (Windows):** Vivace er et program med grafisk brugerflade, så `--help` /
`--version` skriver til den terminal, du startede det fra (skallens prompt kan
vende tilbage først, så teksten kan komme bagefter). Når enkeltinstanstilstand er
slået til (*Indstillinger ▸ Grænseflade ▸ Instanser*), anvender en ny start med en
mediefil og disse tilvalg dem på det allerede kørende vindue.
