# Indstillinger

Menuen **Indstillinger** indeholder indstillingerne og grænsefladekonfigurationen.

- **Indstillinger…** (`Ctrl+P`) — hoveddialogen med indstillinger: Generelt,
  Grænseflade, Undertekster, Tastatur og mus, Afspilningsliste, Drev, Tv,
  Filtyper, Opdateringer, Netværk og Avanceret. Ændringer anvendes med det samme;
  **Annuller** fortryder dem.
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

## Eksport af cookies til YouTube-downloads

Feltet **Cookiefil:** (*Indstillinger ▸ Netværk ▸ YouTube*) gør, at
YouTube-tilstandene **Download og afspil** og **eksternt værktøj** kan
opføre sig, som om du var logget ind — nødvendigt for aldersbegrænsede,
medlemslåste eller på anden måde kontobundne videoer, og det er det, der
låser op for fulde HD/4K-downloads. Det forventer en almindelig
tekstfil, `cookies.txt`, i det klassiske Netscape cookie-format (samme
format som yt-dlps eget flag `--cookies` læser); Vivace læser ikke
cookies direkte fra en browserprofil.

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

**Husk:**

- En `cookies.txt`-fil er i praksis en gemt loginsession — alle, der har
  filen, kan agere som din YouTube-konto, indtil cookies udløber, eller
  du logger ud. Opbevar den et privat sted, og del den ikke.
- Cookies bruges kun af **download**-stien (Download og afspil /
  eksternt værktøj). Vivace sender bevidst aldrig cookies i
  **streaming**-tilstand — en logget-ind stream-URL er bundet til den
  session på en måde, Vivaces enkle videoafspiller ikke kan åbne, så
  streaming forbliver anonym, selv hvis en cookiefil er konfigureret.
- Cookies udløber. Hvis downloads, der tidligere virkede, begynder at
  fejle eller falder tilbage til et resultat af lavere kvalitet/et
  offentligt resultat, så eksportér en ny `cookies.txt`.

## Installation af Deno til YouTube-downloads

yt-dlp selv — ikke kun Vivace — bruger en separat, ekstern
JavaScript-motor til at løse de udfordringer, YouTube stiller, før
tjenesten udleverer en videos rigtige download-URL. Ifølge yt-dlps egen
dokumentation er det "forældet" at køre uden en sådan, men det slår ikke
ligefrem fejl: formatudvalget bliver blot reduceret, og **kraftigt for
en logget ind (cookie-baseret) forespørgsel** — netop den slags
forespørgsel, som tilstanden **Download og afspil** bruger for at låse
op for HD, medlemsindhold og aldersbegrænsede videoer. Tilstanden
**Streaming** sender aldrig cookies (se "Eksport af cookies til
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
