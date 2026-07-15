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
