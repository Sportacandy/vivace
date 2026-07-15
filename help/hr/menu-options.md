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
