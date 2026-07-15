# Reprodueix

El menú **Reprodueix** controla la reproducció i la cerca.

## Transport

- **Reprodueix**, **Pausa**, **Atura** — `Espai` alterna reproduir/pausar;
  `Ctrl+Espai` atura.
- **Fotograma endavant** (`.`) i **Fotograma enrere** — avançar o retrocedir un fotograma.
- **Anterior** / **Següent** — moure's entre els elements de la llista de reproducció.

## Cerca

S'ofereixen tres mides de salt en ambdós sentits (**Enrere** / **Endavant**),
mostrades amb la seva durada actual. Ajusteu les mides a *Preferències ▸
Interfície ▸ Cerca*. També podeu cercar amb `←` / `→`, la barra de cerca o la roda del ratolí sobre el vídeo.

## Velocitat

- **Velocitat normal** (`Retrocés`), **Meitat de velocitat**, **Doble velocitat**.
- **Velocitat −10% / +10%** i passos més fins de ±4% / ±1%.
- **Compensació de to** — manté les veus naturals a velocitats no normals.

La velocitat de reproducció és per fitxer i torna a la normal amb cada fitxer nou.

## Secció A-B

Repetir una part d'un fitxer:

- **Posa el marcador A**, **Posa el marcador B** — marcar l'inici i el final.
- **Repeteix** — repetir la regió A-B (la secció només es repeteix mentre això estigui activat).
- **Neteja els marcadors A-B** — eliminar-los. Els marcadors es reinicien amb cada fitxer nou.

**Vés a…** (anar a un moment concret) està previst per a una versió posterior.

## Transmissió

**Transmet a ▸ Mòbil/tauleta…** inicia un petit servidor web integrat perquè
un telèfon o una tauleta a la mateixa xarxa pugui obrir una pàgina i
reproduir el fitxer que Vivace està reproduint en aquell moment — escanejant
el codi QR o escrivint l'adreça mostrada. El port és fix a *Preferències ▸
Xarxa ▸ Transmissió* (no editable a cada sessió), de manera que només cal
permetre'l una vegada al tallafoc/encaminador. Només es transmet el fitxer
que s'està reproduint, i només mentre la transmissió estigui activada.
