# Opcje wiersza poleceń

Vivace można uruchomić z terminala z plikiem multimedialnym i kilkoma opcjami.
Używaj długich opcji w stylu GNU (krótkie aliasy podano w nawiasach).

```
vivace [opcje] [multimedia]
```

- **multimedia** — ścieżka pliku lub adres URL do odtworzenia. Działa tu
  wszystko, co Vivace zwykle otwiera (plik lokalny, folder, `VIDEO_TS`, strumień
  sieciowy, adres URL YouTube).

## Opcje

- **`--help` (`-h`)** — pokaż listę opcji i zakończ.
- **`--version` (`-v`)** — pokaż wersję i zakończ.
- **`--fullscreen` (`-f`)** — uruchom w trybie pełnoekranowym.
- **`--ontop` (`-t`)** — utrzymuj okno nad innymi oknami.
- **`--close-at-end` (`-c`)** — zamknij Vivace po zakończeniu odtwarzania.
- **`--sub <plik>` (`-s`)** — wczytaj podany plik napisów.
- **`--start <czas>` (`-b`)** — rozpocznij odtwarzanie od `<czas>`, podanego jako
  `h:m:s`, `m:s` lub liczba sekund.
- **`--pos <x,y>`** — umieść okno w położeniu ekranu `x,y`.
- **`--size <sz,wys>`** — ustaw rozmiar okna na `sz`×`wys`.

## Przykłady

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Uwaga (Windows):** Vivace to aplikacja z graficznym interfejsem, więc `--help`
/ `--version` wypisują tekst do terminala, z którego je uruchomiono (znak
zachęty powłoki może wrócić najpierw, więc tekst może pojawić się po nim). Gdy
włączony jest tryb pojedynczej instancji (*Ustawienia ▸ Interfejs ▸ Instancje*),
ponowne uruchomienie z plikiem multimedialnym i tymi opcjami stosuje je do już
działającego okna.
