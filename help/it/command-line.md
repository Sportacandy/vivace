# Opzioni della riga di comando

Vivace può essere avviato da un terminale con un file multimediale e alcune
opzioni. Usate opzioni lunghe in stile GNU (gli alias brevi sono indicati tra
parentesi).

```
vivace [opzioni] [media]
```

- **media** — un percorso di file o un URL da riprodurre. Qui funziona tutto ciò
  che Vivace può aprire normalmente (file locale, cartella, `VIDEO_TS`, flusso di
  rete, URL di YouTube).

## Opzioni

- **`--help` (`-h`)** — mostrare l'elenco delle opzioni ed uscire.
- **`--version` (`-v`)** — mostrare la versione ed uscire.
- **`--fullscreen` (`-f`)** — avviare a schermo intero.
- **`--ontop` (`-t`)** — mantenere la finestra sopra le altre.
- **`--close-at-end` (`-c`)** — chiudere Vivace al termine della riproduzione.
- **`--sub <file>` (`-s`)** — caricare il file di sottotitoli indicato.
- **`--start <tempo>` (`-b`)** — avviare la riproduzione a `<tempo>`, indicato
  come `h:m:s`, `m:s` o un numero di secondi.
- **`--pos <x,y>`** — posizionare la finestra alla posizione sullo schermo `x,y`.
- **`--size <l,a>`** — impostare la dimensione della finestra a `l`×`a`.

## Esempi

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Nota (Windows):** Vivace è un'applicazione grafica, quindi `--help` /
`--version` scrivono nel terminale da cui l'avete avviato (il prompt della shell
potrebbe tornare per primo, quindi il testo può comparire dopo). Quando è attiva
la modalità a istanza singola (*Preferenze ▸ Interfaccia ▸ Istanze*), riavviarlo
con un file multimediale e queste opzioni le applica alla finestra già in esecuzione.
