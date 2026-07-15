# Befehlszeilenoptionen

Vivace kann von einem Terminal aus mit einer Mediendatei und einigen Optionen
gestartet werden. Verwenden Sie lange Optionen im GNU-Stil (Kurzformen stehen in
Klammern).

```
vivace [Optionen] [Medien]
```

- **Medien** — ein Dateipfad oder eine URL zum Abspielen. Alles, was Vivace
  normalerweise öffnen kann (lokale Datei, Ordner, `VIDEO_TS`, Netzwerkstream,
  YouTube-URL), funktioniert hier.

## Optionen

- **`--help` (`-h`)** — die Liste der Optionen anzeigen und beenden.
- **`--version` (`-v`)** — die Version anzeigen und beenden.
- **`--fullscreen` (`-f`)** — im Vollbild starten.
- **`--ontop` (`-t`)** — das Fenster über anderen Fenstern halten.
- **`--close-at-end` (`-c`)** — Vivace schließen, wenn die Wiedergabe endet.
- **`--sub <Datei>` (`-s`)** — die angegebene Untertiteldatei laden.
- **`--start <Zeit>` (`-b`)** — die Wiedergabe bei `<Zeit>` beginnen, angegeben
  als `h:m:s`, `m:s` oder eine Anzahl von Sekunden.
- **`--pos <x,y>`** — das Fenster an der Bildschirmposition `x,y` platzieren.
- **`--size <b,h>`** — die Fenstergröße auf `b`×`h` setzen.

## Beispiele

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Hinweis (Windows):** Vivace ist eine GUI-Anwendung, daher geben `--help` /
`--version` ihre Ausgabe an das Terminal aus, von dem Sie es gestartet haben
(die Eingabeaufforderung kann zuerst zurückkehren, sodass der Text danach
erscheint). Wenn der Einzelinstanzmodus aktiv ist (*Einstellungen ▸ Oberfläche
▸ Instanzen*), wendet ein erneuter Start mit einer Mediendatei und diesen
Optionen sie auf das bereits laufende Fenster an.
