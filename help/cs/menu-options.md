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

## Instalace a aktualizace yt-dlp

*Předvolby ▸ Síť ▸ YouTube* obsahuje zaškrtávací pole **Používat spravovaný
yt-dlp**, které určuje, jak Vivace získává a udržuje program `yt-dlp`, jenž
používá k přehrávání odkazů YouTube:

- **Zapnuto** (výchozí stav) — Vivace za vás dokáže `yt-dlp` nainstalovat a
  udržovat aktuální. Pole **Cesta k yt-dlp:** je pevně nastaveno na vlastní
  kopii Vivace a nelze je přímo upravovat; kdykoli chcete stáhnout nejnovější
  oficiální vydání, použijte tlačítko **Nainstalovat / aktualizovat yt-dlp…**
  (vedle zaškrtávacího pole). Zpřístupní se také nastavení **Automaticky
  aktualizovat yt-dlp:**, které umožňuje Vivace spouštět tuto aktualizaci
  samostatně — **Nikdy**, **Při každém spuštění yt-dlp** nebo jednou
  **denně**/**týdně**. Automatická aktualizace proběhne těsně předtím, než
  se odkaz na YouTube skutečně přeloží nebo stáhne, takže první přehrání po
  jejím vyžádání trvá o něco déle; pokud aktualizace samotná selže (např.
  kvůli nedostupné síti), Vivace potichu pokračuje s tím, co je již
  nainstalováno, místo aby blokovala přehrávání.
- **Vypnuto** — pro yt-dlp, který si spravujete sami (např. nainstalovaný
  přes `pip` nebo správce balíčků vašeho operačního systému). Pole **Cesta
  k yt-dlp:** se stane upravitelným, takže na tuto kopii můžete odkázat, a
  volba **Automaticky aktualizovat yt-dlp** je zakázána — Vivace nikdy
  neinstaluje ani neaktualizuje yt-dlp, který nespravuje sama. V tomto
  režimu je zakázáno i tlačítko **Nainstalovat / aktualizovat yt-dlp…**.

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

## Instalace Deno pro stahování z YouTube

Samotný yt-dlp — nejen Vivace — používá samostatné externí prostředí
JavaScriptu k řešení výzev, které YouTube klade dříve, než vydá skutečnou
adresu URL ke stažení videa. Podle vlastní dokumentace yt-dlp je provoz bez
něj „zastaralý“ (deprecated), ale rovnou neselže: dostupnost formátů se
pouze omezí, a to **výrazně u přihlášených požadavků (s cookies)** — což je
přesně ten druh požadavku, který provádí režim **Stáhnout a přehrát**, aby
odemkl HD, obsah jen pro členy a videa s věkovým omezením. Režim
**Streamování** nikdy neodesílá cookies (viz „Export cookies pro stahování
z YouTube“ výše), takže se ho tento vážnější případ netýká a ve většině
případů funguje bez Deno bez problémů. Proto pole **Cesta k Deno:** patří
do *Předvolby ▸ Síť ▸ YouTube ▸ Stáhnout a přehrát*, nikoli mezi obecná
nastavení YouTube. yt-dlp podporuje několik prostředí JavaScriptu; Deno je
to, které hledá jako výchozí.

**Instalace Deno:**

1. Postupujte podle oficiálních pokynů k instalaci na
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   pro váš operační systém (instalační skript nebo správce balíčků jako
   winget/scoop/Homebrew/apt, podle platformy).
2. Ujistěte se, že spustitelný soubor `deno` skončí v systémové proměnné
   PATH — výše uvedené instalátory to obvykle udělají za vás. Ve Windows
   dbejte na to, abyste získali `deno`, nikoli `denort` (jiný, související
   spustitelný soubor, který zde nefunguje).
3. Pokud raději nechcete upravovat PATH, ponechte ji beze změny a místo
   toho vložte celou cestu k souboru do pole **Cesta k Deno:** (*Předvolby
   ▸ Síť ▸ YouTube ▸ Stáhnout a přehrát*).
4. Po instalaci restartujte Vivace (nebo jen zopakujte stahování).

**Mějte na paměti:**

- Toto je závislost programu **yt-dlp**, nikoli přímo Vivace — Vivace vždy
  spouští yt-dlp pouze jako externí proces a Deno samotné nikdy nevolá.
- yt-dlp vyžaduje přiměřeně novou verzi Deno (v době psaní tohoto textu
  2.3.0 nebo novější). Pokud stahování i po instalaci stále vykazuje
  omezenou kvalitu nebo chyby formátu, zkontrolujte příkazem
  `deno --version`, jakou verzi máte, a případně ji aktualizujte.
- Tento požadavek vychází ze změn na straně YouTube/yt-dlp, nikoli z
  Vivace — právě proto existuje pole **Cesta k Deno:** a jakmile je Deno
  nainstalováno a dostupné, není potřeba žádné další nastavení.
