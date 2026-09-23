# Alternativ

Menyn **Alternativ** innehåller inställningarna och gränssnittskonfigurationen.

- **Inställningar…** (`Ctrl+P`) — huvuddialogrutan för inställningar: Allmänt,
  Gränssnitt, Undertexter, Tangentbord och mus, Spellista, Enheter, Tv,
  Filtyper, Uppdateringar, Nätverk och Avancerat. Ändringar tillämpas direkt;
  **Avbryt** återställer dem.
  - **Allmänt ▸ Video** ställer in standardläget för **Deinterlace** (Ingen /
    Yadif / Bwdif / Automatisk) för nyöppnade filer — ändra det per fil under
    *Video ▸ Deinterlace* (endast Ingen/Yadif/Bwdif; Automatisk erbjuds inte
    där, eftersom det bara är meningsfullt som standardval). **Automatisk**
    använder Bwdif men bara på bildrutor som filen själv märker som flätade,
    och lämnar progressiva bildrutor orörda.
  - **Allmänt ▸ Standardspråk - tal och undertexter** anger de språk som
    Vivace automatiskt väljer bland en fils inbäddade spår. **Visa
    undertexter som standard** slår automatiskt på en undertext när en är
    tillgänglig; dess underalternativ **...men inte om ljudet redan är på
    ett önskat språk** hoppar över den automatiska undertexten när dess
    språk matchar det ljudspår som faktiskt valdes — användbart om du
    redan förstår ljudet och inte vill att en undertext på samma språk ska
    distrahera.
  - **Nätverk** har flikarna OpenSubtitles, YouTube, Proxy och Casta; **Proxy**
    ställer in en valfri HTTP- eller SOCKS5-proxy som gäller för hela
    programmet (OpenSubtitles-sökning, uppdateringskontroll och — endast för
    HTTP — uppspelning och yt-dlp); **Casta** låser porten som *Spela upp ▸
    Casta till ▸ Smarttelefon/surfplatta* lyssnar på. Lösenordet för
    OpenSubtitles-kontot och proxylösenordet lagras säkert i operativsystemets
    autentiseringshanterare, inte i Vivaces egna inställningar.
- **Visa ikon i aktivitetsfältet** — håll Vivace nåbar från aktivitetsfältet.
- **Verktygsfält**
  - **Verktygsfält** / **Kontrollfält** — visa eller dölj varje fält.
  - **Redigera huvudverktygsfält…** / **Redigera kontrollfält…** — välj vilka
    knappar som visas samt deras ordning och ikonstorlek.
- **Statusfält**
  - **Visa statusfält** och vad det visar: **Videoinfo**, **Ljudinfo**,
    **Formatinfo**, **Bithastighetsinfo**, **Bildräknare**, **Visa total tid**,
    **Visa återstående tid** och **Visa aktuell tid med millisekunder**.

**Tips:** den övergripande layouten (Basic / Mini / MPC) väljs i *Inställningar ▸ Gränssnitt*.

## Installera och uppdatera yt-dlp

*Inställningar ▸ Nätverk ▸ YouTube* har kryssrutan **Använd hanterad
yt-dlp**, som styr hur Vivace hämtar och underhåller programmet `yt-dlp`,
som det använder för att spela upp YouTube-länkar:

- **På** (standard) — Vivace kan installera `yt-dlp` åt dig och hålla det
  uppdaterat. Fältet **yt-dlp-sökväg:** är låst till Vivaces egen kopia och
  kan inte redigeras direkt; använd knappen **Installera/uppdatera
  yt-dlp…** (bredvid kryssrutan) när du vill hämta den senaste officiella
  utgåvan. Inställningen **Uppdatera yt-dlp automatiskt:** blir också
  tillgänglig, vilket låter Vivace sköta denna uppdatering själv:
  **Aldrig**, **Varje gång yt-dlp körs**, **En gång om dagen** eller **En
  gång i veckan**. En automatisk uppdatering körs precis innan en
  YouTube-URL faktiskt slås upp eller laddas ner, så den första
  uppspelningen efter att den förfaller tar lite längre tid; om själva
  uppdateringen misslyckas (t.ex. ingen nätverksanslutning) fortsätter
  Vivace tyst med den version som redan är installerad i stället för att
  blockera uppspelningen.
- **Av** — för en yt-dlp som du hanterar själv (t.ex. installerad via
  `pip` eller operativsystemets pakethanterare). Fältet **yt-dlp-sökväg:**
  blir redigerbart så att du kan peka det mot den kopian, och **Uppdatera
  yt-dlp automatiskt** inaktiveras — Vivace installerar eller uppdaterar
  aldrig en yt-dlp som det inte hanterar. Knappen **Installera/uppdatera
  yt-dlp…** inaktiveras också i detta läge.

## Installera YouTube PO-tokenleverantören

Nyare YouTube-videor kräver allt oftare en **PO-token (proof-of-origin)**
bara för att kunna spelas upp alls — ett allmänt krav för spelbarhet,
oberoende av cookies eller inloggning. Utan en sådan rapporterar yt-dlp
videon som otillgänglig även för en vanlig, anonym begäran. Detta gäller
**både** lägena Strömning och Ladda ned och spela upp, till skillnad från
cookies (endast för nedladdning, se nedan) eller Deno (huvudsakligen en
fråga för Ladda ned och spela upp, se längre ned).

*Inställningar ▸ Nätverk ▸ YouTube* har knappen **Installera
PO-tokenleverantör…**, som installerar communityprojektet **"BgUtils POT
Provider"**: ett litet yt-dlp-tillägg plus ett skript — som körs vid behov
via Deno, samma program som beskrivs längre ned — vilket låter yt-dlp
generera en token automatiskt. Klicka på den, vänta tills den korta
processen för nedladdning och byggande är klar, så är det klart; knappens
etikett ändras till **Installera om/uppdatera PO-tokenleverantör…** när
den väl är installerad, ifall en senare uppdatering någonsin behövs.

**Tänk på:**

- Detta kräver en fungerande Deno-installation (se "Installera Deno för
  YouTube-nedladdningar" nedan) — knappen laddar ned och bygger
  leverantörens eget skript, som yt-dlp sedan kör via Deno närhelst en
  video faktiskt behöver en token.
- Inte alla videor behöver en PO-token, så uppspelning kan fungera fint
  utan detta installerat — men om en video rapporterar sig själv som
  otillgänglig när den fungerar fint på andra ställen är det värt att
  installera detta.
- **På Android** ingår detta med Vivace och konfigureras automatiskt —
  det finns inget att installera och ingen motsvarande knapp; det
  fungerar bara.

## Cookies för YouTube-nedladdningar

YouTube-lägena **Ladda ned och spela upp** och **externt verktyg** kan bete
sig som om du vore inloggad — vilket behövs för åldersbegränsade,
medlemslåsta eller på annat sätt kontobundna videor, och det är vad som
låser upp fullständiga HD/4K-nedladdningar. Vivace har stöd för tre sätt
att ange cookies (alla under *Inställningar ▸ Nätverk ▸ YouTube*):
**Hämta cookies från webbläsare** på Windows/Linux/macOS, **Logga in på
YouTube…** på Android, och manuell export av en `cookies.txt`-fil som
reserv överallt.

### Hämta cookies från webbläsare (rekommenderas på Windows/Linux/macOS)

Kombinationsrutan **Hämta cookies från webbläsare:** listar Firefox,
Chrome, Edge, Brave, Chromium, Opera, Safari, Vivaldi och Whale. Välj din
webbläsare, så läser Vivace dess cookies direkt, varje gång — inget att
exportera, inget som blir inaktuellt.

**Tänk på:**

- YouTube har avsevärt förkortat sina cookies livslängd, så en tidigare
  exporterad `cookies.txt`-fil (se nedan) kan bli inaktuell inom några
  dagar — det här alternativet undviker det helt genom att läsa
  webbläsarens egen, alltid aktuella cookielagring.
- **På Windows är det bara Firefox som faktiskt fungerar här.** Chrome,
  Edge och andra Chromium-baserade webbläsare på Windows krypterar sina
  cookies på ett sätt som är kopplat till webbläsarens egen binärfil
  ("App-Bound Encryption", Chrome 127 och senare) — det här hindrar
  yt-dlp, och alla andra externa verktyg, från att läsa dem alls. Det är
  en begränsning på Chromes sida; yt-dlps egna utvecklare kan inte komma
  runt den. Chrome/Edge på Linux och macOS påverkas inte av detta och
  fungerar normalt.
- Att välja en webbläsare här har företräde framför fältet **Cookiefil:**
  nedan, när båda är angivna.
- **Inte tillgängligt på Android** — se **Logga in på YouTube** nedan i
  stället.

### Logga in på YouTube (Android, rekommenderas där)

*Inställningar ▸ Nätverk ▸ YouTube* har en knapp **Logga in på
YouTube…** (bredvid fältet för cookiefilen) som öppnar en riktig
inloggningssida inuti Vivace självt, med hjälp av en inbäddad, OS-egen
webbvy. Logga in med det konto vars åtkomst du vill använda, tryck sedan
på **Jag är inloggad** — Vivace läser de resulterande sessionscookies
och sparar dem som den aktiva cookiefilen automatiskt. Det här är
Androids motsvarighet till **Hämta cookies från webbläsare** ovan: inget
att exportera, inget att överföra från en annan enhet för hand.

**Tänk på:**

- Android har ingen motsvarighet till en aktiv skrivbordswebbläsares
  cookielagring att läsa från alls (varje apps lagring är sandboxad från
  alla andra appars), vilket är varför det här fungerar annorlunda än
  skrivbordsalternativet — att logga in *inuti* Vivaces egen inbäddade
  webbläsare är den praktiska ersättningen.
- Endast tillgängligt på Android; alla andra plattformar använder
  **Hämta cookies från webbläsare** i stället.

### Exportera cookies till en fil (reserv på alla plattformar)

Fältet **Cookiefil:** förväntar sig en vanlig textfil, `cookies.txt`, i
det klassiska Netscape-cookieformatet (samma format som yt-dlps egen
flagga `--cookies` läser) — använd det här när inget av de direkta
alternativen ovan är tillgängligt eller fungerar för dig (t.ex.
Chrome/Edge på Windows).

**Så skapar du en:**

1. Logga in på youtube.com i din vanliga webbläsare, med det konto vars
   åtkomst du vill använda.
2. Installera ett webbläsartillägg för cookie-export som skriver
   Netscape-formatet — för Chrome, Edge eller Brave, sök i webbläsarens
   tilläggsbutik efter något i stil med "Get cookies.txt"; för Firefox,
   sök efter "cookies.txt". Alla tillägg som tydligt anger att de
   exporterar det klassiska Netscape-formatet `cookies.txt` fungerar.
3. Med youtube.com öppet i en flik, använd tillägget för att exportera
   cookies för den webbplatsen och spara resultatet som en `.txt`-fil
   någonstans på disken.
4. I Vivace, öppna *Inställningar ▸ Nätverk ▸ YouTube* och använd
   **Bläddra…** bredvid **Cookiefil:** för att välja den filen.

**På Android:** föredra **Logga in på YouTube…** ovan i stället — det
kräver inget export-/överföringssteg alls. Om du ändå vill göra det på
det här sättet, observera att Chrome för Android inte stöder
webbläsartillägg, så steg 2–3 ovan kan inte göras på själva enheten.
Exportera `cookies.txt` på en
stationär eller bärbar dator enligt beskrivningen ovan, och överför sedan
filen till din Android-enhet (t.ex. via molnlagring, en USB-kabel eller
e-post) innan du använder **Bläddra…** i steg 4.

**Tänk på:**

- En `cookies.txt`-fil är i praktiken en sparad inloggningssession — vem
  som helst som har filen kan agera som ditt YouTube-konto tills
  cookies går ut eller du loggar ut. Förvara den någonstans privat och
  dela den inte.
- Cookies går ut. Om nedladdningar som tidigare fungerade börjar
  misslyckas, eller faller tillbaka till ett resultat med lägre
  kvalitet/allmän tillgång, exportera en ny `cookies.txt` — eller byt
  till **Hämta cookies från webbläsare** ovan, om tillgängligt, för att
  undvika detta helt.

**Gäller båda metoderna ovan:**

- Cookies används bara av **nedladdningsvägen** (Ladda ned och spela upp /
  externt verktyg). Vivace skickar avsiktligt aldrig cookies i
  **strömningsläge** — en inloggad ström-URL är bunden till den sessionen
  på ett sätt som Vivaces enkla videospelare inte kan öppna, så strömning
  förblir anonym oavsett vilken metod som är konfigurerad.

## Installera ffmpeg för YouTube-nedladdningar

Läget **Ladda ner och spela upp** behöver `ffmpeg` för att slå ihop de
separata video- och ljudströmmarna som yt-dlp laddar ner till en enda
spelbar fil — YouTube erbjuder sällan HD som en enda kombinerad ström, så
ett videospår och ett ljudspår laddas ner separat och slås sedan ihop.
Fältet **ffmpeg-plats:** (*Inställningar ▸ Nätverk ▸ YouTube ▸ Ladda ner
och spela upp*) talar om för yt-dlp var det ska hitta programmet; lämna
fältet tomt för att i stället använda `ffmpeg` från systemets PATH.

**Så installerar du ffmpeg:**

1. **Windows** — det enklaste alternativet är en pakethanterare:
   `winget install ffmpeg` (eller `scoop install ffmpeg` / `choco install
   ffmpeg`). Alternativt kan du hämta ett färdigbyggt arkiv från
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) eller
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) och packa
   upp det någonstans.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — installera det via din distributions pakethanterare, t.ex.
   `sudo apt install ffmpeg` (Debian/Ubuntu), `sudo dnf install ffmpeg`
   (Fedora) eller `sudo pacman -S ffmpeg` (Arch).
4. Om du har lagt till ffmpeg i systemets PATH, lämna **ffmpeg-plats:**
   tomt. Annars, klistra in sökvägen till *mappen* som innehåller
   programmet `ffmpeg` (inte själva programmet) i det fältet.
5. Starta om Vivace (eller försök bara igen med en nedladdning) efter
   installationen.

**Tänk på:**

- Det här är ett beroende till **yt-dlp**, precis som Deno nedan —
  Vivace kör bara programmet som en extern process.
- Läget **Strömning** behöver aldrig ffmpeg, eftersom det spelar upp en
  redan kombinerad ström; bara **Ladda ner och spela upp** gör det,
  eftersom det läget hämtar video och ljud separat och slår ihop dem
  lokalt.
- Om en nedladdning misslyckas med ett sammanslagningsrelaterat fel, kontrollera
  ffmpeg-platsen först — det är den vanligaste orsaken, förutom en
  saknad eller föråldrad Deno-installation.

## Installera Deno för YouTube-nedladdningar

yt-dlp självt — inte bara Vivace — använder en separat, extern
JavaScript-körningsmiljö för att lösa de utmaningar som YouTube ställer
upp innan tjänsten lämnar ut en videos riktiga nedladdnings-URL. Enligt
yt-dlps egen dokumentation är det "föråldrat" att köra utan en sådan, men
det misslyckas inte helt: formatutbudet minskar helt enkelt, och
**kraftigt för en inloggad (cookie-baserad) begäran** — precis den typen
av begäran som läget **Ladda ned och spela upp** gör för att låsa upp HD,
medlemsinnehåll och åldersbegränsade videor. Läget **Strömning** skickar
aldrig cookies (se "Cookies för YouTube-nedladdningar" ovan),
så det är inte det allvarliga fallet, och det fungerar oftast bra utan
Deno. Det är därför fältet **Deno-sökväg:** finns under *Inställningar ▸
Nätverk ▸ YouTube ▸ Ladda ned och spela upp*, och inte som en allmän
YouTube-inställning. yt-dlp stöder flera JS-körningsmiljöer; Deno är den
som det letar efter som standard.

**Så installerar du Deno:**

1. Följ de officiella installationsinstruktionerna på
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   för ditt operativsystem (ett installationsskript, eller en
   pakethanterare som winget/scoop/Homebrew/apt, beroende på plattform).
2. Se till att programmet `deno` hamnar i systemets PATH — programmen
   ovan gör vanligtvis detta åt dig. På Windows, se till att du får
   `deno`, inte `denort` (ett annat, besläktat program som inte fungerar
   här).
3. Om du hellre inte vill ändra PATH kan du låta det vara och i stället
   klistra in dess fullständiga sökväg i **Deno-sökväg:** (*Inställningar
   ▸ Nätverk ▸ YouTube ▸ Ladda ned och spela upp*).
4. Starta om Vivace (eller försök bara ladda ner igen) efter
   installationen.

**Tänk på:**

- Detta är ett beroende till **yt-dlp**, inte till Vivace direkt — Vivace
  kör bara yt-dlp som en extern process och anropar aldrig Deno själv.
- yt-dlp kräver en tillräckligt ny version av Deno (2.3.0 eller senare
  vid skrivande stund). Om nedladdningar fortfarande visar sämre
  kvalitet/formatfel efter installationen, kontrollera `deno --version`
  och uppdatera den om den är äldre.
- Det här kravet kommer från ändringar på YouTubes/yt-dlps sida, inte
  från Vivace — samma fält **Deno-sökväg:** finns av just den
  anledningen och behöver ingen ytterligare konfiguration när Deno väl är
  installerat och nåbart.

## Utjämning av bitmapundertexter

*Inställningar ▸ Undertexter ▸ Bitmapundertexter* har en inställning
**Utjämning:** (0–3, standardvärde 1) för undertexter som återges som
bilder i stället för text -- dvd-underbild-, PGS- och DVB-spår. Detta
gäller både en riktig dvd-skivas egna undertexter och ett inbäddat
undertextspår av samma typ i en vanlig videofil (t.ex. en .mp4-fil med
ett spår i `dvd_subtitle`-kodeken). Dessa format är förrenderade
bitmappsbilder, inbrända i den ursprungliga standardupplösningen (SD)
när källan skapades -- kanterna kan se taggiga ut när de skalas upp
till en modern fönsterstorlek. Vivace kan använda en lätt oskärpa för
att mjuka upp dessa kanter:

- **0** — av; visar den ursprungliga undertextbitmappen exakt som den
  skapades.
- **1** (standard) — mjukar upp de grövsta kanterna samtidigt som
  texten behåller i stort sett full ljusstyrka.
- **2** / **3** — gradvis kraftigare oskärpa.

Den här inställningen påverkar bara bitmapundertexter — den påverkar
inte Vivaces egen externa undertextrenderare (SRT/VTT/ASS) eller
vanliga textbaserade undertextspår, vilka båda använder andra
renderingsvägar.
