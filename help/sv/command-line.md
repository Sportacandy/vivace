# Kommandoradsalternativ

Vivace kan startas från en terminal med en mediefil och några alternativ. Använd
långa alternativ i GNU-stil (korta alias visas inom parentes).

```
vivace [alternativ] [media]
```

- **media** — en filsökväg eller URL att spela upp. Allt som Vivace normalt kan
  öppna (lokal fil, mapp, `VIDEO_TS`, nätverksström, YouTube-URL) fungerar här.

## Alternativ

- **`--help` (`-h`)** — visa listan över alternativ och avsluta.
- **`--version` (`-v`)** — visa versionen och avsluta.
- **`--fullscreen` (`-f`)** — starta i helskärm.
- **`--ontop` (`-t`)** — håll fönstret ovanför andra fönster.
- **`--close-at-end` (`-c`)** — stäng Vivace när uppspelningen slutar.
- **`--sub <fil>` (`-s`)** — ladda den angivna undertextfilen.
- **`--start <tid>` (`-b`)** — börja uppspelningen vid `<tid>`, angiven som
  `h:m:s`, `m:s` eller ett antal sekunder.
- **`--pos <x,y>`** — placera fönstret vid skärmpositionen `x,y`.
- **`--size <b,h>`** — ställ in fönsterstorleken till `b`×`h`.

## Exempel

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Obs (Windows):** Vivace är ett grafiskt program, så `--help` / `--version`
skriver till terminalen du startade det från (skalets prompt kan återvända
först, så texten kan visas efteråt). När eninstansläget är på (*Inställningar ▸
Gränssnitt ▸ Instanser*) tillämpar en ny start med en mediefil och dessa
alternativ dem på det redan körande fönstret.
