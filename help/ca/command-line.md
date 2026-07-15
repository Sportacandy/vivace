# Opcions de línia d'ordres

El Vivace es pot iniciar des d'un terminal amb un fitxer multimèdia i algunes
opcions. Useu opcions llargues a l'estil GNU (els àlies curts es mostren entre parèntesis).

```
vivace [opcions] [mitjà]
```

- **mitjà** — un camí de fitxer o URL per reproduir. Aquí funciona tot el que el
  Vivace pot obrir normalment (fitxer local, carpeta, `VIDEO_TS`, flux de xarxa, URL de YouTube).

## Opcions

- **`--help` (`-h`)** — mostrar la llista d'opcions i sortir.
- **`--version` (`-v`)** — mostrar la versió i sortir.
- **`--fullscreen` (`-f`)** — iniciar en pantalla completa.
- **`--ontop` (`-t`)** — mantenir la finestra per damunt de les altres.
- **`--close-at-end` (`-c`)** — tancar el Vivace quan acabi la reproducció.
- **`--sub <fitxer>` (`-s`)** — carregar el fitxer de subtítols indicat.
- **`--start <temps>` (`-b`)** — iniciar la reproducció a `<temps>`, indicat com
  `h:m:s`, `m:s` o un nombre de segons.
- **`--pos <x,y>`** — situar la finestra a la posició de pantalla `x,y`.
- **`--size <a,alç>`** — establir la mida de la finestra a `a`×`alç`.

## Exemples

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Nota (Windows):** el Vivace és una aplicació gràfica, de manera que `--help` /
`--version` s'imprimeixen al terminal des del qual l'heu iniciat (l'indicador de
l'intèrpret d'ordres pot tornar primer, així que el text pot aparèixer després).
Quan el mode d'instància única està activat (*Preferències ▸ Interfície ▸
Instàncies*), tornar-lo a iniciar amb un fitxer multimèdia i aquestes opcions les
aplica a la finestra ja en execució.
