# Alternativ

Menyn **Alternativ** innehåller inställningarna och gränssnittskonfigurationen.

- **Inställningar…** (`Ctrl+P`) — huvuddialogrutan för inställningar: Allmänt,
  Gränssnitt, Undertexter, Tangentbord och mus, Spellista, Enheter, Tv,
  Filtyper, Uppdateringar, Nätverk och Avancerat. Ändringar tillämpas direkt;
  **Avbryt** återställer dem.
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
