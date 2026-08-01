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

## A yt-dlp telepítése és frissítése

A *Beállítások ▸ Hálózat ▸ YouTube* lapon található a **Felügyelt yt-dlp
használata** jelölőnégyzet, amely meghatározza, hogyan szerzi be és
tartja karban a Vivace a YouTube-linkek lejátszásához használt `yt-dlp`
programot:

- **Bekapcsolva** (alapértelmezett) — a Vivace képes telepíteni a
  `yt-dlp`-t, és naprakészen tartani. A **yt-dlp útvonala:** mező a
  Vivace saját másolatára van rögzítve, és nem szerkeszthető közvetlenül;
  a legújabb hivatalos kiadás lekéréséhez használd a **yt-dlp
  telepítése/frissítése…** gombot (a jelölőnégyzet mellett). A **yt-dlp
  automatikus frissítése:** beállítás is elérhetővé válik, amellyel a
  Vivace saját maga elvégezheti ezt a frissítést: **Soha**, **Minden
  yt-dlp-futtatáskor**, **Naponta egyszer** vagy **Hetente egyszer**. Az
  automatikus frissítés közvetlenül azelőtt fut le, hogy egy YouTube-URL
  ténylegesen feloldásra vagy letöltésre kerülne, így az első lejátszás
  azután, hogy a frissítés esedékessé válik, kicsit tovább tart; ha maga
  a frissítés meghiúsul (pl. nincs hálózati kapcsolat), a Vivace csendben
  a már telepített verzióval folytatja, ahelyett hogy blokkolná a
  lejátszást.
- **Kikapcsolva** — a magad kezelte yt-dlp-hez (pl. `pip`-pel vagy az
  operációs rendszer csomagkezelőjével telepítve). A **yt-dlp útvonala:**
  mező szerkeszthetővé válik, hogy arra a másolatra tudj mutatni, a
  **yt-dlp automatikus frissítése** pedig letiltásra kerül — a Vivace
  soha nem telepít vagy frissít olyan yt-dlp-t, amelyet nem ő kezel. A
  **yt-dlp telepítése/frissítése…** gomb szintén le van tiltva ebben a
  módban.

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
  **Adatfolyam** módban — egy bejelentkezett stream URL úgy kötődik
  ahhoz a munkamenethez, hogy a Vivace egyszerű videólejátszója nem
  tudja megnyitni, így a streamelés névtelen marad még akkor is, ha be
  van állítva egy sütifájl.
- A sütik lejárnak. Ha a korábban működő letöltések hibázni kezdenek,
  vagy alacsonyabb minőségű/nyilvános eredményre esnek vissza, exportálj
  egy friss `cookies.txt` fájlt.

## ffmpeg telepítése YouTube-letöltésekhez

A **Letöltés és lejátszás** módhoz szükség van az `ffmpeg` programra,
hogy összefésülje a yt-dlp által letöltött külön videó- és
hangfolyamokat egyetlen lejátszható fájllá — a YouTube ritkán kínálja a
HD-t egyetlen kombinált folyamként, ezért egy videosáv és egy hangsáv
külön kerül letöltésre, majd összefésülésre. A **ffmpeg helye:** mező
(*Beállítások ▸ Hálózat ▸ YouTube ▸ Letöltés és lejátszás*) megmondja a
yt-dlp-nek, hol találja meg a programot; hagyd üresen, ha inkább a
rendszer PATH változójában található `ffmpeg`-et szeretnéd használni.

**A ffmpeg telepítése:**

1. **Windows** — a legegyszerűbb megoldás egy csomagkezelő:
   `winget install ffmpeg` (vagy `scoop install ffmpeg` / `choco install
   ffmpeg`). Alternatívaként letölthetsz egy előre elkészített
   archívumot innen: [gyan.dev](https://www.gyan.dev/ffmpeg/builds/)
   vagy [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds), és
   kicsomagolhatod valahová.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — telepítsd a disztribúciód csomagkezelőjéből, pl. `sudo
   apt install ffmpeg` (Debian/Ubuntu), `sudo dnf install ffmpeg`
   (Fedora) vagy `sudo pacman -S ffmpeg` (Arch).
4. Ha hozzáadtad az ffmpeget a rendszer PATH-jához, hagyd üresen a
   **ffmpeg helye:** mezőt. Ellenkező esetben illeszd be a mezőbe a
   `ffmpeg` végrehajtható fájlt tartalmazó *mappa* elérési útját (ne
   magát a végrehajtható fájlt).
5. Telepítés után indítsd újra a Vivace-ot (vagy egyszerűen próbáld újra
   a letöltést).

**Ne feledd:**

- Ez a **yt-dlp** függősége, akárcsak az alább következő Deno — a
  Vivace csak külső folyamatként futtatja.
- Az **Adatfolyam** mód soha nem igényel ffmpeget, mivel egy már
  összefésült folyamot játszik le; csak a **Letöltés és lejátszás** mód
  igényli, mert ez a mód külön tölti le a videót és a hangot, majd
  helyben fésüli össze őket.
- Ha egy letöltés összefésüléssel kapcsolatos hibával meghiúsul, először
  ellenőrizd az ffmpeg helyét — ez a leggyakoribb ok, a hiányzó vagy
  elavult Deno mellett.

## Deno telepítése YouTube-letöltésekhez

Maga a yt-dlp — nem csak a Vivace — egy külön, külső JavaScript
futtatókörnyezetet használ azoknak a kihívásoknak a megoldására, amelyeket
a YouTube állít, mielőtt kiadná egy videó valódi letöltési URL-jét. A
yt-dlp saját dokumentációja szerint a futtatókörnyezet nélküli futtatás
„elavult”, de nem hiúsul meg teljesen: a formátumok elérhetősége
egyszerűen csökken, és ez **különösen súlyos egy bejelentkezett (süti
alapú) kérés esetén** — pontosan az ilyen típusú kérést használja a
**Letöltés és lejátszás** mód a HD-, csak tagoknak elérhető és
korhatáros videók feloldásához. Az **Adatfolyam** mód soha nem küld
sütiket (lásd a fenti „Cookie-k exportálása YouTube-letöltésekhez”
szakaszt), így ez nem a súlyos eset, és a legtöbb esetben Deno nélkül is
jól működik. Ezért található a **Deno útvonala:** mező a *Beállítások ▸
Hálózat ▸ YouTube ▸ Letöltés és lejátszás* alatt, nem pedig egy általános
YouTube-beállításként. A yt-dlp több JS futtatókörnyezetet is támogat; a
Deno az, amelyet alapértelmezés szerint keres.

**A Deno telepítése:**

1. Kövesd az operációs rendszeredhez tartozó hivatalos telepítési
   útmutatót a [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   oldalon (egy telepítőszkript, vagy egy csomagkezelő, például
   winget/scoop/Homebrew/apt, a platformtól függően).
2. Győződj meg róla, hogy a `deno` végrehajtható fájl bekerül a
   rendszer PATH változójába — a fenti telepítők ezt általában
   automatikusan elvégzik. Windows alatt ügyelj arra, hogy a `deno`-t
   szerezd be, ne a `denort`-ot (egy másik, kapcsolódó, de itt nem
   működő program).
3. Ha inkább nem szeretnéd módosítani a PATH-ot, hagyd változatlanul,
   és illeszd be helyette a teljes útvonalát a **Deno útvonala:**
   mezőbe (*Beállítások ▸ Hálózat ▸ YouTube ▸ Letöltés és lejátszás*).
4. Telepítés után indítsd újra a Vivace-ot (vagy egyszerűen próbáld
   újra a letöltést).

**Ne feledd:**

- Ez a **yt-dlp**, nem közvetlenül a Vivace függősége — a Vivace csak
  külső folyamatként futtatja a yt-dlp-t, és soha nem hívja meg magát a
  Denót.
- A yt-dlp-nek egy kellően friss Deno-verzióra van szüksége (a jelen
  írás idején 2.3.0 vagy újabb). Ha a letöltések a telepítés után is
  csökkentett minőséget mutatnak, vagy formátumhibát adnak, ellenőrizd a
  `deno --version` kimenetét, és frissítsd, ha régebbi.
- Ez a követelmény a YouTube/yt-dlp oldalán történt változásokból ered,
  nem a Vivace-ból — ugyanez a **Deno útvonala:** mező pontosan ezért
  létezik, és nincs szükség további beállításra, ha maga a Deno
  telepítve van és elérhető.
