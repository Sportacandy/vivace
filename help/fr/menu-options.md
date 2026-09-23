# Options

Le menu **Options** contient les préférences et la configuration de l'interface.

- **Préférences…** (`Ctrl+P`) — la boîte de dialogue principale des réglages :
  Général, Interface, Sous-titres, Clavier et souris, Liste de lecture,
  Lecteurs, TV, Types de fichiers, Mises à jour, Réseau et Avancé. Les
  changements s'appliquent immédiatement ; **Annuler** les rétablit.
  - **Général ▸ Vidéo** définit le mode **Désentrelacement** par défaut
    (Aucun / Yadif / Bwdif / Automatique) pour les nouveaux fichiers
    ouverts — modifiable par fichier depuis *Vidéo ▸ Désentrelacement*
    (Aucun/Yadif/Bwdif uniquement ; Automatique n'y est pas proposé, car
    il n'a de sens que comme valeur par défaut). **Automatique** utilise
    Bwdif, mais uniquement sur les images que le fichier lui-même marque
    comme entrelacées, laissant les images progressives inchangées.
  - **Général ▸ Audio et sous-titres préférés** définit les langues parmi
    lesquelles Vivace choisit automatiquement les pistes intégrées d'un
    fichier. **Afficher les sous-titres par défaut** en active un
    automatiquement lorsqu'il est disponible ; sa sous-option **...mais pas
    si l'audio est déjà dans une langue préférée** ignore ce sous-titre
    automatique lorsque sa langue correspond à la piste audio réellement
    sélectionnée — utile si vous comprenez déjà l'audio et ne voulez pas
    qu'un sous-titre dans la même langue détourne votre attention.
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

## Installation du fournisseur de token PO de YouTube

Les vidéos YouTube récentes exigent de plus en plus un **token PO
(proof-of-origin)** juste pour pouvoir être lues — une exigence générale de
lisibilité, indépendante des cookies ou de la connexion. Sans lui, yt-dlp
signale la vidéo comme indisponible, même pour une requête ordinaire et
anonyme. Cela s'applique aux **deux** modes, Diffusion en continu et
Télécharger et lire, contrairement aux cookies (téléchargement uniquement,
voir ci-dessous) ou à Deno (concerne surtout Télécharger et lire, voir plus
bas).

*Préférences ▸ Réseau ▸ YouTube* propose un bouton **Installer le
fournisseur de token PO…** qui met en place le projet communautaire
« **BgUtils POT Provider** » : un petit plugin yt-dlp accompagné d'un
script — exécuté à la demande via Deno, le même programme abordé plus bas —
qui permet à yt-dlp de générer un token automatiquement. Cliquez dessus,
attendez la fin du court processus de téléchargement et de construction, et
c'est terminé ; l'étiquette du bouton devient **Réinstaller / mettre à jour
le fournisseur de token PO…** une fois installé, au cas où une mise à jour
serait un jour nécessaire.

**À garder à l'esprit :**

- Cela nécessite une installation de Deno fonctionnelle (voir « Installation
  de Deno pour les téléchargements YouTube » ci-dessous) — le bouton
  télécharge et construit le script propre au fournisseur, que yt-dlp
  exécute ensuite via Deno chaque fois qu'une vidéo a réellement besoin
  d'un token.
- Toutes les vidéos n'ont pas besoin d'un token PO, donc la lecture peut
  très bien fonctionner sans cette installation — mais si une vidéo se
  signale comme indisponible alors qu'elle se lit bien ailleurs, cela vaut
  la peine de l'installer.
- **Sous Android,** ceci est fourni avec Vivace et configuré automatiquement
  — il n'y a rien à installer et aucun bouton équivalent ; cela fonctionne
  simplement.

## Cookies pour les téléchargements YouTube

Les modes YouTube **Télécharger et lire** et **Outil externe** peuvent se
comporter comme si vous étiez connecté — nécessaire pour les vidéos
soumises à une restriction d'âge, réservées aux membres, ou autrement liées
à un compte, et c'est aussi ce qui débloque les téléchargements en HD/4K
complète. Vivace prend en charge trois façons de fournir des cookies
(toutes sous *Préférences ▸ Réseau ▸ YouTube*) : **Récupérer les cookies
depuis le navigateur** sous Windows/Linux/macOS, **Se connecter à
YouTube…** sous Android, et l'exportation manuelle d'un fichier
`cookies.txt` comme solution de repli sur toutes les plateformes.

### Récupérer les cookies depuis le navigateur (recommandé sous Windows/Linux/macOS)

Le menu déroulant **Récupérer les cookies depuis le navigateur :**
répertorie Firefox, Chrome, Edge, Brave, Chromium, Opera, Safari, Vivaldi
et Whale. Choisissez votre navigateur et Vivace lit ses cookies en direct,
à chaque fois — rien à exporter, rien qui ne devienne obsolète.

**À garder à l'esprit :**

- YouTube a considérablement raccourci la durée de vie de ses propres
  cookies, si bien qu'un fichier `cookies.txt` précédemment exporté (voir
  ci-dessous) peut devenir obsolète en quelques jours — cette option évite
  totalement ce problème en lisant le magasin de cookies propre au
  navigateur, toujours à jour.
- **Sous Windows, seul Firefox fonctionne réellement ici.** Chrome, Edge et
  les autres navigateurs basés sur Chromium sous Windows chiffrent leurs
  cookies d'une manière liée au binaire même du navigateur (« App-Bound
  Encryption », Chrome 127+) — cela empêche complètement yt-dlp, ainsi que
  tout autre outil externe, de les lire. Il s'agit d'une restriction du
  côté de Chrome ; les développeurs de yt-dlp eux-mêmes ne peuvent pas la
  contourner. Chrome/Edge sous Linux et macOS ne sont pas concernés et
  fonctionnent normalement.
- Sélectionner un navigateur ici prend le pas sur le champ **Fichier de
  cookies :** ci-dessous, lorsque les deux sont définis.
- **Non disponible sous Android** — consultez plutôt **Se connecter à
  YouTube** ci-dessous.

### Se connecter à YouTube (Android, recommandé sur cette plateforme)

*Préférences ▸ Réseau ▸ YouTube* propose un bouton **Se connecter à
YouTube…** (à côté du champ du fichier de cookies) qui ouvre une véritable
page de connexion directement dans Vivace, via une vue web (WebView)
intégrée et native de l'OS. Connectez-vous avec le compte dont vous voulez
utiliser l'accès, puis appuyez sur **Je suis connecté** — Vivace lit les
cookies de session obtenus et les enregistre automatiquement comme fichier
de cookies actif. C'est l'équivalent, sous Android, de **Récupérer les
cookies depuis le navigateur** ci-dessus : rien à exporter, rien à
transférer manuellement depuis un autre appareil.

**À garder à l'esprit :**

- Android n'a aucun équivalent d'un magasin de cookies de navigateur de
  bureau consultable en direct (le stockage de chaque application est
  isolé de celui de toutes les autres) — c'est pourquoi cette option
  fonctionne différemment de celle du bureau : se connecter *à l'intérieur*
  du navigateur intégré de Vivace en est le substitut pratique.
- Disponible uniquement sous Android ; toutes les autres plateformes
  utilisent **Récupérer les cookies depuis le navigateur** à la place.

### Exporter les cookies dans un fichier (solution de repli sur toutes les plateformes)

Le champ **Fichier de cookies :** attend un fichier texte `cookies.txt` au
format classique Netscape (le même format que lit l'option `--cookies` de
yt-dlp lui-même) — utilisez-le lorsqu'aucune des deux options en direct
ci-dessus n'est disponible ou ne fonctionne pour vous (par exemple
Chrome/Edge sous Windows).

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

**Sous Android :** préférez plutôt **Se connecter à YouTube…** ci-dessus —
cela ne nécessite aucune étape d'exportation/transfert. Si vous souhaitez
tout de même procéder de cette manière, notez que Chrome pour Android ne
prend pas en charge les extensions de navigateur, donc les étapes 2 et 3
ci-dessus ne peuvent pas être effectuées sur l'appareil lui-même. Exportez
`cookies.txt` sur un
ordinateur de bureau ou portable comme décrit ci-dessus, puis transférez ce
fichier sur votre appareil Android (par exemple via un stockage en ligne,
un câble USB ou par e-mail) avant d'utiliser **Parcourir…** à l'étape 4.

**À garder à l'esprit :**

- Un fichier `cookies.txt` équivaut en pratique à une session de connexion
  enregistrée — quiconque possède ce fichier peut agir en votre nom sur
  YouTube jusqu'à l'expiration des cookies ou votre déconnexion.
  Conservez-le dans un endroit privé et ne le partagez pas.
- Les cookies expirent. Si des téléchargements qui fonctionnaient
  auparavant échouent, ou reviennent à un résultat public ou de moindre
  qualité, exportez un nouveau `cookies.txt` — ou passez à **Récupérer les
  cookies depuis le navigateur** ci-dessus, si disponible, pour éviter
  totalement ce problème.

**S'applique aux deux méthodes ci-dessus :**

- Les cookies ne sont utilisés que par la voie de **téléchargement**
  (Télécharger et lire / Outil externe). Vivace n'envoie délibérément
  jamais de cookies en mode **diffusion en continu** — une URL de flux liée
  à une session est rattachée à celle-ci d'une manière que le simple
  lecteur vidéo de Vivace ne peut pas ouvrir, donc la diffusion reste
  anonyme quelle que soit la configuration des cookies.

## Installation de ffmpeg pour les téléchargements YouTube

Le mode **Télécharger et lire** a besoin de `ffmpeg` pour fusionner les
flux vidéo et audio séparés que yt-dlp télécharge en un seul fichier
lisible — YouTube propose rarement la HD sous la forme d'un flux combiné
unique, si bien qu'une piste vidéo et une piste audio sont téléchargées
séparément puis fusionnées. Le champ **Emplacement de ffmpeg :**
(*Préférences ▸ Réseau ▸ YouTube ▸ Télécharger et lire*) indique à yt-dlp
où le trouver ; laissez-le vide pour utiliser à la place le `ffmpeg` du
PATH de votre système.

**Pour installer ffmpeg :**

1. **Windows** — l'option la plus simple est un gestionnaire de paquets :
   `winget install ffmpeg` (ou `scoop install ffmpeg` / `choco install
   ffmpeg`). Vous pouvez aussi télécharger une archive précompilée depuis
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) ou
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) et la
   décompresser quelque part.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — installez-le depuis le gestionnaire de paquets de votre
   distribution, par exemple `sudo apt install ffmpeg` (Debian/Ubuntu),
   `sudo dnf install ffmpeg` (Fedora), ou `sudo pacman -S ffmpeg` (Arch).
4. Si vous avez ajouté ffmpeg au PATH de votre système, laissez
   **Emplacement de ffmpeg :** vide. Sinon, collez dans ce champ le
   chemin du *dossier* contenant l'exécutable `ffmpeg` (pas l'exécutable
   lui-même).
5. Redémarrez Vivace (ou retentez simplement un téléchargement) après
   l'installation.

**À garder à l'esprit :**

- Il s'agit d'une dépendance de **yt-dlp**, comme Deno ci-dessous — Vivace
  se contente de l'exécuter en tant que processus externe.
- Le mode **Diffusion en continu** n'a jamais besoin de ffmpeg, puisqu'il
  lit un flux déjà combiné unique ; seul **Télécharger et lire** en a
  besoin, car ce mode récupère la vidéo et l'audio séparément puis les
  fusionne localement.
- Si un téléchargement échoue avec une erreur liée à la fusion, vérifiez
  d'abord l'emplacement de ffmpeg — c'est la cause la plus fréquente, à
  part un Deno manquant ou obsolète.

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
continu** n'envoie jamais de cookies (voir « Cookies pour les
téléchargements YouTube » ci-dessus), ce n'est donc pas le cas critique,
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

## Lissage des sous-titres bitmap

*Préférences ▸ Sous-titres ▸ Sous-titres bitmap* propose un réglage
**Lissage :** (0–3, valeur par défaut 1) pour les sous-titres rendus
sous forme d'images plutôt que de texte — pistes de sous-titrage DVD,
PGS et DVB. Cela couvre à la fois les sous-titres propres à un vrai
disque DVD et une piste de sous-titres intégrée du même type dans un
fichier vidéo ordinaire (par exemple un fichier .mp4 avec une piste au
codec `dvd_subtitle`). Ces formats sont des images bitmap pré-rendues,
gravées à la résolution native en définition standard (SD) lors de la
création de la source — leurs contours peuvent paraître crénelés une
fois agrandis à la taille d'une fenêtre moderne. Vivace peut appliquer
un léger flou pour adoucir ces contours :

- **0** — désactivé ; affiche le bitmap de sous-titres original tel
  quel.
- **1** (valeur par défaut) — adoucit les contours les plus marqués tout
  en conservant une luminosité quasi intacte du texte.
- **2** / **3** — flou progressivement plus marqué.

Ce réglage n'affecte que les sous-titres bitmap — il n'a aucun effet
sur le moteur de rendu externe de sous-titres de Vivace (SRT/VTT/ASS)
ni sur les pistes de sous-titres textuelles ordinaires, qui empruntent
toutes deux des chemins de rendu différents.
