# Možnosti

Nabídka **Možnosti** obsahuje předvolby a konfiguraci rozhraní.

- **Předvolby…** (`Ctrl+P`) — hlavní okno nastavení: Obecné, Rozhraní, Titulky,
  Klávesnice a myš, Seznam skladeb, Jednotky, TV, Typy souborů, Aktualizace, Síť
  a Pokročilé. Změny se použijí ihned; **Zrušit** je vrátí zpět.
  - **Síť** obsahuje karty OpenSubtitles, YouTube, Proxy a Vysílání; **Proxy**
    nastavuje volitelnou HTTP nebo SOCKS5 proxy platnou pro celou aplikaci
    (vyhledávání OpenSubtitles, kontrolu aktualizací a — pouze u HTTP —
    přehrávání a yt-dlp); **Vysílání** určuje pevný port, na kterém naslouchá
    *Přehrávání ▸ Vysílat do ▸ Chytrý telefon/tablet*. Heslo účtu OpenSubtitles
    a heslo proxy jsou nyní bezpečně uložena ve správci přihlašovacích údajů
    operačního systému, nikoli ve vlastním nastavení Vivace.
- **Zobrazit ikonu v oznamovací oblasti** — udržet Vivace dostupné z oznamovací oblasti.
- **Panely nástrojů**
  - **Panel nástrojů** / **Ovládací panel** — zobrazit nebo skrýt každý panel.
  - **Upravit hlavní panel nástrojů…** / **Upravit ovládací panel…** — vybrat,
    která tlačítka se zobrazí, jejich pořadí a velikost ikon.
- **Stavový řádek**
  - **Zobrazit stavový řádek** a co zobrazuje: **Informace o videu**, **Informace
    o zvuku**, **Informace o formátu**, **Informace o datovém toku**, **Počítadlo
    snímků**, **Zobrazit celkový čas**, **Zobrazit zbývající čas** a **Zobrazit
    aktuální čas s milisekundami**.

**Tip:** celkové rozvržení (Basic / Mini / MPC) se volí v *Předvolby ▸ Rozhraní*.

## Export cookies pro stahování z YouTube

Pole **Soubor cookies:** (*Předvolby ▸ Síť ▸ YouTube*) umožňuje režimům
YouTube **Stáhnout a přehrát** a **externí nástroj** chovat se, jako byste
byli přihlášeni — to je potřeba pro videa s věkovým omezením, dostupná
pouze členům nebo jinak vázaná na účet, a právě to odemyká plnohodnotné
stahování v HD/4K. Očekává se prostý textový soubor `cookies.txt` v
klasickém formátu Netscape cookie-jar (stejném formátu, jaký čte vlastní
volba `--cookies` nástroje yt-dlp); Vivace nečte cookies přímo z profilu
prohlížeče.

**Jak jej vytvořit:**

1. Přihlaste se na youtube.com ve svém běžném prohlížeči pomocí účtu, jehož
   přístup chcete použít.
2. Nainstalujte rozšíření prohlížeče pro export cookies, které zapisuje
   formát Netscape — pro Chrome, Edge nebo Brave vyhledejte v obchodě s
   rozšířeními vašeho prohlížeče něco jako „Get cookies.txt“; pro Firefox
   vyhledejte „cookies.txt“. Funguje jakékoli rozšíření, které jasně uvádí,
   že exportuje v klasickém formátu Netscape `cookies.txt`.
3. S otevřenou kartou youtube.com použijte rozšíření k exportu cookies pro
   danou stránku a výsledek uložte někam na disk jako soubor `.txt`.
4. Ve Vivace otevřete *Předvolby ▸ Síť ▸ YouTube* a pomocí tlačítka
   **Procházet…** vedle pole **Soubor cookies:** tento soubor vyberte.

**Mějte na paměti:**

- Soubor `cookies.txt` je v podstatě uložená přihlašovací relace — kdokoli,
  kdo tento soubor má, může jednat jako váš účet YouTube, dokud cookies
  nevyprší nebo se neodhlásíte. Uchovávejte jej někde v soukromí a nesdílejte
  jej.
- Cookies se používají pouze pro cestu **stahování** (Stáhnout a přehrát /
  externí nástroj). Vivace záměrně nikdy neodesílá cookies v režimu
  **streamování** — adresa URL streamu přihlášené relace je vázána na danou
  relaci způsobem, který obyčejný video přehrávač Vivace nedokáže otevřít,
  takže streamování zůstává anonymní, i když je nakonfigurován soubor
  cookies.
- Platnost cookies vyprší. Pokud stahování, které dříve fungovalo, začne
  selhávat nebo se vrací k výsledku nižší kvality/veřejnému výsledku,
  exportujte nový soubor `cookies.txt`.
