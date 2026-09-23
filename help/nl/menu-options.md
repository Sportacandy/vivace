# Opties

Het menu **Opties** bevat de voorkeuren en de interfaceconfiguratie.

- **Voorkeuren…** (`Ctrl+P`) — het hoofddialoogvenster met instellingen:
  Algemeen, Interface, Ondertitels, Toetsenbord en muis, Afspeellijst, Stations,
  Tv, Bestandstypen, Updates, Netwerk en Geavanceerd. Wijzigingen worden direct
  toegepast; **Annuleren** draait ze terug.
  - **Algemeen ▸ Video** stelt de standaardmodus voor **Deinterlace** in
    (Geen / Yadif / Bwdif / Automatisch) voor nieuw geopende bestanden —
    wijzig dit per bestand via *Video ▸ Deinterlace* (alleen Geen/Yadif/
    Bwdif; Automatisch wordt daar niet aangeboden, omdat het alleen als
    standaardwaarde zin heeft). **Automatisch** gebruikt Bwdif, maar
    alleen op frames die het bestand zelf als interlaced markeert;
    progressieve frames blijven ongewijzigd.
  - **Algemeen ▸ Audio- en ondertiteling-voorkeuren** stelt de talen in
    waaruit Vivace automatisch kiest tussen de ingesloten sporen van een
    bestand. **Ondertitels standaard tonen** schakelt er automatisch een
    in wanneer beschikbaar; de suboptie **...maar niet als de audio al in
    een voorkeurstaal is** slaat die automatische ondertiteling over als
    de taal overeenkomt met het daadwerkelijk geselecteerde audiospoor —
    handig als u het audio al begrijpt en geen afleidende ondertiteling in
    dezelfde taal wilt.
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

## De YouTube PO-tokenprovider installeren

Steeds meer recente YouTube-video's vereisen tegenwoordig een
**PO-token (proof-of-origin)** om sowieso te kunnen afspelen — een
algemene afspeelbaarheidsvereiste, los van cookies of aangemeld zijn.
Zonder dit token meldt yt-dlp dat de video niet beschikbaar is, zelfs bij
een gewoon, anoniem verzoek. Dit geldt voor **beide** modi, Streaming en
Downloaden en afspelen, in tegenstelling tot cookies (alleen voor
downloaden, zie hieronder) of Deno (vooral van belang bij Downloaden en
afspelen, zie verderop).

*Voorkeuren ▸ Netwerk ▸ YouTube* heeft een knop **PO-tokenprovider
installeren…** die het community-project **"BgUtils POT Provider"**
instelt: een kleine yt-dlp-plug-in plus een script — dat op aanvraag via
Deno wordt uitgevoerd, hetzelfde programma dat verderop wordt behandeld —
waarmee yt-dlp automatisch een token kan genereren. Klik erop, wacht tot
het korte download-en-bouwproces is voltooid, en het is klaar; het label
van de knop verandert in **PO-tokenprovider opnieuw
installeren/bijwerken…** zodra het geïnstalleerd is, voor het geval er
later ooit een update nodig is.

**Houd er rekening mee:**

- Dit vereist een werkende Deno-installatie (zie "Deno installeren voor
  YouTube-downloads" hieronder) — de knop downloadt en bouwt het eigen
  script van de provider, dat yt-dlp vervolgens via Deno uitvoert wanneer
  een video daadwerkelijk een token nodig heeft.
- Niet elke video heeft een PO-token nodig, dus afspelen kan prima werken
  zonder dat dit geïnstalleerd is — maar als een video zich als niet
  beschikbaar meldt terwijl deze elders wel prima afspeelt, is dit de
  moeite waard om te installeren.
- **Op Android** wordt dit met Vivace meegeleverd en automatisch
  ingesteld — er is niets te installeren en er is geen vergelijkbare
  knop; het werkt gewoon.

## Cookies voor YouTube-downloads

De YouTube-modi **Downloaden en afspelen** en **externe tool** kunnen zich
gedragen alsof u bent aangemeld — nodig voor video's met
leeftijdsbeperking, video's die alleen voor leden toegankelijk zijn, of
anderszins aan een account gebonden video's, en het is wat volledige
HD/4K-downloads ontgrendelt. Vivace ondersteunt drie manieren om cookies
aan te leveren (allemaal onder *Voorkeuren ▸ Netwerk ▸ YouTube*):
**Cookies ophalen uit browser** op Windows/Linux/macOS, **Inloggen bij
YouTube…** op Android, en het handmatig exporteren van een
`cookies.txt`-bestand als alternatief overal.

### Cookies ophalen uit browser (aanbevolen op Windows/Linux/macOS)

Het keuzevak **Cookies ophalen uit browser:** biedt Firefox, Chrome, Edge,
Brave, Chromium, Opera, Safari, Vivaldi en Whale. Kies uw browser en
Vivace leest de cookies ervan live, elke keer opnieuw — niets om te
exporteren, niets dat verouderd raakt.

**Houd er rekening mee:**

- YouTube heeft de levensduur van zijn eigen cookies aanzienlijk verkort,
  waardoor een eerder geëxporteerd `cookies.txt`-bestand (zie hieronder)
  binnen enkele dagen verouderd kan raken — deze optie voorkomt dit
  volledig door de eigen, altijd actuele cookie-opslag van de browser te
  lezen.
- **Op Windows werkt hier alleen Firefox daadwerkelijk.** Chrome, Edge en
  andere op Chromium gebaseerde browsers op Windows versleutelen hun
  cookies op een manier die gekoppeld is aan het eigen uitvoerbare
  bestand van de browser ("App-Bound Encryption", Chrome 127+) — dit
  blokkeert yt-dlp, en elk ander extern hulpprogramma, volledig van het
  lezen ervan. Het is een beperking aan de kant van Chrome; de
  ontwikkelaars van yt-dlp zelf kunnen dit niet omzeilen. Chrome/Edge op
  Linux en macOS ondervinden hier geen last van en werken normaal.
- Het selecteren van een browser hier heeft voorrang op het veld
  **Cookiebestand:** hieronder, wanneer beide zijn ingesteld.
- **Niet beschikbaar op Android** — zie **Inloggen bij YouTube**
  hieronder in plaats daarvan.

### Inloggen bij YouTube (Android, daar aanbevolen)

*Voorkeuren ▸ Netwerk ▸ YouTube* heeft een knop **Inloggen bij
YouTube…** (naast het veld voor het cookiebestand) die een echte
aanmeldpagina binnen Vivace zelf opent, met behulp van een ingebouwde,
systeemeigen WebView. Meld u aan met het account waarvan u de toegang
wilt gebruiken en tik vervolgens op **Ik ben ingelogd** — Vivace leest
de resulterende sessiecookies en slaat ze automatisch op als het actieve
cookiebestand. Dit is het Android-equivalent van **Cookies ophalen uit
browser** hierboven: niets om te exporteren, niets om handmatig van een
ander apparaat over te zetten.

**Houd er rekening mee:**

- Android heeft helemaal geen equivalent van een live cookie-opslag van
  een desktopbrowser om uit te lezen (de opslag van elke app is
  afgeschermd van elke andere app) — daarom werkt dit anders dan de
  desktopoptie: aanmelden *binnen* de ingebouwde browser van Vivace zelf
  is het praktische alternatief.
- Alleen beschikbaar op Android; elk ander platform gebruikt in plaats
  daarvan **Cookies ophalen uit browser**.

### Cookies naar een bestand exporteren (alternatief op elk platform)

Het veld **Cookiebestand:** verwacht een gewoon tekstbestand `cookies.txt`
in het klassieke Netscape-cookiejar-formaat (hetzelfde formaat dat
yt-dlp's eigen `--cookies`-optie leest) — gebruik dit wanneer geen van
beide bovenstaande live-opties beschikbaar is of voor u werkt (bijv.
Chrome/Edge op Windows).

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

**Op Android:** gebruik in plaats daarvan liever **Inloggen bij
YouTube…** hierboven — dat vereist helemaal geen export-/overzetstap.
Als u het toch op deze manier wilt doen: Chrome voor Android ondersteunt
geen browserextensies, dus
stap 2–3 hierboven kunnen niet op het apparaat zelf worden uitgevoerd.
Exporteer `cookies.txt` zoals hierboven beschreven op een desktop- of
laptopcomputer, en breng dat bestand vervolgens over naar uw
Android-apparaat (bijvoorbeeld via cloudopslag, een USB-kabel of e-mail)
voordat u **Bladeren…** gebruikt in stap 4.

**Houd er rekening mee:**

- Een `cookies.txt`-bestand is in feite een opgeslagen aanmeldsessie —
  iedereen die het bestand heeft, kan zich voordoen als uw YouTube-account
  totdat de cookies verlopen of u zich afmeldt. Bewaar het ergens privé en
  deel het niet.
- Cookies verlopen. Als downloads die eerder werkten beginnen te mislukken,
  of terugvallen op een resultaat van lagere kwaliteit/openbaar resultaat,
  exporteert u een nieuw `cookies.txt`-bestand — of schakelt u over naar
  **Cookies ophalen uit browser** hierboven, indien beschikbaar, om dit
  volledig te vermijden.

**Geldt voor beide bovenstaande methoden:**

- Cookies worden alleen gebruikt door het **download**-pad (Downloaden en
  afspelen / externe tool). Vivace stuurt bewust nooit cookies in
  **streaming**-modus — een aangemelde stream-URL is aan die sessie gebonden
  op een manier die Vivace's eenvoudige videospeler niet kan openen, dus
  streaming blijft anoniem, ongeacht of cookies op een van beide manieren
  zijn geconfigureerd.

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
**Streaming**-modus stuurt nooit cookies (zie "Cookies voor
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
