# Afspil

Menuen **Afspil** styrer afspilning og søgning.

## Transport

- **Afspil**, **Pause**, **Stop** — `Mellemrum` skifter mellem afspil/pause;
  `Ctrl+Mellemrum` stopper.
- **Billede frem** (`.`) og **Billede tilbage** — ét billede frem eller tilbage.
- **Forrige** / **Næste** — flyt mellem elementer i afspilningslisten.

## Søgning

Der tilbydes tre søgetrin i begge retninger (**Tilbage** / **Frem**), vist med
deres aktuelle varighed. Justér trinstørrelserne i *Indstillinger ▸ Grænseflade ▸
Søgning*. Du kan også søge med `←` / `→`, søgeskyderen eller musehjulet over videoen.

## Hastighed

- **Normal hastighed** (`Tilbagetast`), **Halvér hastighed**, **Fordobl hastighed**.
- **Hastighed −10 % / +10 %** og finere trin på ±4 % / ±1 %.
- **Tonehøjdekompensation** — holder stemmer naturlige ved ikke-normal hastighed.

Afspilningshastigheden gælder pr. fil og går tilbage til normal for hver ny fil.

## A-B-afsnit

Gentag en del af en fil:

- **Sæt A-markør**, **Sæt B-markør** — markér begyndelsen og slutningen.
- **Gentag** — gentag A-B-området (afsnittet gentages kun, mens dette er slået til).
- **Ryd A-B-markører** — fjern dem. Markører nulstilles for hver ny fil.

**Gå til…** (gå til et bestemt tidspunkt) er planlagt til en senere version.

## Udsendelse

**Udsend til ▸ Smartphone/tablet…** starter en lille indbygget webserver, så
en telefon eller tablet på samme netværk kan åbne en side og afspille den fil,
Vivace afspiller lige nu — ved at scanne QR-koden eller indtaste den viste
adresse. Porten er fast i *Indstillinger ▸ Netværk ▸ Udsendelse* (kan ikke
ændres pr. session), så du kan tillade den én gang i din firewall/router. Kun
den fil, der afspilles lige nu, sendes, og kun mens udsendelse er slået til.
