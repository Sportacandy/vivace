# Opciones

El menú **Opciones** contiene las preferencias y la configuración de la interfaz.

- **Preferencias…** (`Ctrl+P`) — el cuadro de diálogo principal de ajustes:
  General, Interfaz, Subtítulos, Teclado y ratón, Lista de reproducción,
  Unidades, TV, Tipos de archivo, Actualizaciones, Red y Avanzado. Los cambios se
  aplican al instante; **Cancelar** los revierte.
  - **General ▸ Vídeo** establece el modo **Desentrelazado** predeterminado
    (Desactivar / Yadif / Bwdif) para los archivos recién abiertos —
    cámbielo por archivo desde *Vídeo ▸ Desentrelazado*.
  - **Red** tiene las pestañas OpenSubtitles, YouTube, Proxy y Transmisión;
    **Proxy** configura un proxy HTTP o SOCKS5 opcional aplicado a toda la
    aplicación (búsqueda en OpenSubtitles, la comprobación de actualizaciones
    y, solo con HTTP, la reproducción y yt-dlp); **Transmisión** fija el
    puerto en el que escucha *Reproducir ▸ Transmitir a ▸ Teléfono/tableta*.
    La contraseña de la cuenta de OpenSubtitles y la contraseña del proxy se
    almacenan ahora de forma segura en el administrador de credenciales de su
    sistema operativo, no en los propios ajustes de Vivace.
- **Mostrar icono en la bandeja del sistema** — mantener Vivace accesible desde la bandeja.
- **Barras de herramientas**
  - **Barra de herramientas** / **Barra de control** — mostrar u ocultar cada barra.
  - **Editar barra de herramientas principal…** / **Editar barra de control…** —
    elegir qué botones aparecen, su orden y el tamaño de los iconos.
- **Barra de estado**
  - **Mostrar barra de estado** y lo que muestra: **Info de vídeo**, **Info de
    audio**, **Info de formato**, **Info de tasa de bits**, **Contador de
    fotogramas**, **Mostrar tiempo total**, **Mostrar tiempo restante** y
    **Mostrar la hora actual con milisegundos**.

**Sugerencia:** la disposición general (Basic / Mini / MPC) se elige en
*Preferencias ▸ Interfaz*.

## Instalación y actualización de yt-dlp

*Preferencias ▸ Red ▸ YouTube* tiene una casilla **Usar yt-dlp gestionado**
que controla cómo obtiene y mantiene Vivace el programa `yt-dlp` que usa
para reproducir enlaces de YouTube:

- **Activada** (opción predeterminada) — Vivace puede instalar `yt-dlp` por
  usted y mantenerlo actualizado. El campo **Ruta de yt-dlp:** queda fijado
  en la copia propia de Vivace y no se puede editar directamente; use el
  botón **Instalar / actualizar yt-dlp…** (junto a la casilla) siempre que
  quiera obtener la última versión oficial. El ajuste **Actualizar yt-dlp
  automáticamente:** también queda disponible, permitiendo que Vivace
  ejecute esta actualización por su cuenta — **Nunca**, o bien **Cada vez
  que se ejecuta yt-dlp**, o una vez al **día** o a la **semana**. Una
  actualización automática se ejecuta justo antes de resolver o descargar
  realmente una URL de YouTube, por lo que la primera reproducción después
  de que corresponda tarda un poco más; si la propia actualización falla
  (por ejemplo, por falta de red), Vivace continúa discretamente con la
  versión ya instalada en lugar de bloquear la reproducción.
- **Desactivada** — para un yt-dlp que usted mismo gestiona (por ejemplo,
  instalado mediante `pip` o el gestor de paquetes de su sistema operativo).
  El campo **Ruta de yt-dlp:** se vuelve editable para que pueda apuntarlo a
  esa copia, y **Actualizar yt-dlp automáticamente** queda desactivado —
  Vivace nunca instala ni actualiza un yt-dlp que no gestiona. El botón
  **Instalar / actualizar yt-dlp…** también queda desactivado en este modo.

## Exportar cookies para descargas de YouTube

El campo **Archivo de cookies:** (*Preferencias ▸ Red ▸ YouTube*) permite
que los modos de YouTube **Descargar y reproducir** y **Herramienta
externa** actúen como si hubiera iniciado sesión — necesario para vídeos
con restricción de edad, exclusivos para miembros o vinculados de otro modo
a una cuenta, y es lo que además desbloquea las descargas en HD/4K
completa. Se espera un archivo de texto plano `cookies.txt` en el clásico
formato Netscape de cookies (el mismo formato que lee la propia opción
`--cookies` de yt-dlp); Vivace no lee las cookies directamente del perfil
de un navegador.

**Para crear uno:**

1. Inicie sesión en youtube.com en su navegador habitual, con la cuenta
   cuyo acceso desea usar.
2. Instale una extensión del navegador para exportar cookies que escriba en
   formato Netscape — para Chrome, Edge o Brave, busque en la tienda de
   extensiones de su navegador algo como "Get cookies.txt"; para Firefox,
   busque "cookies.txt". Cualquier extensión que indique claramente que
   exporta en el formato clásico Netscape `cookies.txt` funcionará.
3. Con youtube.com abierto en una pestaña, use la extensión para exportar
   las cookies de ese sitio y guarde el resultado en algún lugar del disco
   como archivo `.txt`.
4. En Vivace, abra *Preferencias ▸ Red ▸ YouTube* y use **Examinar…** junto
   a **Archivo de cookies:** para seleccionar ese archivo.

**Tenga en cuenta:**

- Un archivo `cookies.txt` es, en la práctica, una sesión de inicio de
  sesión guardada: cualquiera que tenga el archivo puede actuar como su
  cuenta de YouTube hasta que las cookies caduquen o cierre sesión.
  Guárdelo en un lugar privado y no lo comparta.
- Las cookies solo se usan en la vía de **descarga** (Descargar y
  reproducir / Herramienta externa). Vivace deliberadamente nunca envía
  cookies en modo de **transmisión** — una URL de transmisión con la
  sesión iniciada queda vinculada a esa sesión de un modo que el
  reproductor de vídeo sencillo de Vivace no puede abrir, así que la
  transmisión sigue siendo anónima aunque haya un archivo de cookies
  configurado.
- Las cookies caducan. Si descargas que antes funcionaban empiezan a
  fallar, o se recurre a un resultado público o de menor calidad, exporte
  un `cookies.txt` nuevo.

## Instalación de ffmpeg para descargas de YouTube

El modo **Descargar y reproducir** necesita `ffmpeg` para combinar las
transmisiones de vídeo y audio independientes que descarga yt-dlp en un
único archivo reproducible — YouTube rara vez ofrece la HD como una única
transmisión combinada, así que una pista de vídeo y una de audio se
descargan por separado y luego se combinan. El campo **Ubicación de
ffmpeg:** (*Preferencias ▸ Red ▸ YouTube ▸ Descargar y reproducir*) le
indica a yt-dlp dónde encontrarlo; déjelo vacío para usar en su lugar el
`ffmpeg` del PATH de su sistema.

**Para instalar ffmpeg:**

1. **Windows** — la opción más sencilla es un gestor de paquetes:
   `winget install ffmpeg` (o `scoop install ffmpeg` / `choco install
   ffmpeg`). También puede descargar un archivo precompilado desde
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) o
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) y
   descomprimirlo en algún lugar.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — instálelo desde el gestor de paquetes de su distribución, p.
   ej. `sudo apt install ffmpeg` (Debian/Ubuntu), `sudo dnf install
   ffmpeg` (Fedora), o `sudo pacman -S ffmpeg` (Arch).
4. Si añadió ffmpeg al PATH de su sistema, deje **Ubicación de ffmpeg:**
   vacío. En caso contrario, pegue en ese campo la ruta de la *carpeta* que
   contiene el ejecutable `ffmpeg` (no el ejecutable en sí).
5. Reinicie Vivace (o simplemente reintente una descarga) después de
   instalarlo.

**Tenga en cuenta:**

- Esto es una dependencia de **yt-dlp**, igual que Deno más abajo — Vivace
  solo lo ejecuta como un proceso externo.
- El modo **Transmisión** nunca necesita ffmpeg, ya que reproduce una única
  transmisión ya combinada; solo **Descargar y reproducir** lo necesita,
  porque ese modo obtiene el vídeo y el audio por separado y los combina
  localmente.
- Si una descarga falla con un error relacionado con la combinación,
  compruebe primero la ubicación de ffmpeg — es la causa más habitual,
  aparte de un Deno faltante o desactualizado.

## Instalación de Deno para descargas de YouTube

El propio yt-dlp —no solo Vivace— usa un entorno de ejecución de JavaScript
externo independiente para resolver los desafíos que presenta YouTube antes
de entregar la URL de descarga real de un vídeo. Según la propia
documentación de yt-dlp, funcionar sin uno está "obsoleto" (deprecated) pero
no falla directamente: la disponibilidad de formatos simplemente se reduce,
y **de forma severa en una solicitud con sesión iniciada (con cookies)** —
justo el tipo de solicitud que hace el modo **Descargar y reproducir** para
desbloquear vídeos en HD, exclusivos para miembros y con restricción de
edad. El modo **Transmisión** nunca envía cookies (consulte «Exportar
cookies para descargas de YouTube» más arriba), así que no es el caso grave
y funciona bien sin Deno en la mayoría de los casos. Por eso el campo
**Ruta de Deno:** se encuentra en *Preferencias ▸ Red ▸ YouTube ▸ Descargar
y reproducir*, y no como un ajuste general de YouTube. yt-dlp admite varios
entornos de JavaScript; Deno es el que busca de forma predeterminada.

**Para instalar Deno:**

1. Siga las instrucciones de instalación oficiales en
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   para su sistema operativo (un script instalador, o un gestor de paquetes
   como winget/scoop/Homebrew/apt, según la plataforma).
2. Asegúrese de que el ejecutable `deno` termine en el PATH de su sistema —
   los instaladores anteriores normalmente lo hacen por usted. En Windows,
   asegúrese de obtener `deno`, no `denort` (un ejecutable diferente,
   relacionado, que no funcionará aquí).
3. Si prefiere no modificar el PATH, déjelo como está y en su lugar pegue su
   ruta completa en **Ruta de Deno:** (*Preferencias ▸ Red ▸ YouTube ▸
   Descargar y reproducir*).
4. Reinicie Vivace (o simplemente reintente una descarga) después de
   instalarlo.

**Tenga en cuenta:**

- Esto es una dependencia de **yt-dlp**, no de Vivace directamente — Vivace
  solo ejecuta yt-dlp como un proceso externo y nunca invoca Deno por sí
  mismo.
- yt-dlp requiere una versión de Deno razonablemente reciente (2.3.0 o
  posterior en el momento de escribir esto). Si las descargas siguen
  mostrando calidad reducida o errores de formato tras la instalación,
  compruebe `deno --version` y actualícelo si es más antiguo.
- Este requisito proviene de cambios en el lado de YouTube/yt-dlp, no de
  Vivace — el mismo campo **Ruta de Deno:** existe precisamente por este
  motivo y no necesita más configuración una vez que el propio Deno está
  instalado y accesible.

## Suavizado de subtítulos de mapa de bits

*Preferencias ▸ Subtítulos ▸ Subtítulos de mapa de bits* incluye un
ajuste **Suavizado:** (0–3, valor predeterminado 1) para los subtítulos
que se representan como imágenes en lugar de texto: pistas de subimagen
de DVD, PGS y DVB. Esto abarca tanto los subtítulos propios de un disco
DVD real como una pista de subtítulos incrustada del mismo tipo en un
archivo de vídeo normal (por ejemplo, un archivo .mp4 con una pista de
códec `dvd_subtitle`). Estos formatos son imágenes de mapa de bits
prerrenderizadas, grabadas a la resolución nativa de definición
estándar (SD) al crear la fuente — sus bordes pueden verse dentados al
ampliarse al tamaño de una ventana moderna. Vivace puede aplicar un
ligero desenfoque para suavizar esos bordes:

- **0** — desactivado; muestra el mapa de bits de subtítulos original
  tal cual se creó.
- **1** (valor predeterminado) — suaviza los bordes más pronunciados
  manteniendo el texto con un brillo prácticamente completo.
- **2** / **3** — desenfoque progresivamente mayor.

Este ajuste solo afecta a los subtítulos de mapa de bits — no tiene
ningún efecto en el renderizador de subtítulos externo propio de Vivace
(SRT/VTT/ASS) ni en las pistas de subtítulos de texto normales, que
usan rutas de renderizado distintas.
