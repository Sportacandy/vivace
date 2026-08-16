# Opcions

El menú **Opcions** conté les preferències i la configuració de la interfície.

- **Preferències…** (`Ctrl+P`) — el diàleg principal de configuració: General,
  Interfície, Subtítols, Teclat i ratolí, Llista de reproducció, Unitats, TV,
  Tipus de fitxer, Actualitzacions, Xarxa i Avançat. Els canvis s'apliquen a
  l'instant; **Cancel·la** els reverteix.
  - **General ▸ Vídeo** estableix el mode per defecte de **Desentrellaça**
    (Sense / Yadif / Bwdif) per als fitxers acabats d'obrir — canvia'l per
    fitxer des de *Vídeo ▸ Desentrellaça*.
  - **Xarxa** té les pestanyes OpenSubtitles, YouTube, Proxy i Transmissió;
    **Proxy** configura un servidor intermediari HTTP o SOCKS5 opcional,
    aplicat a tota l'aplicació (cerca a OpenSubtitles, comprovació
    d'actualitzacions i, només amb HTTP, la reproducció i yt-dlp);
    **Transmissió** fixa el port que escolta *Reprodueix ▸ Transmet a ▸
    Mòbil/tauleta*. La contrasenya del compte d'OpenSubtitles i la
    contrasenya del servidor intermediari es desen de manera segura al
    gestor de credencials del sistema operatiu, no a la configuració pròpia
    del Vivace.
- **Mostra la icona a la safata del sistema** — mantenir el Vivace accessible des de la safata.
- **Barres d'eines**
  - **Barra d'eines** / **Barra de control** — mostrar o amagar cada barra.
  - **Edita la barra d'eines principal…** / **Edita la barra de control…** —
    triar quins botons apareixen, el seu ordre i la mida de les icones.
- **Barra d'estat**
  - **Mostra la barra d'estat** i el que mostra: **Info de vídeo**, **Info
    d'àudio**, **Info de format**, **Info de taxa de bits**, **Comptador de
    fotogrames**, **Mostra el temps total**, **Mostra el temps restant** i
    **Mostra l'hora actual amb mil·lisegons**.

**Consell:** la disposició general (Basic / Mini / MPC) es tria a *Preferències ▸ Interfície*.

## Instal·lar i actualitzar yt-dlp

*Preferències ▸ Xarxa ▸ YouTube* té la casella de selecció **Utilitza
yt-dlp gestionat**, que controla com el Vivace obté i manté el programa
`yt-dlp` que fa servir per reproduir enllaços de YouTube:

- **Activat** (per defecte) — el Vivace pot instal·lar `yt-dlp` per tu i
  mantenir-lo actualitzat. El camp **Camí de yt-dlp:** queda fixat a la
  còpia pròpia del Vivace i no es pot editar directament; utilitza el
  botó **Instal·la / actualitza yt-dlp…** (al costat de la casella de
  selecció) sempre que vulguis obtenir l'última versió oficial. També
  esdevé disponible la configuració **Actualitza yt-dlp
  automàticament:**, que permet que el Vivace faci aquesta actualització
  pel seu compte: **Mai**, **Cada vegada que s'executa yt-dlp**, **Un cop
  al dia** o **Un cop a la setmana**. Una actualització automàtica
  s'executa just abans que una URL de YouTube es resolgui o es baixi
  realment, de manera que la primera reproducció després que
  l'actualització esdevingui pendent triga una mica més; si
  l'actualització en si falla (p. ex. sense connexió de xarxa), el
  Vivace continua silenciosament amb la versió que ja hi ha instal·lada
  en lloc de bloquejar la reproducció.
- **Desactivat** — per a un yt-dlp que gestiones tu mateix (p. ex.
  instal·lat via `pip` o el gestor de paquets del teu sistema operatiu).
  El camp **Camí de yt-dlp:** esdevé editable perquè el puguis apuntar
  cap a aquella còpia, i **Actualitza yt-dlp automàticament** es
  desactiva — el Vivace mai no instal·la ni actualitza un yt-dlp que no
  gestiona. El botó **Instal·la / actualitza yt-dlp…** també queda
  desactivat en aquest mode.

## Exportar galetes per a les baixades de YouTube

El camp **Fitxer de galetes:** (*Preferències ▸ Xarxa ▸ YouTube*) permet
que els modes de YouTube **Baixa i reprodueix** i **eina externa** actuïn
com si haguessis iniciat la sessió — necessari per a vídeos amb
restricció d'edat, exclusius per a membres o d'una altra manera
vinculats a un compte, i és el que desbloqueja les baixades completes en
HD/4K. Espera un fitxer de text pla, `cookies.txt`, en el format clàssic
de galetes de Netscape (el mateix format que llegeix l'opció pròpia
`--cookies` del yt-dlp); el Vivace no llegeix les galetes directament
del perfil d'un navegador.

**Per crear-ne un:**

1. Inicia sessió a youtube.com amb el navegador que fas servir
   habitualment, amb el compte l'accés del qual vols utilitzar.
2. Instal·la una extensió del navegador d'exportació de galetes que
   escrigui el format de Netscape — per al Chrome, l'Edge o el Brave,
   cerca a la botiga d'extensions del teu navegador alguna cosa com
   «Get cookies.txt»; per al Firefox, cerca «cookies.txt». Qualsevol
   extensió que indiqui clarament que exporta el format clàssic de
   Netscape `cookies.txt` funcionarà.
3. Amb youtube.com obert en una pestanya, utilitza l'extensió per
   exportar les galetes d'aquest lloc i desa el resultat com a fitxer
   `.txt` en algun lloc del disc.
4. Al Vivace, obre *Preferències ▸ Xarxa ▸ YouTube* i utilitza
   **Navega…** al costat de **Fitxer de galetes:** per seleccionar
   aquest fitxer.

**Tingues en compte:**

- Un fitxer `cookies.txt` és, de fet, una sessió d'inici de sessió
  desada — qualsevol persona que tingui el fitxer pot actuar com el teu
  compte de YouTube fins que les galetes caduquin o tanquis la sessió.
  Guarda'l en un lloc privat i no el comparteixis.
- Les galetes només s'utilitzen per la via de **baixada** (Baixa i
  reprodueix / eina externa). El Vivace deliberadament mai envia
  galetes en mode de **transmissió** — un URL de transmissió amb
  sessió iniciada està vinculat a aquesta sessió d'una manera que el
  reproductor de vídeo senzill del Vivace no pot obrir, de manera que
  la transmissió continua sent anònima encara que hi hagi configurat un
  fitxer de galetes.
- Les galetes caduquen. Si les baixades que abans funcionaven comencen a
  fallar, o donen com a resultat una versió de qualitat
  inferior/pública, exporta un `cookies.txt` nou.

## Instal·lar ffmpeg per a les baixades de YouTube

El mode **Baixa i reprodueix** necessita el `ffmpeg` per combinar els
fluxos de vídeo i àudio separats que baixa el yt-dlp en un únic fitxer
reproduïble — el YouTube rarament ofereix l'HD com un únic flux combinat,
de manera que una pista de vídeo i una pista d'àudio es baixen per
separat i després es combinen. El camp **Ubicació de ffmpeg:**
(*Preferències ▸ Xarxa ▸ YouTube ▸ Baixa i reprodueix*) indica al yt-dlp
on trobar-lo; deixa'l buit per utilitzar en el seu lloc el `ffmpeg` del
PATH del sistema.

**Per instal·lar ffmpeg:**

1. **Windows** — l'opció més senzilla és un gestor de paquets:
   `winget install ffmpeg` (o `scoop install ffmpeg` / `choco install
   ffmpeg`). També pots baixar un arxiu precompilat des de
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) o
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) i
   descomprimir-lo en algun lloc.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — instal·la'l des del gestor de paquets de la teva
   distribució, p. ex. `sudo apt install ffmpeg` (Debian/Ubuntu), `sudo
   dnf install ffmpeg` (Fedora) o `sudo pacman -S ffmpeg` (Arch).
4. Si has afegit el ffmpeg al PATH del sistema, deixa buit el camp
   **Ubicació de ffmpeg:**. En cas contrari, enganxa en aquest camp el
   camí a la *carpeta* que conté l'executable `ffmpeg` (no l'executable
   mateix).
5. Reinicia el Vivace (o simplement torna a provar una baixada) després
   d'instal·lar-lo.

**Tingues en compte:**

- Aquesta és una dependència del **yt-dlp**, igual que el Deno de més
  avall — el Vivace només l'executa com a procés extern.
- El mode **Reproducció en flux** mai no necessita el ffmpeg, ja que
  reprodueix un flux ja combinat; només el mode **Baixa i reprodueix**
  el necessita, perquè aquest mode baixa el vídeo i l'àudio per separat
  i els combina localment.
- Si una baixada falla amb un error relacionat amb la combinació, comprova
  primer la ubicació del ffmpeg — és la causa més habitual, a banda d'un
  Deno absent o desactualitzat.

## Instal·lar Deno per a les baixades de YouTube

El mateix yt-dlp — no només el Vivace — utilitza un entorn d'execució
JavaScript extern i separat per resoldre els reptes que planteja YouTube
abans de facilitar l'URL de baixada real d'un vídeo. Segons la mateixa
documentació de yt-dlp, executar-lo sense aquest entorn està "obsolet"
(deprecated), però no falla directament: la disponibilitat de formats
simplement es redueix, i **de manera severa per a una sol·licitud amb
sessió iniciada (basada en galetes)** — exactament el tipus de
sol·licitud que fa el mode **Baixa i reprodueix** per desbloquejar
vídeos en HD, exclusius per a membres i amb restricció d'edat. El mode
**Reproducció en flux** mai no envia galetes (vegeu «Exportar galetes per a les
baixades de YouTube» més amunt), de manera que no és el cas sever, i
funciona bé sense Deno en la majoria dels casos. Per això el camp
**Camí de Deno:** es troba a *Preferències ▸ Xarxa ▸ YouTube ▸ Baixa i
reprodueix*, i no com a configuració general de YouTube. El yt-dlp
admet diversos entorns d'execució JS; Deno és el que cerca per defecte.

**Per instal·lar Deno:**

1. Segueix les instruccions oficials d'instal·lació a
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   per al teu sistema operatiu (un script d'instal·lació, o un gestor
   de paquets com winget/scoop/Homebrew/apt, segons la plataforma).
2. Assegura't que l'executable `deno` acabi al PATH del sistema — els
   instal·ladors anteriors normalment ho fan per tu. A Windows, assegura't
   d'obtenir `deno`, no `denort` (un executable diferent, relacionat,
   que no funcionarà aquí).
3. Si prefereixes no modificar el PATH, deixa'l tal com està i, en
   comptes d'això, enganxa el seu camí complet a **Camí de Deno:**
   (*Preferències ▸ Xarxa ▸ YouTube ▸ Baixa i reprodueix*).
4. Reinicia el Vivace (o simplement torna a provar una baixada) després
   d'instal·lar-lo.

**Tingues en compte:**

- Aquesta és una dependència de **yt-dlp**, no del Vivace directament —
  el Vivace només executa yt-dlp com a procés extern i mai no invoca
  Deno directament.
- El yt-dlp requereix una versió de Deno raonablement recent (2.3.0 o
  posterior en el moment d'escriure això). Si les baixades encara
  mostren errors de qualitat/format reduïts després d'instal·lar-lo,
  comprova `deno --version` i actualitza'l si és més antic.
- Aquest requisit prové de canvis al costat de YouTube/yt-dlp, no del
  Vivace — el mateix camp **Camí de Deno:** existeix exactament per
  aquest motiu i no necessita cap configuració addicional un cop Deno
  mateix està instal·lat i és accessible.

## Suavitzat dels subtítols de mapa de bits

*Preferències ▸ Subtítols ▸ Subtítols de mapa de bits* té una opció
**Suavitzat:** (0–3, valor per defecte 1) per als subtítols que es
representen com a imatges en lloc de text -- pistes de subimatge de
DVD, PGS i DVB. Això inclou tant els subtítols propis d'un disc DVD
real com una pista de subtítols incrustada del mateix tipus en un
fitxer de vídeo normal (per exemple, un fitxer .mp4 amb una pista amb
còdec `dvd_subtitle`). Aquests formats són imatges de mapa de bits
prerenderitzades, gravades a la resolució nativa de definició estàndard
(SD) quan es va crear la font -- les vores poden semblar dentades en
ampliar-se a la mida d'una finestra moderna. El Vivace pot aplicar un
lleuger difuminat per suavitzar aquestes vores:

- **0** — desactivat; mostra el mapa de bits de subtítols original tal
  com es va crear.
- **1** (valor per defecte) — suavitza les vores més marcades mantenint
  el text amb una brillantor pràcticament completa.
- **2** / **3** — difuminat progressivament més intens.

Aquesta opció només afecta els subtítols de mapa de bits — no té cap
efecte sobre el renderitzador extern de subtítols propi del Vivace
(SRT/VTT/ASS) ni sobre les pistes de subtítols de text normals, totes
dues amb camins de renderització diferents.
