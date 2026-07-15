# Options de ligne de commande

Vivace peut être lancé depuis un terminal avec un fichier multimédia et quelques
options. Utilisez des options longues de style GNU (les alias courts sont
indiqués entre parenthèses).

```
vivace [options] [média]
```

- **média** — un chemin de fichier ou une URL à lire. Tout ce que Vivace peut
  ouvrir normalement (fichier local, dossier, `VIDEO_TS`, flux réseau, URL
  YouTube) fonctionne ici.

## Options

- **`--help` (`-h`)** — afficher la liste des options et quitter.
- **`--version` (`-v`)** — afficher la version et quitter.
- **`--fullscreen` (`-f`)** — démarrer en plein écran.
- **`--ontop` (`-t`)** — garder la fenêtre au-dessus des autres fenêtres.
- **`--close-at-end` (`-c`)** — fermer Vivace à la fin de la lecture.
- **`--sub <fichier>` (`-s`)** — charger le fichier de sous-titres indiqué.
- **`--start <temps>` (`-b`)** — démarrer la lecture à `<temps>`, indiqué sous la
  forme `h:m:s`, `m:s` ou un nombre de secondes.
- **`--pos <x,y>`** — placer la fenêtre à la position d'écran `x,y`.
- **`--size <l,h>`** — définir la taille de la fenêtre à `l`×`h`.

## Exemples

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Remarque (Windows) :** Vivace est une application graphique, donc `--help` /
`--version` s'affichent dans le terminal depuis lequel vous l'avez lancé (l'invite
du shell peut revenir en premier, si bien que le texte peut apparaître après).
Lorsque le mode instance unique est activé (*Préférences ▸ Interface ▸
Instances*), relancer avec un fichier multimédia et ces options les applique à la
fenêtre déjà ouverte.
