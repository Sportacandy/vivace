# Opciones de línea de comandos

Vivace puede iniciarse desde un terminal con un archivo multimedia y algunas
opciones. Use opciones largas de estilo GNU (los alias cortos se muestran entre
paréntesis).

```
vivace [opciones] [medio]
```

- **medio** — una ruta de archivo o URL para reproducir. Aquí funciona todo lo
  que Vivace puede abrir normalmente (archivo local, carpeta, `VIDEO_TS`, emisión
  de red, URL de YouTube).

## Opciones

- **`--help` (`-h`)** — mostrar la lista de opciones y salir.
- **`--version` (`-v`)** — mostrar la versión y salir.
- **`--fullscreen` (`-f`)** — iniciar en pantalla completa.
- **`--ontop` (`-t`)** — mantener la ventana por encima de las demás.
- **`--close-at-end` (`-c`)** — cerrar Vivace cuando termine la reproducción.
- **`--sub <archivo>` (`-s`)** — cargar el archivo de subtítulos indicado.
- **`--start <tiempo>` (`-b`)** — comenzar la reproducción en `<tiempo>`, indicado
  como `h:m:s`, `m:s` o un número de segundos.
- **`--pos <x,y>`** — situar la ventana en la posición de pantalla `x,y`.
- **`--size <an,al>`** — establecer el tamaño de la ventana a `an`×`al`.

## Ejemplos

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Nota (Windows):** Vivace es una aplicación gráfica, por lo que `--help` /
`--version` imprimen en el terminal desde el que lo inició (puede que el símbolo
del sistema vuelva primero, así que el texto puede aparecer después). Cuando el
modo de instancia única está activado (*Preferencias ▸ Interfaz ▸ Instancias*),
volver a iniciarlo con un archivo multimedia y estas opciones las aplica a la
ventana ya en ejecución.
