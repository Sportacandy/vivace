# Opcions

El menú **Opcions** conté les preferències i la configuració de la interfície.

- **Preferències…** (`Ctrl+P`) — el diàleg principal de configuració: General,
  Interfície, Subtítols, Teclat i ratolí, Llista de reproducció, Unitats, TV,
  Tipus de fitxer, Actualitzacions, Xarxa i Avançat. Els canvis s'apliquen a
  l'instant; **Cancel·la** els reverteix.
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
