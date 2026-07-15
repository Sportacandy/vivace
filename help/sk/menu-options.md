# Možnosti

Ponuka **Možnosti** obsahuje nastavenia a konfiguráciu rozhrania.

- **Nastavenia…** (`Ctrl+P`) — hlavné okno nastavení: Všeobecné, Rozhranie,
  Titulky, Klávesnica a myš, Zoznam skladieb, Jednotky, TV, Typy súborov,
  Aktualizácie, Sieť a Pokročilé. Zmeny sa použijú ihneď; **Zrušiť** ich vráti späť.
  - **Sieť** obsahuje karty OpenSubtitles, YouTube, Proxy a Prenos; **Proxy**
    nastavuje voliteľnú HTTP alebo SOCKS5 proxy platnú pre celú aplikáciu
    (vyhľadávanie OpenSubtitles, kontrolu aktualizácií a — iba pri HTTP —
    prehrávanie a yt-dlp); **Prenos** určuje pevný port, na ktorom počúva
    *Prehrávanie ▸ Prenášať do ▸ Smartfón/tablet*. Heslo účtu OpenSubtitles
    a heslo proxy sú teraz bezpečne uložené v správcovi poverení operačného
    systému, nie vo vlastných nastaveniach Vivace.
- **Zobraziť ikonu v oblasti oznámení** — udržať Vivace dostupné z oblasti oznámení.
- **Panely nástrojov**
  - **Panel nástrojov** / **Ovládací panel** — zobraziť alebo skryť každý panel.
  - **Upraviť hlavný panel nástrojov…** / **Upraviť ovládací panel…** — vybrať,
    ktoré tlačidlá sa zobrazia, ich poradie a veľkosť ikon.
- **Stavový riadok**
  - **Zobraziť stavový riadok** a čo zobrazuje: **Informácie o videu**,
    **Informácie o zvuku**, **Informácie o formáte**, **Informácie o dátovom
    toku**, **Počítadlo snímok**, **Zobraziť celkový čas**, **Zobraziť zostávajúci
    čas** a **Zobraziť aktuálny čas s milisekundami**.

**Tip:** celkové rozloženie (Basic / Mini / MPC) sa volí v *Nastavenia ▸ Rozhranie*.

## Export cookies pre sťahovanie z YouTube

Pole **Súbor cookies:** (*Nastavenia ▸ Sieť ▸ YouTube*) umožňuje režimom
YouTube **Stiahnuť a prehrať** a **externý nástroj** správať sa, akoby ste
boli prihlásení — to je potrebné pre videá s vekovým obmedzením, dostupné
iba členom alebo inak viazané na účet, a práve to odomyká plnohodnotné
sťahovanie v HD/4K. Očakáva sa obyčajný textový súbor `cookies.txt` v
klasickom formáte Netscape cookie-jar (rovnakom formáte, aký číta vlastná
voľba `--cookies` nástroja yt-dlp); Vivace nečíta cookies priamo z profilu
prehliadača.

**Ako ho vytvoriť:**

1. Prihláste sa na youtube.com vo svojom bežnom prehliadači pomocou účtu,
   ktorého prístup chcete použiť.
2. Nainštalujte rozšírenie prehliadača na export cookies, ktoré zapisuje
   formát Netscape — pre Chrome, Edge alebo Brave vyhľadajte v obchode s
   rozšíreniami vášho prehliadača niečo ako „Get cookies.txt“; pre Firefox
   vyhľadajte „cookies.txt“. Funguje akékoľvek rozšírenie, ktoré jasne
   uvádza, že exportuje v klasickom formáte Netscape `cookies.txt`.
3. S otvorenou kartou youtube.com použite rozšírenie na export cookies pre
   danú stránku a výsledok uložte niekam na disk ako súbor `.txt`.
4. Vo Vivace otvorte *Nastavenia ▸ Sieť ▸ YouTube* a pomocou tlačidla
   **Prehľadávať…** vedľa poľa **Súbor cookies:** tento súbor vyberte.

**Majte na pamäti:**

- Súbor `cookies.txt` je v podstate uložená prihlasovacia relácia —
  ktokoľvek, kto tento súbor má, môže konať ako váš účet YouTube, kým
  cookies nevypršia alebo sa neodhlásite. Uchovávajte ho niekde v súkromí a
  nezdieľajte ho.
- Cookies sa používajú iba pre cestu **sťahovania** (Stiahnuť a prehrať /
  externý nástroj). Vivace zámerne nikdy neposiela cookies v režime
  **streamovania** — adresa URL streamu prihlásenej relácie je viazaná na
  danú reláciu spôsobom, ktorý obyčajný video prehrávač Vivace nedokáže
  otvoriť, takže streamovanie zostáva anonymné, aj keď je nakonfigurovaný
  súbor cookies.
- Platnosť cookies vyprší. Ak sťahovanie, ktoré predtým fungovalo, začne
  zlyhávať alebo sa vracia k výsledku nižšej kvality/verejnému výsledku,
  exportujte nový súbor `cookies.txt`.
