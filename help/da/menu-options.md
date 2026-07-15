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
