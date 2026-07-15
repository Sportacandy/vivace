# Beállítások menü

A **Beállítások** menü tartalmazza a beállításokat és a felület konfigurációját.

- **Beállítások…** (`Ctrl+P`) — a fő beállítások párbeszédpanel: Általános,
  Felület, Felirat, Billentyűzet és egér, Lejátszási lista, Meghajtók, TV,
  Fájltípusok, Frissítések, Hálózat és Speciális. A módosítások azonnal
  érvénybe lépnek; a **Mégse** visszavonja őket.
  - A **Hálózat** oldal az OpenSubtitles, YouTube, Proxy és Átküldés füleket
    tartalmazza; a **Proxy** fül egy opcionális HTTP- vagy SOCKS5-proxyt
    állít be, amely az egész alkalmazásra érvényes (OpenSubtitles-keresés,
    frissítésellenőrzés, valamint — csak HTTP esetén — lejátszás és yt-dlp);
    az **Átküldés** fül rögzíti azt a portot, amelyet a *Lejátszás ▸ Átküldés
    ide ▸ Okostelefon/Tablet* figyel. Az OpenSubtitles-fiók jelszava és a
    proxy jelszava biztonságosan, az operációs rendszer
    hitelesítőadat-kezelőjében tárolódik, nem a Vivace saját beállításaiban.
- **Ikon megjelenítése a tálcán** — a Vivace elérhető marad a tálcáról.
- **Eszköztárak**
  - **Eszköztár** / **Vezérlősáv** — az egyes sávok megjelenítése vagy elrejtése.
  - **Fő eszköztár szerkesztése…** / **Vezérlősáv szerkesztése…** — válassza ki,
    mely gombok jelenjenek meg, valamint sorrendjüket és ikonméretüket.
- **Állapotsor**
  - **Állapotsor megjelenítése** és a megjelenítettek: **Videóinformáció**,
    **Hanginformáció**, **Formátuminformáció**, **Bitráta-információ**,
    **Képkockaszámláló**, **Teljes idő megjelenítése**, **Hátralévő idő
    megjelenítése** és **Aktuális idő megjelenítése ezredmásodperccel**.

**Tipp:** az általános elrendezést (Basic / Mini / MPC) a *Beállítások ▸ Felület* alatt választhatja ki.

## Cookie-k exportálása YouTube-letöltésekhez

A **Sütifájl:** mező (*Beállítások ▸ Hálózat ▸ YouTube*) lehetővé teszi,
hogy a **Letöltés és lejátszás** és a **külső eszköz** YouTube-módok úgy
viselkedjenek, mintha be lennél jelentkezve — ez szükséges a
korhatáros, csak tagoknak elérhető vagy más módon fiókhoz kötött
videókhoz, és ez teszi lehetővé a teljes HD/4K letöltéseket is. Egy
egyszerű szöveges `cookies.txt` fájlt vár a klasszikus Netscape
süti-formátumban (ugyanaz a formátum, amelyet a yt-dlp saját
`--cookies` kapcsolója is olvas); a Vivace nem olvassa ki közvetlenül a
sütiket egy böngésző profiljából.

**Létrehozás menete:**

1. Jelentkezz be a youtube.com oldalra a mindennapi böngésződben, azzal
   a fiókkal, amelynek hozzáférését használni szeretnéd.
2. Telepíts egy süti-exportáló böngészőbővítményt, amely Netscape
   formátumban ír — Chrome, Edge vagy Brave esetén keress a böngésződ
   bővítménybolt­jában valami olyasmit, mint a „Get cookies.txt”;
   Firefoxhoz keress rá a „cookies.txt”-re. Bármelyik bővítmény
   megfelel, amely egyértelműen jelzi, hogy a klasszikus Netscape
   `cookies.txt` formátumban exportál.
3. Miközben a youtube.com egy lapon nyitva van, használd a bővítményt a
   sütik exportálásához az adott oldalhoz, és mentsd az eredményt egy
   `.txt` fájlba valahová a lemezre.
4. A Vivace-ban nyisd meg a *Beállítások ▸ Hálózat ▸ YouTube* lapot, és
   használd a **Sütifájl:** melletti **Tallózás…** gombot a fájl
   kiválasztásához.

**Ne feledd:**

- A `cookies.txt` fájl gyakorlatilag egy elmentett bejelentkezési
  munkamenet — bárki, akinek megvan a fájl, felhasználhatja a YouTube
  fiókodat, amíg a sütik le nem járnak, vagy ki nem jelentkezel. Tárold
  privát helyen, és ne oszd meg senkivel.
- A sütiket csak a **letöltési** útvonal használja (Letöltés és
  lejátszás / külső eszköz). A Vivace szándékosan soha nem küld sütiket
  **streamelési** módban — egy bejelentkezett stream URL úgy kötődik
  ahhoz a munkamenethez, hogy a Vivace egyszerű videólejátszója nem
  tudja megnyitni, így a streamelés névtelen marad még akkor is, ha be
  van állítva egy sütifájl.
- A sütik lejárnak. Ha a korábban működő letöltések hibázni kezdenek,
  vagy alacsonyabb minőségű/nyilvános eredményre esnek vissza, exportálj
  egy friss `cookies.txt` fájlt.
