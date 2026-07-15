# Opcije naredbenog retka

Vivace se može pokrenuti iz terminala s medijskom datotekom i nekoliko opcija.
Koristite duge opcije u GNU stilu (kratki nadimci prikazani su u zagradama).

```
vivace [opcije] [medij]
```

- **medij** — putanja datoteke ili URL za reprodukciju. Ovdje radi sve što Vivace
  inače može otvoriti (lokalna datoteka, mapa, `VIDEO_TS`, mrežni stream, YouTube URL).

## Opcije

- **`--help` (`-h`)** — prikaz popisa opcija i izlaz.
- **`--version` (`-v`)** — prikaz verzije i izlaz.
- **`--fullscreen` (`-f`)** — pokretanje u cijelom zaslonu.
- **`--ontop` (`-t`)** — držanje prozora iznad drugih prozora.
- **`--close-at-end` (`-c`)** — zatvaranje Vivacea kad reprodukcija završi.
- **`--sub <datoteka>` (`-s`)** — učitavanje navedene datoteke titlova.
- **`--start <vrijeme>` (`-b`)** — početak reprodukcije od `<vrijeme>`, navedenog
  kao `h:m:s`, `m:s` ili broj sekundi.
- **`--pos <x,y>`** — postavljanje prozora na položaj zaslona `x,y`.
- **`--size <š,v>`** — postavljanje veličine prozora na `š`×`v`.

## Primjeri

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Napomena (Windows):** Vivace je aplikacija s grafičkim sučeljem, pa `--help` /
`--version` ispisuju u terminal iz kojeg ste ga pokrenuli (upit ljuske može se
vratiti prvi, pa se tekst može pojaviti nakon njega). Kad je uključen način rada
jedne instance (*Postavke ▸ Sučelje ▸ Instance*), ponovno pokretanje s medijskom
datotekom i ovim opcijama primjenjuje ih na već pokrenuti prozor.
