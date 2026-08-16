# Opties

Het menu **Opties** bevat de voorkeuren en de interfaceconfiguratie.

- **Voorkeuren…** (`Ctrl+P`) — het hoofddialoogvenster met instellingen:
  Algemeen, Interface, Ondertitels, Toetsenbord en muis, Afspeellijst, Stations,
  Tv, Bestandstypen, Updates, Netwerk en Geavanceerd. Wijzigingen worden direct
  toegepast; **Annuleren** draait ze terug.
  - **Algemeen ▸ Video** stelt de standaardmodus voor **Deinterlace** in
    (Geen / Yadif / Bwdif) voor nieuw geopende bestanden — wijzig dit per
    bestand via *Video ▸ Deinterlace*.
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

## yt-dlp installeren en bijwerken

*Voorkeuren ▸ Netwerk ▸ YouTube* heeft een selectievakje **Beheerde yt-dlp
gebruiken** dat bepaalt hoe Vivace het programma `yt-dlp` verkrijgt en
onderhoudt dat wordt gebruikt om YouTube-links af te spelen:

- **Aan** (standaard) — Vivace kan `yt-dlp` voor u installeren en actueel
  houden. Het veld **yt-dlp-pad:** staat vast op Vivace's eigen exemplaar en
  kan niet rechtstreeks worden bewerkt; gebruik de knop **yt-dlp
  installeren/bijwerken…** (naast het selectievakje) wanneer u de nieuwste
  officiële release wilt ophalen. De instelling **yt-dlp automatisch
  bijwerken:** wordt dan ook beschikbaar, zodat Vivace deze update zelf kan
  uitvoeren — **Nooit**, **Elke keer dat yt-dlp wordt uitgevoerd**, of één
  keer per **dag**/**week**. Een automatische update wordt uitgevoerd vlak
  voordat een YouTube-URL daadwerkelijk wordt herleid of gedownload, dus de
  eerste keer afspelen nadat de update verschuldigd is geraakt, duurt iets
  langer; als de update zelf mislukt (bijv. geen netwerkverbinding), gaat
  Vivace stilletjes verder met de reeds geïnstalleerde versie in plaats van
  het afspelen te blokkeren.
- **Uit** — voor een yt-dlp die u zelf beheert (bijvoorbeeld geïnstalleerd
  via `pip` of de pakketbeheerder van uw besturingssysteem). Het veld
  **yt-dlp-pad:** wordt bewerkbaar, zodat u het naar dat exemplaar kunt laten
  wijzen, en **yt-dlp automatisch bijwerken** is uitgeschakeld — Vivace
  installeert of werkt nooit een yt-dlp bij die het niet zelf beheert. De
  knop **yt-dlp installeren/bijwerken…** is in deze modus ook uitgeschakeld.

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

## ffmpeg installeren voor YouTube-downloads

**Downloaden en afspelen** heeft `ffmpeg` nodig om de aparte video- en
audiostromen die yt-dlp downloadt samen te voegen tot één afspeelbaar
bestand — YouTube biedt HD zelden als één gecombineerde stream aan, dus een
videospoor en een audiospoor worden apart gedownload en vervolgens
samengevoegd. Het veld **ffmpeg-locatie:** (*Voorkeuren ▸ Netwerk ▸ YouTube ▸
Downloaden en afspelen*) vertelt yt-dlp waar het dit kan vinden; laat het leeg
om in plaats daarvan `ffmpeg` van het PATH van uw systeem te gebruiken.

**Zo installeert u ffmpeg:**

1. **Windows** — de eenvoudigste optie is een pakketbeheerder:
   `winget install ffmpeg` (of `scoop install ffmpeg` / `choco install
   ffmpeg`). U kunt ook een kant-en-klaar archief downloaden van
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) of
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) en dit ergens
   uitpakken.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — installeer het via de pakketbeheerder van uw distributie,
   bijv. `sudo apt install ffmpeg` (Debian/Ubuntu), `sudo dnf install ffmpeg`
   (Fedora), of `sudo pacman -S ffmpeg` (Arch).
4. Als u ffmpeg aan het PATH van uw systeem hebt toegevoegd, laat u
   **ffmpeg-locatie:** leeg. Plak anders het pad naar de *map* met het
   uitvoerbare bestand `ffmpeg` (niet het uitvoerbare bestand zelf) in dat
   veld.
5. Start Vivace opnieuw (of probeer gewoon opnieuw een download) na de
   installatie.

**Houd er rekening mee:**

- Dit is een afhankelijkheid van **yt-dlp**, net als Deno hieronder — Vivace
  voert het altijd uit als een extern proces.
- **Streaming**-modus heeft nooit ffmpeg nodig, omdat deze een reeds
  gecombineerde stream afspeelt; alleen **Downloaden en afspelen** wel, omdat
  die modus video en audio apart ophaalt en ze lokaal samenvoegt.
- Als een download mislukt met een foutmelding over samenvoegen, controleer
  dan eerst de ffmpeg-locatie — dat is de meest voorkomende oorzaak, naast
  een ontbrekende of verouderde Deno.

## Deno installeren voor YouTube-downloads

yt-dlp zelf — niet alleen Vivace — gebruikt een aparte, externe
JavaScript-runtime om de uitdagingen op te lossen die YouTube opwerpt voordat
het de echte download-URL van een video vrijgeeft. Volgens de eigen
documentatie van yt-dlp is het draaien zonder zo'n runtime "deprecated" maar
niet meteen kapot: de beschikbaarheid van formaten wordt gewoon beperkt, en
**vooral ernstig bij een aangemeld (cookie-)verzoek** — precies het soort
verzoek dat de modus **Downloaden en afspelen** doet om HD-,
alleen-voor-leden- en leeftijdsbeperkte video's te ontgrendelen.
**Streaming**-modus stuurt nooit cookies (zie "Cookies exporteren voor
YouTube-downloads" hierboven), dus dat is niet het ernstige geval en werkt in
de meeste gevallen prima zonder Deno. Dit is waarom het veld
**Deno-locatie:** te vinden is bij *Voorkeuren ▸ Netwerk ▸ YouTube ▸
Downloaden en afspelen*, en niet als algemene YouTube-instelling. yt-dlp
ondersteunt meerdere JS-runtimes; Deno is degene waarnaar standaard wordt
gezocht.

**Zo installeert u Deno:**

1. Volg de officiële installatie-instructies op
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   voor uw besturingssysteem (een installatiescript, of een pakketbeheerder
   zoals winget/scoop/Homebrew/apt, afhankelijk van het platform).
2. Zorg dat het uitvoerbare bestand `deno` terechtkomt op het PATH van uw
   systeem — de installatieprogramma's hierboven doen dit normaal gesproken
   automatisch voor u. Zorg er op Windows voor dat u `deno` krijgt, niet
   `denort` (een ander, verwant uitvoerbaar bestand dat hier niet werkt).
3. Als u liever niets aan PATH wijzigt, laat u het zoals het is en plakt u
   in plaats daarvan het volledige pad ernaartoe in **Deno-locatie:**
   (*Voorkeuren ▸ Netwerk ▸ YouTube ▸ Downloaden en afspelen*).
4. Start Vivace opnieuw (of probeer gewoon opnieuw een download) na de
   installatie.

**Houd er rekening mee:**

- Dit is een afhankelijkheid van **yt-dlp**, niet rechtstreeks van Vivace —
  Vivace voert yt-dlp altijd uit als een extern proces en roept Deno zelf
  nooit aan.
- yt-dlp vereist een redelijk recente versie van Deno (op het moment van
  schrijven 2.3.0 of hoger). Als downloads na installatie nog steeds een
  beperkte kwaliteit of formaatfouten tonen, controleert u `deno --version`
  en werkt u het bij als het een oudere versie is.
- Deze vereiste komt voort uit veranderingen aan de kant van YouTube/yt-dlp,
  niet van Vivace — precies om deze reden bestaat het veld
  **Deno-locatie:**, en er is verder geen configuratie nodig zodra Deno zelf
  is geïnstalleerd en bereikbaar is.

## Verzachting van bitmap-ondertiteling

*Voorkeuren ▸ Ondertiteling ▸ Bitmap-ondertiteling* heeft een instelling
**Verzachting:** (0–3, standaard 1) voor ondertiteling die als
afbeeldingen in plaats van tekst wordt weergegeven: dvd-subpicture-,
PGS- en DVB-sporen. Dit geldt zowel voor de eigen ondertiteling van een
echte dvd als voor een ingesloten ondertitelingsspoor van hetzelfde type
in een gewoon videobestand (bijvoorbeeld een .mp4-bestand met een spoor
met de `dvd_subtitle`-codec). Deze formaten zijn vooraf gerenderde
bitmapafbeeldingen, ingebakken op de originele
standaarddefinitie(SD)-resolutie toen de bron werd gemaakt — de randen
kunnen er gekarteld uitzien wanneer ze worden vergroot naar een moderne
venstergrootte. Vivace kan een lichte vervaging toepassen om die randen
te verzachten:

- **0** — uit; toont de originele ondertitelingsbitmap precies zoals
  gemaakt.
- **1** (standaard) — verzacht de grofste randen terwijl de tekst
  vrijwel volledig helder blijft.
- **2** / **3** — geleidelijk meer vervaging.

Deze instelling heeft alleen invloed op bitmap-ondertiteling — ze heeft
geen effect op Vivace's eigen externe ondertitelingsrenderer
(SRT/VTT/ASS) of op gewone tekstgebaseerde ondertitelingssporen; beide
gebruiken een ander renderpad.
