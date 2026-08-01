# Opcije

Izbornik **Opcije** sadrži postavke i konfiguraciju sučelja.

- **Postavke…** (`Ctrl+P`) — glavni dijalog postavki: Općenito, Sučelje, Titlovi,
  Tipkovnica i miš, Popis za reprodukciju, Pogoni, TV, Vrste datoteka, Ažuriranja,
  Mreža i Napredno. Promjene se primjenjuju odmah; **Odustani** ih vraća.
  - **Mreža** sadrži kartice OpenSubtitles, YouTube, Proxy i Emitiranje;
    **Proxy** postavlja neobavezni HTTP ili SOCKS5 proxy koji vrijedi za
    cijelu aplikaciju (pretraga OpenSubtitles, provjera ažuriranja i — samo
    za HTTP — reprodukcija i yt-dlp); **Emitiranje** fiksira ulaz na kojem
    osluškuje *Reprodukcija ▸ Emitiraj na ▸ Pametni telefon/Tablet*.
    Lozinka OpenSubtitles računa i lozinka proxyja sigurno se pohranjuju u
    upravitelju vjerodajnicama vašeg operacijskog sustava, a ne u vlastitim
    postavkama Vivacea.
- **Prikaži ikonu u traci sustava** — održavanje Vivacea dostupnim iz trake sustava.
- **Alatne trake**
  - **Alatna traka** / **Kontrolna traka** — prikaz ili skrivanje svake trake.
  - **Uredi glavnu alatnu traku…** / **Uredi kontrolnu traku…** — odabir gumba
    koji se prikazuju te njihovog redoslijeda i veličine ikona.
- **Statusna traka**
  - **Prikaži statusnu traku** i ono što prikazuje: **Informacije o videu**,
    **Informacije o zvuku**, **Informacije o formatu**, **Informacije o brzini
    prijenosa**, **Brojač sličica**, **Prikaži ukupno vrijeme**, **Prikaži
    preostalo vrijeme** i **Prikaži trenutačno vrijeme s milisekundama**.

**Savjet:** ukupni raspored (Basic / Mini / MPC) bira se u *Postavke ▸ Sučelje*.

## Instalacija i ažuriranje yt-dlp-a

*Postavke ▸ Mreža ▸ YouTube* ima potvrdni okvir **Koristi upravljani yt-dlp** koji
određuje kako Vivace nabavlja i održava program `yt-dlp` koji koristi za
reprodukciju YouTube poveznica:

- **Uključeno** (zadano) — Vivace može instalirati `yt-dlp` umjesto vas i
  održavati ga ažurnim. Polje **Putanja yt-dlp:** fiksirano je na vlastitu
  kopiju Vivacea i ne može se izravno uređivati; koristite gumb
  **Instaliraj / ažuriraj yt-dlp…** (pored potvrdnog okvira) kad god želite
  dohvatiti najnovije službeno izdanje. Postaje dostupna i postavka
  **Automatski ažuriraj yt-dlp:**, koja omogućuje Vivaceu da samostalno
  pokreće ovo ažuriranje — **Nikada**, prije **Svaki put kad se yt-dlp
  pokrene**, ili jednom **dnevno**/**tjedno**. Automatsko ažuriranje
  pokreće se neposredno prije nego što se YouTube URL zapravo razriješi ili
  preuzme, pa prva reprodukcija nakon što ažuriranje dospije potraje malo
  dulje; ako samo ažuriranje ne uspije (npr. nema mreže), Vivace tiho
  nastavlja s verzijom koja je već instalirana, umjesto da blokira
  reprodukciju.
- **Isključeno** — za yt-dlp kojim upravljate sami (npr. instaliran putem
  `pip`-a ili upravitelja paketima vašeg operacijskog sustava). Polje
  **Putanja yt-dlp:** postaje uredivo kako biste ga mogli usmjeriti na tu
  kopiju, a **Automatski ažuriraj yt-dlp** je onemogućeno — Vivace nikada
  ne instalira niti ažurira yt-dlp kojim ne upravlja. Gumb **Instaliraj /
  ažuriraj yt-dlp…** također je onemogućen u ovom načinu rada.

## Izvoz kolačića za YouTube preuzimanja

Polje **Datoteka kolačića:** (*Postavke ▸ Mreža ▸ YouTube*) omogućuje da
se YouTube načini rada **Preuzmi i reproduciraj** i **vanjski alat**
ponašaju kao da ste prijavljeni — potrebno za videozapise s dobnim
ograničenjem, samo za članove ili na drugi način vezane uz račun, a to
je i ono što otključava potpuna HD/4K preuzimanja. Očekuje se obična
tekstna datoteka `cookies.txt` u klasičnom Netscape formatu kolačića
(isti format koji čita vlastita opcija `--cookies` alata yt-dlp); Vivace
ne čita kolačiće izravno iz profila preglednika.

**Kako ga izraditi:**

1. Prijavite se na youtube.com u svom svakodnevnom pregledniku, s
   računom čiji pristup želite koristiti.
2. Instalirajte proširenje preglednika za izvoz kolačića koje piše u
   Netscape formatu — za Chrome, Edge ili Brave potražite u trgovini
   proširenja svog preglednika nešto poput „Get cookies.txt”; za
   Firefox potražite „cookies.txt”. Svako proširenje koje jasno navodi
   da izvozi u klasičnom Netscape formatu `cookies.txt` će funkcionirati.
3. Dok je youtube.com otvoren u kartici, upotrijebite proširenje za
   izvoz kolačića za tu web-lokaciju i spremite rezultat kao datoteku
   `.txt` negdje na disku.
4. U Vivaceu otvorite *Postavke ▸ Mreža ▸ YouTube* i upotrijebite
   **Pregledaj…** pored **Datoteka kolačića:** za odabir te datoteke.

**Imajte na umu:**

- Datoteka `cookies.txt` zapravo je spremljena prijavljena sesija — bilo
  tko tko posjeduje tu datoteku može djelovati kao vaš YouTube račun sve
  dok kolačići ne isteknu ili se ne odjavite. Čuvajte je na privatnom
  mjestu i nemojte je dijeliti.
- Kolačići se koriste samo za put **preuzimanja** (Preuzmi i reproduciraj
  / vanjski alat). Vivace namjerno nikada ne šalje kolačiće u načinu
  **streaminga** — URL streama s prijavljenim korisnikom vezan je uz tu
  sesiju na način koji jednostavni videoplayer Vivacea ne može otvoriti,
  pa streaming ostaje anoniman čak i ako je konfigurirana datoteka
  kolačića.
- Kolačići istječu. Ako preuzimanja koja su prije radila počnu ne
  uspijevati ili se vrate na rezultat niže kvalitete/javni rezultat,
  izvezite novu datoteku `cookies.txt`.

## Instalacija ffmpeg-a za YouTube preuzimanja

Način rada **Preuzmi i reproduciraj** zahtijeva `ffmpeg` za spajanje
zasebnih video i audio zapisa koje yt-dlp preuzima u jednu datoteku
spremnu za reprodukciju — YouTube rijetko nudi HD kao jedan spojeni
zapis, pa se video zapis i audio zapis preuzimaju odvojeno, a zatim
spajaju. Polje **Lokacija ffmpeg:** (*Postavke ▸ Mreža ▸ YouTube ▸
Preuzmi i reproduciraj*) govori yt-dlp-u gdje ga pronaći; ostavite
prazno da se koristi `ffmpeg` sa sistemske PATH varijable.

**Instalacija ffmpeg-a:**

1. **Windows** — najjednostavnija je opcija upravitelj paketima:
   `winget install ffmpeg` (ili `scoop install ffmpeg` /
   `choco install ffmpeg`). Alternativno, preuzmite gotovu arhivu s
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) ili
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) i
   raspakirajte je negdje.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — instalirajte ga putem upravitelja paketima svoje
   distribucije, npr. `sudo apt install ffmpeg` (Debian/Ubuntu),
   `sudo dnf install ffmpeg` (Fedora), ili `sudo pacman -S ffmpeg`
   (Arch).
4. Ako ste dodali ffmpeg u sistemsku PATH varijablu, ostavite
   **Lokacija ffmpeg:** prazno. U suprotnom, zalijepite putanju do
   *mape* koja sadrži izvršnu datoteku `ffmpeg` (ne samu izvršnu
   datoteku) u to polje.
5. Ponovno pokrenite Vivace (ili samo pokušajte preuzimanje ponovno)
   nakon instalacije.

**Zapamtite:**

- Ovo je ovisnost o **yt-dlp-u**, poput Dena ispod — Vivace ga uvijek
  pokreće samo kao vanjski proces.
- Način **strujanja** nikada ne zahtijeva ffmpeg, jer reproducira jedan
  već spojeni zapis; potreban je samo za **Preuzmi i reproduciraj**,
  jer taj način dohvaća video i audio odvojeno i spaja ih lokalno.
- Ako preuzimanje ne uspije uz grešku povezanu sa spajanjem, prvo
  provjerite lokaciju ffmpeg-a — to je najčešći uzrok, osim
  nedostajućeg ili zastarjelog Dena.

## Instalacija Deno-a za YouTube preuzimanja

Sam yt-dlp — ne samo Vivace — koristi zaseban vanjski JavaScript izvršni
sustav za rješavanje izazova koje YouTube postavlja prije nego što otkrije
stvarni URL za preuzimanje videozapisa. Prema vlastitoj dokumentaciji
yt-dlp-a, pokretanje bez njega je "zastarjelo", no ne dovodi izravno do
neuspjeha: dostupnost formata jednostavno je smanjena, i to **znatno za
prijavljeni (kolačić) zahtjev** — upravo onu vrstu zahtjeva koju način rada
**Preuzmi i reproduciraj** upućuje kako bi otključao HD, sadržaj samo za
članove i videozapise s dobnim ograničenjem. Način **streaminga** nikada ne
šalje kolačiće (pogledajte "Izvoz kolačića za YouTube preuzimanja" iznad),
pa to nije ozbiljan slučaj i u većini slučajeva funkcionira dobro i bez
Dena. Zato se polje **Putanja do Deno-a:** nalazi pod *Postavke ▸ Mreža ▸
YouTube ▸ Preuzmi i reproduciraj*, a ne kao opća YouTube postavka. yt-dlp
podržava nekoliko JS izvršnih sustava; Deno je onaj koji po zadanom traži.

**Instalacija Dena:**

1. Slijedite službene upute za instalaciju na
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   za svoj operacijski sustav (instalacijska skripta ili upravitelj
   paketima poput winget/scoop/Homebrew/apt, ovisno o platformi).
2. Provjerite da se izvršna datoteka `deno` nalazi na sistemskoj PATH
   varijabli — gornji instalacijski programi to obično učine umjesto vas.
   Na Windowsima svakako preuzmite `deno`, a ne `denort` (drugu, povezanu
   izvršnu datoteku koja ovdje ne funkcionira).
3. Ako radije ne biste mijenjali PATH, ostavite ga kakav jest i umjesto
   toga zalijepite punu putanju u polje **Putanja do Deno-a:** (*Postavke ▸
   Mreža ▸ YouTube ▸ Preuzmi i reproduciraj*).
4. Ponovno pokrenite Vivace (ili samo pokušajte preuzimanje ponovno) nakon
   instalacije.

**Zapamtite:**

- Ovo je ovisnost o **yt-dlp-u**, a ne izravno o Vivaceu — Vivace uvijek
  pokreće yt-dlp samo kao vanjski proces i nikada sam ne poziva Deno.
- yt-dlp zahtijeva razmjerno noviju verziju Dena (2.3.0 ili noviju u
  trenutku pisanja). Ako preuzimanja i nakon instalacije i dalje pokazuju
  smanjenu kvalitetu/greške formata, provjerite `deno --version` i
  ažurirajte ga ako je stariji.
- Ovaj zahtjev proizlazi iz promjena na strani YouTubea/yt-dlp-a, a ne
  Vivacea — isto polje **Putanja do Deno-a:** postoji upravo iz tog
  razloga i ne zahtijeva daljnju konfiguraciju nakon što je sam Deno
  instaliran i dostupan.
