# Volby příkazového řádku

Vivace lze spustit z terminálu se souborem médií a několika volbami. Používejte
dlouhé volby ve stylu GNU (krátké aliasy jsou uvedeny v závorkách).

```
vivace [volby] [médium]
```

- **médium** — cesta k souboru nebo URL k přehrání. Funguje zde vše, co Vivace
  běžně otevře (místní soubor, složka, `VIDEO_TS`, síťový stream, URL YouTube).

## Volby

- **`--help` (`-h`)** — zobrazit seznam voleb a skončit.
- **`--version` (`-v`)** — zobrazit verzi a skončit.
- **`--fullscreen` (`-f`)** — spustit v celé obrazovce.
- **`--ontop` (`-t`)** — udržet okno nad ostatními okny.
- **`--close-at-end` (`-c`)** — zavřít Vivace po skončení přehrávání.
- **`--sub <soubor>` (`-s`)** — načíst zadaný soubor titulků.
- **`--start <čas>` (`-b`)** — začít přehrávání v `<čas>`, zadaném jako `h:m:s`,
  `m:s` nebo počet sekund.
- **`--pos <x,y>`** — umístit okno na pozici obrazovky `x,y`.
- **`--size <š,v>`** — nastavit velikost okna na `š`×`v`.

## Příklady

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Poznámka (Windows):** Vivace je aplikace s grafickým rozhraním, takže `--help`
/ `--version` vypisují do terminálu, ze kterého jste jej spustili (výzva shellu
se může vrátit dříve, takže text se může objevit až po ní). Když je zapnutý režim
jediné instance (*Předvolby ▸ Rozhraní ▸ Instance*), opětovné spuštění se
souborem médií a těmito volbami je použije na již běžící okno.
