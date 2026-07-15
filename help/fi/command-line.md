# Komentorivivalinnat

Vivacen voi käynnistää päätteestä mediatiedoston ja muutamien valintojen kanssa.
Käytä GNU-tyylisiä pitkiä valintoja (lyhyet aliakset näkyvät suluissa).

```
vivace [valinnat] [media]
```

- **media** — toistettavan tiedoston polku tai URL-osoite. Kaikki, minkä Vivace
  yleensä voi avata (paikallinen tiedosto, kansio, `VIDEO_TS`, verkkosuoratoisto,
  YouTube-URL), toimii tässä.

## Valinnat

- **`--help` (`-h`)** — näytä valintojen luettelo ja lopeta.
- **`--version` (`-v`)** — näytä versio ja lopeta.
- **`--fullscreen` (`-f`)** — käynnistä koko näytön tilassa.
- **`--ontop` (`-t`)** — pidä ikkuna muiden ikkunoiden päällä.
- **`--close-at-end` (`-c`)** — sulje Vivace, kun toisto päättyy.
- **`--sub <tiedosto>` (`-s`)** — lataa annettu tekstitystiedosto.
- **`--start <aika>` (`-b`)** — aloita toisto kohdasta `<aika>`, joka annetaan
  muodossa `h:m:s`, `m:s` tai sekunteina.
- **`--pos <x,y>`** — sijoita ikkuna näytön sijaintiin `x,y`.
- **`--size <l,k>`** — aseta ikkunan kooksi `l`×`k`.

## Esimerkkejä

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Huomautus (Windows):** Vivace on graafinen sovellus, joten `--help` /
`--version` tulostuvat päätteeseen, josta käynnistit sen (komentokehote voi palata
ensin, joten teksti voi ilmestyä sen jälkeen). Kun yhden esiintymän tila on
käytössä (*Asetukset ▸ Käyttöliittymä ▸ Esiintymät*), uudelleenkäynnistys
mediatiedoston ja näiden valintojen kanssa soveltaa ne jo käynnissä olevaan ikkunaan.
