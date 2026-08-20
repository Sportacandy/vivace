# Alternativ

Menyn **Alternativ** innehåller inställningarna och gränssnittskonfigurationen.

- **Inställningar…** (`Ctrl+P`) — huvuddialogrutan för inställningar: Allmänt,
  Gränssnitt, Undertexter, Tangentbord och mus, Spellista, Enheter, Tv,
  Filtyper, Uppdateringar, Nätverk och Avancerat. Ändringar tillämpas direkt;
  **Avbryt** återställer dem.
  - **Allmänt ▸ Video** ställer in standardläget för **Deinterlace** (Ingen /
    Yadif / Bwdif) för nyöppnade filer — ändra det per fil under *Video ▸
    Deinterlace*.
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

## Exportera cookies för YouTube-nedladdningar

Fältet **Cookiefil:** (*Inställningar ▸ Nätverk ▸ YouTube*) låter
YouTube-lägena **Ladda ned och spela upp** och **externt verktyg** bete sig
som om du vore inloggad — vilket behövs för åldersbegränsade,
medlemslåsta eller på annat sätt kontobundna videor, och det är vad som
låser upp fullständiga HD/4K-nedladdningar. Den förväntar sig en vanlig
textfil, `cookies.txt`, i det klassiska Netscape-cookieformatet (samma
format som yt-dlps egen flagga `--cookies` läser); Vivace läser inte
cookies direkt från en webbläsares profil.

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

**Tänk på:**

- En `cookies.txt`-fil är i praktiken en sparad inloggningssession — vem
  som helst som har filen kan agera som ditt YouTube-konto tills
  cookies går ut eller du loggar ut. Förvara den någonstans privat och
  dela den inte.
- Cookies används bara av **nedladdnings**vägen (Ladda ned och spela upp /
  externt verktyg). Vivace skickar avsiktligt aldrig cookies i
  **strömnings**läge — en inloggad ström-URL är bunden till den sessionen
  på ett sätt som Vivaces enkla videospelare inte kan öppna, så strömning
  förblir anonym även om en cookiefil är konfigurerad.
- Cookies går ut. Om nedladdningar som tidigare fungerade börjar
  misslyckas, eller faller tillbaka till ett resultat med lägre
  kvalitet/allmän tillgång, exportera en ny `cookies.txt`.

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
aldrig cookies (se "Exportera cookies för YouTube-nedladdningar" ovan),
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
