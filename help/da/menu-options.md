# Indstillinger

Menuen **Indstillinger** indeholder indstillingerne og grænsefladekonfigurationen.

- **Indstillinger…** (`Ctrl+P`) — hoveddialogen med indstillinger: Generelt,
  Grænseflade, Undertekster, Tastatur og mus, Afspilningsliste, Drev, Tv,
  Filtyper, Opdateringer, Netværk og Avanceret. Ændringer anvendes med det samme;
  **Annuller** fortryder dem.
  - **Generelt ▸ Video** angiver standardtilstanden for **Deinterlace**
    (Ingen / Yadif / Bwdif / Automatisk) for nyligt åbnede filer — skift den
    pr. fil under *Video ▸ Deinterlace* (kun Ingen/Yadif/Bwdif; Automatisk
    tilbydes ikke dér, da det kun giver mening som standardværdi).
    **Automatisk** bruger Bwdif, men kun på billeder, filen selv markerer
    som interlacet, og lader progressive billeder være urørte.
  - **Generelt ▸ Foretrukket lyd og undertekster** angiver de sprog,
    Vivace vælger automatisk blandt en fils indlejrede spor. **Vis
    undertekster som standard** slår automatisk en undertekst til, når en
    er tilgængelig; underindstillingen **...men ikke hvis lyden allerede er
    på et foretrukket sprog** springer denne automatiske undertekst over,
    når dens sprog matcher det lydspor, der faktisk blev valgt — nyttigt,
    hvis du allerede forstår lyden og ikke ønsker en undertekst på samme
    sprog, der distraherer.
  - **Netværk** har fanerne OpenSubtitles, YouTube, Proxy og Udsendelse;
    **Proxy** konfigurerer en valgfri HTTP- eller SOCKS5-proxy, der gælder
    for hele programmet (OpenSubtitles-søgning, opdateringstjek og — kun for
    HTTP — medieafspilning og yt-dlp); **Udsendelse** fastlåser den port,
    *Afspil ▸ Udsend til ▸ Smartphone/tablet* lytter på. Adgangskoden til
    OpenSubtitles-kontoen og proxy-adgangskoden gemmes sikkert i
    operativsystemets loginoplysningshåndtering, ikke i Vivaces egne
    indstillinger.
- **Vis ikon i systembakken** — hold Vivace tilgængelig fra systembakken.
- **Værktøjslinjer**
  - **Værktøjslinje** / **Kontrollinje** — vis eller skjul hver linje.
  - **Rediger hovedværktøjslinje…** / **Rediger kontrollinje…** — vælg, hvilke
    knapper der vises, samt deres rækkefølge og ikonstørrelse.
- **Statuslinje**
  - **Vis statuslinje** og hvad den viser: **Videoinfo**, **Lydinfo**,
    **Formatinfo**, **Bithastighedsinfo**, **Billedtæller**, **Vis samlet tid**,
    **Vis resterende tid** og **Vis det aktuelle klokkeslæt med millisekunder**.

**Tip:** det overordnede layout (Basic / Mini / MPC) vælges i *Indstillinger ▸ Grænseflade*.

## Installation og opdatering af yt-dlp

*Indstillinger ▸ Netværk ▸ YouTube* har afkrydsningsfeltet **Brug
administreret yt-dlp**, som styrer, hvordan Vivace henter og
vedligeholder programmet `yt-dlp`, som det bruger til at afspille
YouTube-links:

- **Til** (standard) — Vivace kan installere `yt-dlp` for dig og holde
  det opdateret. Feltet **yt-dlp-sti:** er låst til Vivaces egen kopi og
  kan ikke redigeres direkte; brug knappen **Installer/opdater
  yt-dlp…** (ved siden af afkrydsningsfeltet), når du vil hente den
  nyeste officielle udgivelse. Indstillingen **Opdater yt-dlp
  automatisk:** bliver også tilgængelig, så Vivace selv kan udføre denne
  opdatering: **Aldrig**, **Hver gang yt-dlp køres**, **En gang om dagen**
  eller **En gang om ugen**. En automatisk opdatering køres, lige inden
  en YouTube-URL rent faktisk slås op eller downloades, så den første
  afspilning, efter den forfalder, tager lidt længere tid; hvis selve
  opdateringen mislykkes (f.eks. ingen netværksforbindelse), fortsætter
  Vivace stille og roligt med den version, der allerede er installeret,
  i stedet for at blokere afspilningen.
- **Fra** — til en yt-dlp, du selv administrerer (f.eks. installeret via
  `pip` eller dit styresystems pakkehåndtering). Feltet **yt-dlp-sti:**
  bliver redigerbart, så du kan pege det på den kopi, og **Opdater
  yt-dlp automatisk** deaktiveres — Vivace installerer eller opdaterer
  aldrig en yt-dlp, den ikke administrerer. Knappen **Installer/opdater
  yt-dlp…** er også deaktiveret i denne tilstand.

## Installation af YouTube PO-token-udbyderen

Nyere YouTube-videoer kræver i stigende grad en **PO-token
(proof-of-origin)** bare for overhovedet at kunne afspilles — et generelt
krav til afspilbarhed, uafhængigt af cookies eller om man er logget ind.
Uden en sådan rapporterer yt-dlp videoen som utilgængelig, selv ved en
almindelig, anonym forespørgsel. Dette gælder for **både** tilstandene
Streaming og Download og afspil, i modsætning til cookies (kun til
download, se nedenfor) eller Deno (primært en sag for Download og afspil,
se længere nede).

*Indstillinger ▸ Netværk ▸ YouTube* har knappen **Installér
PO-token-udbyder…**, som sætter communityprojektet **"BgUtils POT
Provider"** op: et lille yt-dlp-plugin plus et script — der køres efter
behov via Deno, det samme program, der dækkes længere nede — som lader
yt-dlp generere en token automatisk. Klik på den, vent på, at den korte
download- og byggeproces er færdig, og så er det klaret; knappens
etiket ændres til **Geninstallér/opdatér PO-token-udbyder…**, når den
først er installeret, i tilfælde af at en senere opdatering nogensinde
bliver nødvendig.

**Husk:**

- Dette kræver en fungerende Deno-installation (se "Installation af Deno
  til YouTube-downloads" nedenfor) — knappen downloader og bygger
  udbyderens eget script, som yt-dlp derefter kører via Deno, når en
  video rent faktisk har brug for en token.
- Ikke alle videoer har brug for en PO-token, så afspilning kan fungere
  fint uden dette installeret — men hvis en video rapporterer sig selv
  som utilgængelig, når den ellers spiller fint andre steder, er det
  værd at installere dette.
- **På Android** følger dette med Vivace og sættes op automatisk — der
  er intet at installere og ingen tilsvarende knap; det virker bare.

## Cookies til YouTube-downloads

YouTube-tilstandene **Download og afspil** og **eksternt værktøj** kan
opføre sig, som om du var logget ind — nødvendigt for aldersbegrænsede,
medlemslåste eller på anden måde kontobundne videoer, og det er det, der
låser op for fulde HD/4K-downloads. Vivace understøtter tre måder at
angive cookies på (alle under *Indstillinger ▸ Netværk ▸ YouTube*):
**Hent cookies fra browser** på Windows/Linux/macOS, **Log ind på
YouTube…** på Android, og manuel eksport af en `cookies.txt`-fil som
reserveløsning overalt.

### Hent cookies fra browser (anbefales på Windows/Linux/macOS)

Kombinationsfeltet **Hent cookies fra browser:** viser Firefox, Chrome,
Edge, Brave, Chromium, Opera, Safari, Vivaldi og Whale. Vælg din browser,
og Vivace læser dens cookies direkte, hver gang — intet at eksportere,
intet der bliver forældet.

**Husk:**

- YouTube har afkortet sine cookies' levetid betydeligt, så en tidligere
  eksporteret `cookies.txt`-fil (se nedenfor) kan blive forældet inden for
  få dage — denne mulighed undgår det helt ved at læse browserens egen,
  altid opdaterede cookielager.
- **På Windows virker kun Firefox reelt her.** Chrome, Edge og andre
  Chromium-baserede browsere på Windows krypterer deres cookies på en
  måde, der er bundet til browserens egen binærfil ("App-Bound
  Encryption", Chrome 127+) — dette forhindrer yt-dlp, og ethvert andet
  eksternt værktøj, i overhovedet at læse dem. Det er en begrænsning på
  Chromes side; yt-dlps egne udviklere kan ikke omgå den. Chrome/Edge på
  Linux og macOS er ikke berørt af dette og fungerer normalt.
- At vælge en browser her har forrang frem for feltet **Cookiefil:**
  nedenfor, når begge er angivet.
- **Ikke tilgængeligt på Android** — se **Log ind på YouTube** nedenfor
  i stedet.

### Log ind på YouTube (Android, anbefales der)

*Indstillinger ▸ Netværk ▸ YouTube* har en knap **Log ind på
YouTube…** (ved siden af feltet til cookiefilen), som åbner en rigtig
loginside inde i Vivace selv, via en indlejret, OS-native WebView. Log
ind med den konto, hvis adgang du vil bruge, og tryk derefter på **Jeg
er logget ind** — Vivace læser de resulterende sessionscookies og
gemmer dem automatisk som den aktive cookiefil. Dette er Androids
modstykke til **Hent cookies fra browser** ovenfor: intet at
eksportere, intet at overføre fra en anden enhed manuelt.

**Husk:**

- Android har intet modstykke til et aktivt skrivebords-browsers
  cookielager at læse fra overhovedet (hver apps lager er sandboxet fra
  alle andre appers), hvilket er grunden til, at dette fungerer
  anderledes end skrivebordsmuligheden — at logge ind *inde i* Vivaces
  egen indlejrede browser er den praktiske erstatning.
- Kun tilgængeligt på Android; alle andre platforme bruger **Hent
  cookies fra browser** i stedet.

### Eksport af cookies til en fil (reserveløsning på alle platforme)

Feltet **Cookiefil:** forventer en almindelig tekstfil, `cookies.txt`, i
det klassiske Netscape cookie-format (samme format som yt-dlps eget flag
`--cookies` læser) — brug dette, når ingen af de direkte muligheder
ovenfor er tilgængelige eller virker for dig (f.eks. Chrome/Edge på
Windows).

**Sådan opretter du en:**

1. Log ind på youtube.com i din daglige browser med den konto, hvis
   adgang du vil bruge.
2. Installer en browserudvidelse til cookie-eksport, der skriver
   Netscape-formatet — for Chrome, Edge eller Brave, søg i browserens
   udvidelsesbutik efter noget i stil med "Get cookies.txt"; for
   Firefox, søg efter "cookies.txt". Enhver udvidelse, der tydeligt
   angiver, at den eksporterer det klassiske Netscape-format
   `cookies.txt`, vil virke.
3. Med youtube.com åben i en fane, brug udvidelsen til at eksportere
   cookies for det pågældende site, og gem resultatet som en `.txt`-fil
   et sted på disken.
4. Åbn i Vivace *Indstillinger ▸ Netværk ▸ YouTube*, og brug
   **Gennemse…** ved siden af **Cookiefil:** til at vælge den fil.

**På Android:** foretræk **Log ind på YouTube…** ovenfor i stedet — det
kræver intet eksport-/overførselstrin overhovedet. Hvis du stadig vil
gøre det på denne måde, bemærk, at Chrome til Android ikke understøtter
browserudvidelser, så trin 2-3 ovenfor kan ikke udføres på selve
enheden. Eksportér
`cookies.txt` på en stationær eller bærbar computer som beskrevet ovenfor,
og overfør derefter filen til din Android-enhed (f.eks. via cloud-lagring,
et USB-kabel eller e-mail), før du bruger **Gennemse…** i trin 4.

**Husk:**

- En `cookies.txt`-fil er i praksis en gemt loginsession — alle, der har
  filen, kan agere som din YouTube-konto, indtil cookies udløber, eller
  du logger ud. Opbevar den et privat sted, og del den ikke.
- Cookies udløber. Hvis downloads, der tidligere virkede, begynder at
  fejle eller falder tilbage til et resultat af lavere kvalitet/et
  offentligt resultat, så eksportér en ny `cookies.txt` — eller skift til
  **Hent cookies fra browser** ovenfor, hvis tilgængeligt, for at undgå
  dette helt.

**Gælder for begge metoder ovenfor:**

- Cookies bruges kun af **download**-stien (Download og afspil /
  eksternt værktøj). Vivace sender bevidst aldrig cookies i
  **streaming**-tilstand — en logget-ind stream-URL er bundet til den
  session på en måde, Vivaces enkle videoafspiller ikke kan åbne, så
  streaming forbliver anonym, uanset hvilken metode der er konfigureret.

## Installation af ffmpeg til YouTube-downloads

Tilstanden **Download og afspil** har brug for `ffmpeg` til at flette de
separate video- og lydstrømme, som yt-dlp downloader, sammen til én
afspilbar fil — YouTube tilbyder sjældent HD som en enkelt kombineret
strøm, så et videospor og et lydspor downloades separat og flettes
derefter sammen. Feltet **ffmpeg-placering:** (*Indstillinger ▸ Netværk ▸
YouTube ▸ Download og afspil*) fortæller yt-dlp, hvor det skal finde
programmet; lad feltet stå tomt for i stedet at bruge `ffmpeg` fra
systemets PATH.

**Sådan installerer du ffmpeg:**

1. **Windows** — den nemmeste mulighed er en pakkehåndtering:
   `winget install ffmpeg` (eller `scoop install ffmpeg` / `choco install
   ffmpeg`). Alternativt kan du downloade et prækompileret arkiv fra
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) eller
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) og pakke
   det ud et sted.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — installer det via din distributions pakkehåndtering, f.eks.
   `sudo apt install ffmpeg` (Debian/Ubuntu), `sudo dnf install ffmpeg`
   (Fedora) eller `sudo pacman -S ffmpeg` (Arch).
4. Hvis du har føjet ffmpeg til systemets PATH, så lad
   **ffmpeg-placering:** stå tomt. Ellers skal du indsætte stien til den
   *mappe*, der indeholder programmet `ffmpeg` (ikke selve programmet), i
   det felt.
5. Genstart Vivace (eller prøv blot en download igen) efter
   installationen.

**Husk:**

- Dette er en afhængighed for **yt-dlp**, ligesom Deno nedenfor — Vivace
  kører kun programmet som en ekstern proces.
- Tilstanden **Streaming** har aldrig brug for ffmpeg, da den afspiller
  en allerede kombineret strøm; det gør kun **Download og afspil**,
  fordi den tilstand henter video og lyd separat og fletter dem sammen
  lokalt.
- Hvis en download fejler med en flettningsrelateret fejl, så tjek
  ffmpeg-placeringen først — det er den mest almindelige årsag, ud over
  en manglende eller forældet Deno.

## Installation af Deno til YouTube-downloads

yt-dlp selv — ikke kun Vivace — bruger en separat, ekstern
JavaScript-motor til at løse de udfordringer, YouTube stiller, før
tjenesten udleverer en videos rigtige download-URL. Ifølge yt-dlps egen
dokumentation er det "forældet" at køre uden en sådan, men det slår ikke
ligefrem fejl: formatudvalget bliver blot reduceret, og **kraftigt for
en logget ind (cookie-baseret) forespørgsel** — netop den slags
forespørgsel, som tilstanden **Download og afspil** bruger for at låse
op for HD, medlemsindhold og aldersbegrænsede videoer. Tilstanden
**Streaming** sender aldrig cookies (se "Cookies til
YouTube-downloads" ovenfor), så det er ikke det alvorlige tilfælde, og
det fungerer som regel fint uden Deno. Det er derfor, feltet **Deno-sti:**
findes under *Indstillinger ▸ Netværk ▸ YouTube ▸ Download og afspil* og
ikke som en generel YouTube-indstilling. yt-dlp understøtter flere
JS-motorer; Deno er den, den leder efter som standard.

**Sådan installerer du Deno:**

1. Følg den officielle installationsvejledning på
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   for dit styresystem (et installationsscript eller en
   pakkehåndtering som winget/scoop/Homebrew/apt, afhængigt af
   platformen).
2. Sørg for, at programmet `deno` havner i systemets PATH — ovenstående
   installationsprogrammer gør normalt dette for dig. Sørg på Windows
   for at få `deno`, ikke `denort` (et andet, beslægtet program, der
   ikke virker her).
3. Hvis du hellere ikke vil ændre PATH, kan du lade det være og i
   stedet indsætte den fulde sti i **Deno-sti:** (*Indstillinger ▸
   Netværk ▸ YouTube ▸ Download og afspil*).
4. Genstart Vivace (eller prøv blot en download igen) efter
   installationen.

**Husk:**

- Dette er en afhængighed for **yt-dlp**, ikke for Vivace direkte —
  Vivace kører kun yt-dlp som en ekstern proces og kalder aldrig selv
  Deno.
- yt-dlp kræver en rimeligt ny version af Deno (2.3.0 eller nyere på
  skrivetidspunktet). Hvis downloads stadig viser reduceret
  kvalitet/formatfejl efter installationen, så tjek `deno --version`,
  og opdater den, hvis den er ældre.
- Dette krav skyldes ændringer på YouTubes/yt-dlps side, ikke Vivace —
  det samme felt, **Deno-sti:**, findes af netop den grund og kræver
  ingen yderligere konfiguration, når Deno selv er installeret og
  tilgængeligt.

## Udjævning af bitmap-undertekster

*Indstillinger ▸ Undertekster ▸ Bitmap-undertekster* har en
indstilling **Udjævning:** (0–3, standard 1) til undertekster, der
gengives som billeder i stedet for tekst -- dvd-underbillede-, PGS- og
DVB-spor. Dette gælder både en rigtig dvd-disks egne undertekster og et
integreret undertekstspor af samme type i en almindelig videofil (f.eks.
en .mp4-fil med et spor i `dvd_subtitle`-kodeket). Disse formater er
forudrenderede bitmap-billeder, der er brændt i den oprindelige
standardopløsning (SD), da kilden blev fremstillet -- deres kanter kan
se takkede ud, når de skaleres op til en moderne vinduesstørrelse.
Vivace kan anvende en let sløring for at udjævne disse kanter:

- **0** — fra; viser den oprindelige undertekst-bitmap præcis som den
  blev fremstillet.
- **1** (standard) — udjævner de groveste kanter, mens teksten bevarer
  stort set fuld lysstyrke.
- **2** / **3** — gradvist kraftigere sløring.

Denne indstilling påvirker kun bitmap-undertekster — den har ingen
indflydelse på Vivaces egen eksterne undertekstgengivelse (SRT/VTT/ASS)
eller på almindelige tekstbaserede undertekstspor, som begge bruger
andre gengivelsesveje.
