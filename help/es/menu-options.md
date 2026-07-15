# Opciones

El menú **Opciones** contiene las preferencias y la configuración de la interfaz.

- **Preferencias…** (`Ctrl+P`) — el cuadro de diálogo principal de ajustes:
  General, Interfaz, Subtítulos, Teclado y ratón, Lista de reproducción,
  Unidades, TV, Tipos de archivo, Actualizaciones, Red y Avanzado. Los cambios se
  aplican al instante; **Cancelar** los revierte.
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
