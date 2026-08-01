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

## Inštalácia a aktualizácia yt-dlp

*Nastavenia ▸ Sieť ▸ YouTube* obsahuje začiarkavacie políčko **Používať
spravovaný yt-dlp**, ktoré určuje, ako Vivace získava a udržiava program
`yt-dlp`, ktorý používa na prehrávanie odkazov YouTube:

- **Zapnuté** (predvolené) — Vivace vám dokáže `yt-dlp` nainštalovať a
  udržiavať ho aktuálny. Pole **Cesta k yt-dlp:** je pevne nastavené na
  vlastnú kópiu Vivace a nedá sa upravovať priamo; kedykoľvek chcete
  stiahnuť najnovšie oficiálne vydanie, použite tlačidlo **Nainštalovať /
  aktualizovať yt-dlp…** (vedľa začiarkavacieho políčka). Sprístupní sa aj
  nastavenie **Automaticky aktualizovať yt-dlp:**, ktoré necháva Vivace
  vykonávať túto aktualizáciu samostatne — **Nikdy**, **Pri každom
  spustení yt-dlp**, alebo **Raz denne** či **Raz týždenne**. Automatická
  aktualizácia prebieha tesne predtým, než sa adresa URL z YouTube
  skutočne rozpozná alebo stiahne, takže prvé prehratie po tom, čo je na
  rade, trvá o niečo dlhšie; ak samotná aktualizácia zlyhá (napríklad pri
  výpadku siete), Vivace ticho pokračuje s už nainštalovanou verziou
  namiesto toho, aby prehrávanie zablokovala.
- **Vypnuté** — pre yt-dlp, ktorý si spravujete sami (napríklad
  nainštalovaný cez `pip` alebo správcu balíkov vášho operačného systému).
  Pole **Cesta k yt-dlp:** sa stane upraviteľným, takže ho môžete
  nasmerovať na túto kópiu, a **Automaticky aktualizovať yt-dlp** je
  zablokované — Vivace nikdy neinštaluje ani neaktualizuje yt-dlp, ktorý
  nespravuje. V tomto režime je zablokované aj tlačidlo **Nainštalovať /
  aktualizovať yt-dlp…**.

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

## Inštalácia ffmpeg pre sťahovanie z YouTube

Režim **Stiahnuť a prehrať** potrebuje `ffmpeg` na zlúčenie oddelených
video- a audio streamov, ktoré sťahuje yt-dlp, do jedného prehrateľného
súboru — YouTube len zriedka ponúka HD ako jeden spojený stream, takže sa
video stopa a audio stopa sťahujú samostatne a potom sa zlúčia. Pole
**Umiestnenie ffmpeg:** (*Nastavenia ▸ Sieť ▸ YouTube ▸ Stiahnuť a
prehrať*) hovorí yt-dlp, kde ho nájsť; ponechajte ho prázdne, ak chcete
namiesto toho použiť `ffmpeg` zo systémovej premennej PATH.

**Ako nainštalovať ffmpeg:**

1. **Windows** — najjednoduchšou možnosťou je správca balíkov:
   `winget install ffmpeg` (alebo `scoop install ffmpeg` / `choco install
   ffmpeg`). Prípadne si stiahnite predpripravený archív z
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) alebo
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) a niekam ho
   rozbaľte.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — nainštalujte ho zo správcu balíkov svojej distribúcie, napr.
   `sudo apt install ffmpeg` (Debian/Ubuntu), `sudo dnf install ffmpeg`
   (Fedora) alebo `sudo pacman -S ffmpeg` (Arch).
4. Ak ste ffmpeg pridali do systémovej premennej PATH, ponechajte pole
   **Umiestnenie ffmpeg:** prázdne. Inak do tohto poľa vložte cestu k
   *priečinku* obsahujúcemu spustiteľný súbor `ffmpeg` (nie samotný
   spustiteľný súbor).
5. Po inštalácii reštartujte Vivace (alebo jednoducho skúste sťahovanie
   znova).

**Majte na pamäti:**

- Ide o závislosť nástroja **yt-dlp**, rovnako ako Deno nižšie — Vivace ho
  vždy iba spúšťa ako externý proces.
- Režim **Streamovanie** nikdy nepotrebuje ffmpeg, pretože prehráva jeden už
  spojený stream; potrebuje ho iba **Stiahnuť a prehrať**, pretože tento
  režim sťahuje video a audio samostatne a zlučuje ich lokálne.
- Ak sťahovanie zlyhá s chybou súvisiacou so zlučovaním, skontrolujte
  najprv umiestnenie ffmpeg — to je najčastejšia príčina, hneď po
  chýbajúcom alebo zastaranom Deno.

## Inštalácia Deno pre sťahovanie z YouTube

Samotný yt-dlp — nielen Vivace — používa samostatné externé prostredie
JavaScript na vyriešenie výziev, ktoré YouTube kladie predtým, než vydá
skutočnú adresu URL na stiahnutie videa. Podľa vlastnej dokumentácie
yt-dlp je spustenie bez neho „zastarané“ (deprecated), no priamo
nezlyhá: dostupnosť formátov sa jednoducho zníži, a to **výrazne pri
prihlásenej (cookie) požiadavke** — presne o taký typ požiadavky ide v
režime **Stiahnuť a prehrať**, ktorý odomyká HD, videá iba pre členov a
videá s vekovým obmedzením. Režim **Streamovanie** nikdy neposiela
cookies (pozri „Export cookies pre sťahovanie z YouTube“ vyššie), takže
nejde o tento závažný prípad a bez Deno funguje vo väčšine prípadov
bez problémov. Preto sa pole **Cesta k Deno:** nachádza v *Nastavenia ▸
Sieť ▸ YouTube ▸ Stiahnuť a prehrať*, nie ako všeobecné nastavenie
YouTube. yt-dlp podporuje viacero JS prostredí; Deno je to, ktoré
vyhľadáva predvolene.

**Ako nainštalovať Deno:**

1. Postupujte podľa oficiálnych inštalačných pokynov na stránke
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   pre váš operačný systém (inštalačný skript, alebo správca balíkov ako
   winget/scoop/Homebrew/apt, podľa platformy).
2. Uistite sa, že spustiteľný súbor `deno` skončí v systémovej premennej
   PATH — vyššie uvedené inštalátory to zvyčajne urobia za vás. Vo
   Windows dbajte na to, aby ste získali `deno`, nie `denort` (iný,
   príbuzný spustiteľný súbor, ktorý tu nebude fungovať).
3. Ak radšej nechcete upravovať PATH, ponechajte ho tak, ako je, a
   namiesto toho vložte jeho úplnú cestu do poľa **Cesta k Deno:**
   (*Nastavenia ▸ Sieť ▸ YouTube ▸ Stiahnuť a prehrať*).
4. Po inštalácii reštartujte Vivace (alebo jednoducho skúste sťahovanie
   znova).

**Majte na pamäti:**

- Ide o závislosť nástroja **yt-dlp**, nie priamo Vivace — Vivace vždy
  iba spúšťa yt-dlp ako externý proces a samotné Deno nikdy nevolá.
- yt-dlp vyžaduje primerane novú verziu Deno (v čase písania tohto textu
  2.3.0 alebo novšiu). Ak sa aj po inštalácii pri sťahovaní stále
  zobrazujú chyby zníženej kvality/formátu, skontrolujte `deno --version`
  a v prípade staršej verzie ju aktualizujte.
- Táto požiadavka vyplýva zo zmien na strane YouTube/yt-dlp, nie od
  Vivace — presne z tohto dôvodu existuje pole **Cesta k Deno:**, ktoré
  po nainštalovaní a sprístupnení samotného Deno už nevyžaduje žiadnu
  ďalšiu konfiguráciu.
