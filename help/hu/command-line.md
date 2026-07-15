# Parancssori kapcsolók

A Vivace terminálból is indítható egy médiafájllal és néhány kapcsolóval.
Használjon GNU-stílusú hosszú kapcsolókat (a rövid álnevek zárójelben szerepelnek).

```
vivace [kapcsolók] [média]
```

- **média** — a lejátszandó fájl elérési útja vagy URL-je. Minden működik itt,
  amit a Vivace általában meg tud nyitni (helyi fájl, mappa, `VIDEO_TS`, hálózati
  adatfolyam, YouTube-URL).

## Kapcsolók

- **`--help` (`-h`)** — a kapcsolók listájának megjelenítése és kilépés.
- **`--version` (`-v`)** — a verzió megjelenítése és kilépés.
- **`--fullscreen` (`-f`)** — indítás teljes képernyőn.
- **`--ontop` (`-t`)** — az ablak más ablakok felett tartása.
- **`--close-at-end` (`-c`)** — a Vivace bezárása a lejátszás végén.
- **`--sub <fájl>` (`-s`)** — a megadott feliratfájl betöltése.
- **`--start <idő>` (`-b`)** — a lejátszás indítása `<idő>`-nél, `h:m:s`, `m:s`
  vagy másodpercek formájában megadva.
- **`--pos <x,y>`** — az ablak elhelyezése az `x,y` képernyőpozícióban.
- **`--size <sz,m>`** — az ablak méretének beállítása `sz`×`m` értékre.

## Példák

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Megjegyzés (Windows):** a Vivace grafikus alkalmazás, ezért a `--help` /
`--version` abba a terminálba ír, amelyből elindította (a shell prompt elsőként
térhet vissza, így a szöveg utána jelenhet meg). Ha az egypéldányos mód be van
kapcsolva (*Beállítások ▸ Felület ▸ Példányok*), egy médiafájllal és ezekkel a
kapcsolókkal történő újraindítás azokat a már futó ablakra alkalmazza.
