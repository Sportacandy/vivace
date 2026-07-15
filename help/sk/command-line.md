# Voľby príkazového riadka

Vivace možno spustiť z terminálu so súborom médií a niekoľkými voľbami. Používajte
dlhé voľby v štýle GNU (krátke aliasy sú uvedené v zátvorkách).

```
vivace [voľby] [médium]
```

- **médium** — cesta k súboru alebo URL na prehranie. Funguje tu všetko, čo
  Vivace bežne otvorí (miestny súbor, priečinok, `VIDEO_TS`, sieťový stream, URL YouTube).

## Voľby

- **`--help` (`-h`)** — zobraziť zoznam volieb a skončiť.
- **`--version` (`-v`)** — zobraziť verziu a skončiť.
- **`--fullscreen` (`-f`)** — spustiť v celej obrazovke.
- **`--ontop` (`-t`)** — udržať okno nad ostatnými oknami.
- **`--close-at-end` (`-c`)** — zavrieť Vivace po skončení prehrávania.
- **`--sub <súbor>` (`-s`)** — načítať zadaný súbor titulkov.
- **`--start <čas>` (`-b`)** — začať prehrávanie od `<čas>`, zadaného ako `h:m:s`,
  `m:s` alebo počet sekúnd.
- **`--pos <x,y>`** — umiestniť okno na pozíciu obrazovky `x,y`.
- **`--size <š,v>`** — nastaviť veľkosť okna na `š`×`v`.

## Príklady

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Poznámka (Windows):** Vivace je aplikácia s grafickým rozhraním, takže `--help`
/ `--version` vypisujú do terminálu, z ktorého ste ho spustili (výzva shellu sa
môže vrátiť skôr, takže text sa môže objaviť až po nej). Keď je zapnutý režim
jedinej inštancie (*Nastavenia ▸ Rozhranie ▸ Inštancie*), opätovné spustenie so
súborom médií a týmito voľbami ich použije na už bežiace okno.
