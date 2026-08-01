# Options

Le menu **Options** contient les préférences et la configuration de l'interface.

- **Préférences…** (`Ctrl+P`) — la boîte de dialogue principale des réglages :
  Général, Interface, Sous-titres, Clavier et souris, Liste de lecture,
  Lecteurs, TV, Types de fichiers, Mises à jour, Réseau et Avancé. Les
  changements s'appliquent immédiatement ; **Annuler** les rétablit.
  - **Réseau** propose les onglets OpenSubtitles, YouTube, Proxy et Diffusion ;
    **Proxy** configure un proxy HTTP ou SOCKS5 facultatif, appliqué à toute
    l'application (recherche OpenSubtitles, vérification des mises à jour et,
    uniquement en HTTP, lecture multimédia et yt-dlp) ; **Diffusion** fixe le
    port sur lequel *Lecture ▸ Diffuser vers ▸ Smartphone/tablette* écoute.
    Le mot de passe du compte OpenSubtitles et le mot de passe du proxy sont
    désormais stockés de manière sécurisée dans le gestionnaire
    d'informations d'identification de votre système d'exploitation,
    et non dans les réglages propres de Vivace.
- **Afficher l'icône dans la zone de notification** — garder Vivace accessible
  depuis la zone de notification.
- **Barres d'outils**
  - **Barre d'outils** / **Barre de contrôle** — afficher ou masquer chaque barre.
  - **Modifier la barre d'outils principale…** / **Modifier la barre de
    contrôle…** — choisir les boutons affichés, leur ordre et la taille des icônes.
- **Barre d'état**
  - **Afficher la barre d'état** et ce qu'elle affiche : **Infos vidéo**,
    **Infos audio**, **Infos de format**, **Infos de débit**, **Compteur
    d'images**, **Afficher la durée totale**, **Afficher le temps restant** et
    **Afficher l'heure actuelle avec les millisecondes**.

**Astuce :** la disposition globale (Basic / Mini / MPC) se choisit dans
*Préférences ▸ Interface*.

## Installation et mise à jour de yt-dlp

*Préférences ▸ Réseau ▸ YouTube* propose une case à cocher **Utiliser un
yt-dlp géré** qui détermine comment Vivace obtient et maintient le programme
`yt-dlp` qu'il utilise pour lire les liens YouTube :

- **Activée** (par défaut) — Vivace peut installer `yt-dlp` pour vous et le
  maintenir à jour. Le champ **Chemin de yt-dlp :** est fixé sur la copie
  propre à Vivace et ne peut pas être modifié directement ; utilisez le
  bouton **Installer / mettre à jour yt-dlp…** (à côté de la case à cocher)
  chaque fois que vous voulez récupérer la dernière version officielle. Le
  réglage **Mettre à jour yt-dlp automatiquement :** devient également
  disponible, permettant à Vivace d'effectuer cette mise à jour de
  lui-même — **Jamais**, ou bien **À chaque exécution de yt-dlp**, ou
  encore une fois par **jour**/**semaine**. Une mise à jour automatique
  s'exécute juste avant qu'une URL YouTube ne soit réellement résolue ou
  téléchargée, ce qui rend la première lecture un peu plus longue une fois
  l'échéance atteinte ; si la mise à jour elle-même échoue (par exemple, pas
  de réseau), Vivace continue discrètement avec la version déjà installée
  plutôt que de bloquer la lecture.
- **Désactivée** — pour un yt-dlp que vous gérez vous-même (par exemple
  installé via `pip` ou le gestionnaire de paquets de votre système
  d'exploitation). Le champ **Chemin de yt-dlp :** devient modifiable afin
  que vous puissiez le faire pointer vers cette copie, et **Mettre à jour
  yt-dlp automatiquement** est désactivé — Vivace n'installe ni ne met
  jamais à jour un yt-dlp qu'il ne gère pas lui-même. Le bouton
  **Installer / mettre à jour yt-dlp…** est également désactivé dans ce
  mode.

## Exportation des cookies pour les téléchargements YouTube

Le champ **Fichier de cookies :** (*Préférences ▸ Réseau ▸ YouTube*) permet
aux modes YouTube **Télécharger et lire** et **Outil externe** de se
comporter comme si vous étiez connecté — nécessaire pour les vidéos
soumises à une restriction d'âge, réservées aux membres, ou autrement liées
à un compte, et c'est aussi ce qui débloque les téléchargements en HD/4K
complète. Il attend un fichier texte `cookies.txt` au format classique
Netscape (le même format que lit l'option `--cookies` de yt-dlp lui-même) ;
Vivace ne lit jamais les cookies directement depuis le profil d'un
navigateur.

**Pour en créer un :**

1. Connectez-vous à youtube.com dans votre navigateur habituel, avec le
   compte dont vous voulez utiliser l'accès.
2. Installez une extension de navigateur d'export de cookies qui écrit au
   format Netscape — pour Chrome, Edge ou Brave, recherchez dans la
   boutique d'extensions de votre navigateur quelque chose comme
   « Get cookies.txt » ; pour Firefox, recherchez « cookies.txt ». Toute
   extension qui indique clairement exporter au format Netscape classique
   `cookies.txt` conviendra.
3. Une fois youtube.com ouvert dans un onglet, utilisez l'extension pour
   exporter les cookies de ce site, et enregistrez le résultat quelque part
   sur le disque sous forme de fichier `.txt`.
4. Dans Vivace, ouvrez *Préférences ▸ Réseau ▸ YouTube* et utilisez
   **Parcourir…** à côté de **Fichier de cookies :** pour sélectionner ce
   fichier.

**À garder à l'esprit :**

- Un fichier `cookies.txt` équivaut en pratique à une session de connexion
  enregistrée — quiconque possède ce fichier peut agir en votre nom sur
  YouTube jusqu'à l'expiration des cookies ou votre déconnexion.
  Conservez-le dans un endroit privé et ne le partagez pas.
- Les cookies ne sont utilisés que par la voie de **téléchargement**
  (Télécharger et lire / Outil externe). Vivace n'envoie délibérément
  jamais de cookies en mode **diffusion en continu** — une URL de flux liée
  à une session est rattachée à celle-ci d'une manière que le simple
  lecteur vidéo de Vivace ne peut pas ouvrir, donc la diffusion reste
  anonyme même si un fichier de cookies est configuré.
- Les cookies expirent. Si des téléchargements qui fonctionnaient
  auparavant échouent, ou reviennent à un résultat public ou de moindre
  qualité, exportez un nouveau `cookies.txt`.

## Installation de Deno pour les téléchargements YouTube

yt-dlp lui-même — pas seulement Vivace — utilise un moteur d'exécution
JavaScript externe distinct pour résoudre les défis posés par YouTube avant
de fournir l'URL de téléchargement réelle d'une vidéo. Selon la
documentation propre de yt-dlp, fonctionner sans un tel moteur est
« obsolète » (deprecated) mais n'échoue pas forcément : la disponibilité des
formats est simplement réduite, et ce **fortement pour une requête connectée
(avec cookies)** — exactement le type de requête qu'effectue le mode
**Télécharger et lire** pour débloquer les vidéos en HD, réservées aux
membres et soumises à une restriction d'âge. Le mode **Diffusion en
continu** n'envoie jamais de cookies (voir « Exportation des cookies pour
les téléchargements YouTube » ci-dessus), ce n'est donc pas le cas critique,
et il fonctionne bien sans Deno dans la plupart des cas. C'est pourquoi le
champ **Chemin de Deno :** se trouve sous *Préférences ▸ Réseau ▸ YouTube ▸
Télécharger et lire*, et non comme un réglage YouTube général. yt-dlp prend
en charge plusieurs moteurs JavaScript ; Deno est celui qu'il recherche par
défaut.

**Pour installer Deno :**

1. Suivez les instructions d'installation officielles sur
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   pour votre système d'exploitation (un script d'installation, ou un
   gestionnaire de paquets comme winget/scoop/Homebrew/apt, selon la
   plateforme).
2. Assurez-vous que l'exécutable `deno` se retrouve dans le PATH de votre
   système — les installeurs ci-dessus le font normalement pour vous. Sous
   Windows, veillez bien à obtenir `deno`, et non `denort` (un exécutable
   différent, apparenté, qui ne fonctionnera pas ici).
3. Si vous préférez ne pas modifier le PATH, laissez-le tel quel et collez
   plutôt son chemin complet dans **Chemin de Deno :** (*Préférences ▸
   Réseau ▸ YouTube ▸ Télécharger et lire*).
4. Redémarrez Vivace (ou retentez simplement un téléchargement) après
   l'installation.

**À garder à l'esprit :**

- Il s'agit d'une dépendance de **yt-dlp**, pas de Vivace directement —
  Vivace se contente d'exécuter yt-dlp en tant que processus externe et
  n'invoque jamais Deno lui-même.
- yt-dlp exige une version de Deno raisonnablement récente (2.3.0 ou
  ultérieure au moment de la rédaction). Si les téléchargements montrent
  toujours une qualité réduite ou des erreurs de format après
  l'installation, vérifiez `deno --version` et mettez-le à jour s'il est
  plus ancien.
- Cette exigence vient de changements du côté de YouTube/yt-dlp, pas de
  Vivace — le champ **Chemin de Deno :** existe précisément pour cette
  raison et ne nécessite aucune configuration supplémentaire une fois Deno
  lui-même installé et accessible.
