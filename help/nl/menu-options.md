# Opties

Het menu **Opties** bevat de voorkeuren en de interfaceconfiguratie.

- **Voorkeuren…** (`Ctrl+P`) — het hoofddialoogvenster met instellingen:
  Algemeen, Interface, Ondertitels, Toetsenbord en muis, Afspeellijst, Stations,
  Tv, Bestandstypen, Updates, Netwerk en Geavanceerd. Wijzigingen worden direct
  toegepast; **Annuleren** draait ze terug.
  - **Netwerk** heeft de tabbladen OpenSubtitles, YouTube, Proxy en Casten;
    **Proxy** stelt een optionele HTTP- of SOCKS5-proxy in die
    applicatiebreed geldt (OpenSubtitles-zoekopdrachten, de update-controle
    en — alleen bij HTTP — mediaweergave en yt-dlp); **Casten** legt de poort
    vast waarop *Afspelen ▸ Casten naar ▸ Smartphone/tablet* luistert. Het
    OpenSubtitles-accountwachtwoord en het proxywachtwoord worden nu veilig
    opgeslagen via de referentiebeheerder van uw besturingssysteem, niet in
    Vivace's eigen instellingen.
- **Pictogram in systeemvak tonen** — Vivace bereikbaar houden via het systeemvak.
- **Werkbalken**
  - **Werkbalk** / **Bedieningsbalk** — elke balk tonen of verbergen.
  - **Hoofdwerkbalk bewerken…** / **Bedieningsbalk bewerken…** — kiezen welke
    knoppen verschijnen, hun volgorde en de pictogramgrootte.
- **Statusbalk**
  - **Statusbalk tonen** en wat ze toont: **Video-info**, **Audio-info**,
    **Formaatinfo**, **Bitrate-info**, **Frameteller**, **Totale tijd tonen**,
    **Resterende tijd tonen** en **Huidige tijd met milliseconden tonen**.

**Tip:** de algehele indeling (Basic / Mini / MPC) kiest u bij *Voorkeuren ▸ Interface*.

## Cookies exporteren voor YouTube-downloads

Met het veld **Cookiebestand:** (*Voorkeuren ▸ Netwerk ▸ YouTube*) kunnen de
YouTube-modi **Downloaden en afspelen** en **externe tool** zich gedragen
alsof u bent aangemeld — nodig voor video's met leeftijdsbeperking, video's
die alleen voor leden toegankelijk zijn, of anderszins aan een account
gebonden video's, en het is wat volledige HD/4K-downloads ontgrendelt. Het
verwacht een gewoon tekstbestand `cookies.txt` in het klassieke
Netscape-cookiejar-formaat (hetzelfde formaat dat yt-dlp's eigen
`--cookies`-optie leest); Vivace leest cookies niet rechtstreeks uit het
profiel van een browser.

**Zo maakt u er een:**

1. Meld u aan bij youtube.com in uw dagelijkse browser, met het account
   waarvan u de toegang wilt gebruiken.
2. Installeer een browserextensie voor het exporteren van cookies die het
   Netscape-formaat schrijft — zoek voor Chrome, Edge of Brave in de
   extensiewinkel van uw browser naar iets als "Get cookies.txt"; zoek voor
   Firefox naar "cookies.txt". Elke extensie die duidelijk aangeeft het
   klassieke Netscape-formaat `cookies.txt` te exporteren, werkt.
3. Terwijl youtube.com in een tabblad open staat, gebruikt u de extensie om
   cookies voor die site te exporteren en slaat u het resultaat ergens op
   schijf op als een `.txt`-bestand.
4. Open in Vivace *Voorkeuren ▸ Netwerk ▸ YouTube* en gebruik
   **Bladeren…** naast **Cookiebestand:** om dat bestand te selecteren.

**Houd er rekening mee:**

- Een `cookies.txt`-bestand is in feite een opgeslagen aanmeldsessie —
  iedereen die het bestand heeft, kan zich voordoen als uw YouTube-account
  totdat de cookies verlopen of u zich afmeldt. Bewaar het ergens privé en
  deel het niet.
- Cookies worden alleen gebruikt door het **download**-pad (Downloaden en
  afspelen / externe tool). Vivace stuurt bewust nooit cookies in
  **streaming**-modus — een aangemelde stream-URL is aan die sessie gebonden
  op een manier die Vivace's eenvoudige videospeler niet kan openen, dus
  streaming blijft anoniem, zelfs als er een cookiebestand is geconfigureerd.
- Cookies verlopen. Als downloads die eerder werkten beginnen te mislukken,
  of terugvallen op een resultaat van lagere kwaliteit/openbaar resultaat,
  exporteert u een nieuw `cookies.txt`-bestand.
