# Valinnat

**Valinnat**-valikko sisältää asetukset ja käyttöliittymän määrityksen.

- **Asetukset…** (`Ctrl+P`) — pääasetusikkuna: Yleiset, Käyttöliittymä, Tekstitys,
  Näppäimistö ja hiiri, Soittolista, Asemat, TV, Tiedostotyypit, Päivitykset,
  Verkko ja Lisäasetukset. Muutokset tulevat voimaan heti; **Peruuta** kumoaa ne.
  - **Yleiset ▸ Video** asettaa oletusarvon kohdalle **Lomituksen poisto**
    (Ei mitään / Yadif / Bwdif / Automaattinen) äskettäin avatuille
    tiedostoille — muuta sitä tiedostokohtaisesti kohdassa *Video ▸
    Lomituksen poisto* (vain Ei mitään/Yadif/Bwdif; Automaattinen ei ole
    tarjolla siellä, koska se on mielekäs vain oletusarvona).
    **Automaattinen** käyttää Bwdifiä, mutta vain kuvissa, jotka tiedosto
    itse merkitsee lomitetuiksi, jättäen progressiiviset kuvat
    koskemattomiksi.
  - **Yleiset ▸ Haluttu ääni ja tekstitys** asettaa kielet, joiden
    joukosta Vivace valitsee automaattisesti tiedoston sisäänrakennetuista
    raidoista. **Näytä tekstitykset oletuksena** ottaa tekstityksen
    automaattisesti käyttöön, kun sellainen on saatavilla; sen alavalinta
    **...mutta ei, jos ääni on jo suositellulla kielellä** ohittaa tämän
    automaattisen tekstityksen, kun sen kieli vastaa todellisuudessa
    valittua ääniraitaa — hyödyllinen, jos ymmärrät äänen jo etkä halua
    samankielisen tekstityksen häiritsevän.
  - **Verkko** sisältää välilehdet OpenSubtitles, YouTube, Proxy ja Lähetys;
    **Proxy** määrittää valinnaisen HTTP- tai SOCKS5-välityspalvelimen, jota
    käytetään koko sovelluksessa (OpenSubtitles-haku, päivitystarkistus ja —
    vain HTTP:llä — toisto ja yt-dlp); **Lähetys** kiinnittää portin, jota
    *Toista ▸ Lähetä ▸ Puhelin/tabletti* kuuntelee. OpenSubtitles-tilin
    salasana ja välityspalvelimen salasana tallennetaan turvallisesti
    käyttöjärjestelmän tunnistetietojen hallintaan, ei Vivacen omiin
    asetuksiin.
- **Näytä kuvake ilmoitusalueella** — pidä Vivace tavoitettavissa ilmoitusalueelta.
- **Työkalurivit**
  - **Työkalurivi** / **Ohjauspalkki** — näytä tai piilota kukin palkki.
  - **Muokkaa päätyökaluriviä…** / **Muokkaa ohjauspalkkia…** — valitse, mitkä
    painikkeet näkyvät, sekä niiden järjestys ja kuvakekoko.
- **Tilarivi**
  - **Näytä tilarivi** ja mitä se näyttää: **Videotiedot**, **Äänitiedot**,
    **Muototiedot**, **Bittinopeustiedot**, **Ruutulaskuri**, **Näytä kokonaisaika**,
    **Näytä jäljellä oleva aika** ja **Näytä nykyinen aika millisekunteineen**.

**Vihje:** yleisasettelu (Basic / Mini / MPC) valitaan kohdassa *Asetukset ▸ Käyttöliittymä*.

## yt-dlp:n asentaminen ja päivittäminen

Kohdassa *Asetukset ▸ Verkko ▸ YouTube* on valintaruutu **Käytä hallittua
yt-dlp:tä**, joka määrittää, miten Vivace hankkii ja ylläpitää
YouTube-linkkien toistamiseen käyttämäänsä `yt-dlp`-ohjelmaa:

- **Käytössä** (oletus) — Vivace voi asentaa `yt-dlp`:n puolestasi ja
  pitää sen ajan tasalla. Kenttä **yt-dlp-polku:** on kiinnitetty
  Vivacen omaan kopioon eikä sitä voi muokata suoraan; käytä painiketta
  **Asenna/päivitä yt-dlp…** (valintaruudun vieressä), kun haluat hakea
  uusimman virallisen julkaisun. Asetus **Päivitä yt-dlp
  automaattisesti:** tulee myös käyttöön, jolloin Vivace voi hoitaa
  tämän päivityksen itse: **Ei koskaan**, **Aina kun yt-dlp
  suoritetaan**, **Kerran päivässä** tai **Kerran viikossa**.
  Automaattinen päivitys suoritetaan juuri ennen kuin YouTube-URL
  todella ratkaistaan tai ladataan, joten ensimmäinen toisto sen jälkeen,
  kun päivitys erääntyy, kestää hieman kauemmin; jos itse päivitys
  epäonnistuu (esim. ei verkkoyhteyttä), Vivace jatkaa hiljaisesti jo
  asennetulla versiolla sen sijaan, että se estäisi toiston.
- **Ei käytössä** — itse hallitsemaasi yt-dlp:hen (esim. asennettu
  `pip`:llä tai käyttöjärjestelmän pakettienhallinnalla). Kenttä
  **yt-dlp-polku:** muuttuu muokattavaksi, jotta voit osoittaa sen
  kyseiseen kopioon, ja **Päivitä yt-dlp automaattisesti** poistetaan
  käytöstä — Vivace ei koskaan asenna tai päivitä yt-dlp:tä, jota se ei
  hallitse. Myös painike **Asenna/päivitä yt-dlp…** on poissa käytöstä
  tässä tilassa.

## YouTuben PO-token-palveluntarjoajan asentaminen

Uudet YouTube-videot vaativat yhä useammin **PO-tokenin
(proof-of-origin)** edes toiston mahdollistamiseksi — yleisen
toistettavuusvaatimuksen, joka on riippumaton evästeistä tai
kirjautumisesta. Ilman sitä yt-dlp ilmoittaa videon olevan
tavoittamattomissa jopa tavallisella, anonyymillä pyynnöllä. Tämä koskee
**sekä** Suoratoisto- että Lataa ja toista -tiloja, toisin kuin evästeet
(vain latauksia varten, katso alta) tai Deno (lähinnä Lataa ja toista
-tilan asia, katso alempaa).

Kohdassa *Asetukset ▸ Verkko ▸ YouTube* on painike **Asenna
PO-token-palveluntarjoaja…**, joka ottaa käyttöön yhteisön
**"BgUtils POT Provider"** -projektin: pienen yt-dlp-laajennuksen sekä
skriptin — joka ajetaan tarvittaessa Denolla, samalla ohjelmalla, jota
käsitellään alempana — minkä ansiosta yt-dlp voi luoda tokenin
automaattisesti. Napsauta sitä, odota, että lyhyt lataus- ja
koontiprosessi valmistuu, ja se on tehty; painikkeen teksti muuttuu
muotoon **Asenna uudelleen / päivitä PO-token-palveluntarjoaja…**, kun
se on asennettu, siltä varalta, että myöhempi päivitys on joskus
tarpeen.

**Muista:**

- Tämä vaatii toimivan Deno-asennuksen (katso "Denon asentaminen
  YouTube-latauksia varten" alta) — painike lataa ja koontaa
  palveluntarjoajan oman skriptin, jonka yt-dlp sitten ajaa Denolla aina
  kun video todella tarvitsee tokenin.
- Kaikki videot eivät tarvitse PO-tokenia, joten toisto voi toimia hyvin
  ilman tätä asennusta — mutta jos video ilmoittaa olevansa
  tavoittamattomissa, vaikka se toimii hyvin muualla, tämän asentaminen
  kannattaa.
- **Androidissa** tämä sisältyy Vivaceen ja otetaan käyttöön
  automaattisesti — mitään ei tarvitse asentaa, eikä vastaavaa painiketta
  ole; se vain toimii.

## Evästeet YouTube-latauksia varten

YouTube-tilat **Lataa ja toista** ja **ulkoinen työkalu** voivat toimia
ikään kuin olisit kirjautunut sisään — tarpeen ikärajoitetuille, vain
jäsenille tarkoitetuille tai muuten tiliin sidotuille videoille, ja se on
myös se, mikä avaa täydet HD/4K-lataukset. Vivace tukee kolmea tapaa
antaa evästeet (kaikki kohdassa *Asetukset ▸ Verkko ▸ YouTube*): **Hae
evästeet selaimesta** Windowsissa/Linuxissa/macOS:ssä, **Kirjaudu
YouTubeen…** Androidissa, ja `cookies.txt`-tiedoston manuaalinen vienti
varavaihtoehtona kaikkialla.

### Hae evästeet selaimesta (suositeltu Windowsissa/Linuxissa/macOS:ssä)

Yhdistelmäruutu **Hae evästeet selaimesta:** listaa Firefoxin, Chromen,
Edgen, Braven, Chromiumin, Operan, Safarin, Vivaldin ja Whalen. Valitse
selaimesi, niin Vivace lukee sen evästeet suoraan, joka kerta — ei
mitään vietävää, ei mitään vanhenevaa.

**Muista:**

- YouTube lyhensi omien evästeidensä elinikää merkittävästi, joten
  aiemmin viety `cookies.txt`-tiedosto (katso alta) voi vanhentua
  muutamassa päivässä — tämä vaihtoehto välttää sen kokonaan lukemalla
  selaimen omaa, aina ajantasaista evästevarastoa.
- **Windowsissa vain Firefox toimii tässä käytännössä.** Chrome, Edge ja
  muut Chromium-pohjaiset selaimet Windowsissa salaavat evästeensä
  tavalla, joka on sidottu selaimen omaan binääritiedostoon
  ("App-Bound Encryption", Chrome 127+) — tämä estää yt-dlpiä, ja
  kaikkia muita ulkoisia työkaluja, lukemasta niitä lainkaan. Tämä on
  Chromen puolelta tuleva rajoitus; yt-dlpin omat kehittäjät eivät voi
  kiertää sitä. Linuxin ja macOS:n Chrome/Edge eivät kärsi tästä ja
  toimivat normaalisti.
- Selaimen valitseminen tässä ohittaa alla olevan **Evästetiedosto:**
  -kentän, kun molemmat on asetettu.
- **Ei toimi lainkaan Androidissa, eikä toimi Chromessa/Edgessä
  Windowsissa** — katso alta **Kirjaudu YouTubeen** kummassakin
  tapauksessa.

### Kirjaudu YouTubeen (Android, ja varavaihtoehto Chromelle/Edgelle Windowsissa)

Kohdassa *Asetukset ▸ Verkko ▸ YouTube* on **Kirjaudu YouTubeen…**
-painike (evästetiedostokentän vieressä), joka avaa oikean
kirjautumissivun Vivacen sisällä — Androidissa upotetun,
käyttöjärjestelmän oman WebView-näkymän kautta, tai Windowsissa/
Linuxissa/macOS:ssä mukana tulevan Chromium-pohjaisen näkymän
(QtWebEngine) kautta. Kirjaudu sillä tilillä, jonka käyttöoikeuksia
haluat käyttää, ja napauta sitten **Tallenna evästeet** — Vivace lukee
syntyneet istuntoevästeet ja tallentaa ne automaattisesti aktiiviseksi
evästetiedostoksi. Napauta **Sulje**, kun olet valmis. Androidissa tämä
on ainoa tapa saada tuoreita evästeitä ylipäätään; työpöydällä se on
lähinnä Chromea/Edgeä varten Windowsissa, joissa yllä oleva **Hae
evästeet selaimesta** ei pysty lukemaan niiden evästeitä — kummassakin
tapauksessa ei ole mitään vietävää eikä siirrettävää toiselta laitteelta
käsin.

**Muista:**

- Androidissa ei ole lainkaan vastinetta työpöytäselaimen aktiiviselle
  evästevarastolle, josta lukea (jokaisen sovelluksen tallennustila on
  eristetty muiden sovellusten tallennustilasta), minkä vuoksi tämä
  toimii eri tavalla kuin työpöytävaihtoehto — kirjautuminen Vivacen
  omaan upotettuun selaimeen *sisällä* on käytännön korvike.
- Työpöydällä tämä on oikea, erillinen selainistunto, jota Vivace itse
  ohjaa — aidosti raskaampi ominaisuus kuin **Hae evästeet selaimesta**
  (se sisältää oman selainmoottorinsa), joten sitä kannattaa käyttää
  lähinnä silloin, kun tuo yksinkertaisempi vaihtoehto ei toimi, eli
  Chromen/Edgen kanssa Windowsissa.

### Evästeiden vieminen tiedostoon (varavaihtoehto kaikilla alustoilla)

Kenttä **Evästetiedosto:** odottaa pelkkää tekstitiedostoa,
`cookies.txt`, klassisessa Netscape-evästemuodossa (sama muoto, jota
yt-dlpin oma valitsin `--cookies` lukee) — käytä tätä, kun kumpikaan
yllä olevista suorista vaihtoehdoista ei ole saatavilla tai toimi
sinulla (esim. jos **Kirjaudu YouTubeen** ei ole saatavilla käyttämässäsi
Vivace-versiossa).

**Näin luot sellaisen:**

1. Kirjaudu youtube.comiin päivittäin käyttämässäsi selaimessa sillä
   tilillä, jonka käyttöoikeuksia haluat käyttää.
2. Asenna eväste-vientiin tarkoitettu selainlaajennus, joka kirjoittaa
   Netscape-muodossa — Chromessa, Edgessä tai Bravessa etsi selaimesi
   laajennuskaupasta jotain vastaavaa kuin "Get cookies.txt"; Firefoxissa
   etsi "cookies.txt". Mikä tahansa laajennus, joka ilmoittaa selvästi
   vievänsä tiedot klassisessa Netscape `cookies.txt` -muodossa, toimii.
3. Kun youtube.com on auki välilehdellä, vie evästeet kyseiselle
   sivustolle laajennuksen avulla ja tallenna tulos `.txt`-tiedostona
   levylle.
4. Avaa Vivacessa *Asetukset ▸ Verkko ▸ YouTube* ja käytä **Selaa…**
   kohdan **Evästetiedosto:** vieressä valitaksesi kyseisen tiedoston.

**Androidissa:** suosi sen sijaan yllä olevaa **Kirjaudu YouTubeen…**
-toimintoa — se ei vaadi lainkaan vienti-/siirtovaihetta. Jos haluat
silti tehdä sen tällä tavalla, huomaa, että Chrome Androidille ei tue
selainlaajennuksia, joten yllä olevia vaiheita 2–3 ei voi tehdä itse
laitteella. Vie
`cookies.txt` pöytäkoneella tai kannettavalla tietokoneella yllä
kuvatulla tavalla ja siirrä sitten tiedosto Android-laitteellesi (esim.
pilvitallennuksen, USB-kaapelin tai sähköpostin kautta) ennen kuin
käytät **Selaa…**-toimintoa vaiheessa 4.

**Muista:**

- `Cookies.txt`-tiedosto on käytännössä tallennettu kirjautumisistunto —
  kuka tahansa, jolla on tiedosto, voi toimia YouTube-tilinäsi, kunnes
  evästeet vanhenevat tai kirjaudut ulos. Säilytä sitä yksityisessä
  paikassa äläkä jaa sitä.
- Evästeet vanhenevat. Jos aiemmin toimineet lataukset alkavat
  epäonnistua tai palata alempilaatuiseen/julkiseen tulokseen, vie uusi
  `cookies.txt` — tai vaihda yllä olevaan **Hae evästeet selaimesta**
  -vaihtoehtoon, jos se on saatavilla, jotta vältät tämän kokonaan.

**Koskee molempia yllä mainittuja tapoja:**

- Evästeitä käytetään vain **lataus**-polulla (Lataa ja toista / ulkoinen
  työkalu). Vivace ei koskaan lähetä evästeitä **suoratoisto**tilassa
  tarkoituksella — kirjautuneen suoratoisto-URL:n on sidottu tähän
  istuntoon tavalla, jota Vivacen yksinkertainen videosoitin ei pysty
  avaamaan, joten suoratoisto pysyy nimettömänä riippumatta siitä, kumpi
  tapa on määritetty.

## ffmpegin asentaminen YouTube-latauksia varten

**Lataa ja toista** -tila tarvitsee ohjelman `ffmpeg` yhdistääkseen
yt-dlpin lataamat erilliset video- ja äänivirrat yhdeksi toistettavaksi
tiedostoksi — YouTube tarjoaa HD:n harvoin yhtenä yhdistettynä virtana,
joten videoraita ja ääniraita ladataan erikseen ja yhdistetään sitten.
Kenttä **ffmpegin sijainti:** (*Asetukset ▸ Verkko ▸ YouTube ▸ Lataa ja
toista*) kertoo yt-dlpille, mistä se löytää ohjelman; jätä kenttä
tyhjäksi, jos haluat käyttää järjestelmän PATH-muuttujasta löytyvää
`ffmpeg`-ohjelmaa sen sijaan.

**ffmpegin asentaminen:**

1. **Windows** — helpoin tapa on pakettienhallinta: `winget install
   ffmpeg` (tai `scoop install ffmpeg` / `choco install ffmpeg`).
   Vaihtoehtoisesti voit ladata valmiiksi käännetyn paketin osoitteesta
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) tai
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) ja purkaa
   sen johonkin.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — asenna se jakelusi pakettienhallinnasta, esim. `sudo apt
   install ffmpeg` (Debian/Ubuntu), `sudo dnf install ffmpeg` (Fedora) tai
   `sudo pacman -S ffmpeg` (Arch).
4. Jos lisäsit ffmpegin järjestelmän PATH-muuttujaan, jätä
   **ffmpegin sijainti:** tyhjäksi. Muussa tapauksessa liitä kenttään
   polku *kansioon*, joka sisältää `ffmpeg`-ohjelman (ei itse ohjelmaa).
5. Käynnistä Vivace uudelleen (tai yritä vain ladata uudelleen)
   asennuksen jälkeen.

**Muista:**

- Tämä on **yt-dlp:n**, ei Vivacen, riippuvuus, aivan kuten alla oleva
  Deno — Vivace vain suorittaa sitä ulkoisena prosessina.
- **Suoratoisto**-tila ei koskaan tarvitse ffmpegiä, koska se toistaa jo
  yhdistetyn virran; vain **Lataa ja toista** tarvitsee, koska se tila
  hakee videon ja äänen erikseen ja yhdistää ne paikallisesti.
- Jos lataus epäonnistuu yhdistämiseen liittyvään virheeseen, tarkista
  ensin ffmpegin sijainti — se on yleisin syy puuttuvan tai vanhentuneen
  Denon ohella.

## Denon asentaminen YouTube-latauksia varten

yt-dlp itse — ei vain Vivace — käyttää erillistä, ulkoista
JavaScript-ajoympäristöä ratkaistakseen haasteet, joita YouTube asettaa
ennen videon todellisen latausosoitteen luovuttamista. yt-dlpin oman
dokumentaation mukaan ajaminen ilman sitä on "vanhentunutta" (deprecated),
mutta ei suoranaisesti epäonnistu: muotojen saatavuus vain vähenee, ja
**merkittävästi kirjautuneen (evästepohjaisen) pyynnön kohdalla** — juuri
sellaisen pyynnön, jonka **Lataa ja toista** -tila tekee avatakseen
HD-, jäsen- ja ikärajoitetut videot. **Suoratoisto**-tila ei koskaan
lähetä evästeitä (katso yllä oleva "Evästeet YouTube-latauksia
varten"), joten se ei ole tämä vakavampi tapaus, ja se toimii useimmiten
hyvin ilman Denoa. Tästä syystä kenttä **Deno-polku:** sijaitsee kohdassa
*Asetukset ▸ Verkko ▸ YouTube ▸ Lataa ja toista* eikä yleisenä
YouTube-asetuksena. yt-dlp tukee useita JS-ajoympäristöjä; Deno on se,
jota se etsii oletusarvoisesti.

**Denon asentaminen:**

1. Seuraa virallisia asennusohjeita osoitteessa
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   käyttöjärjestelmällesi (asennusskripti tai pakettienhallinta, kuten
   winget/scoop/Homebrew/apt, alustasta riippuen).
2. Varmista, että `deno`-suoritettava tiedosto päätyy järjestelmän
   PATH-muuttujaan — yllä mainitut asennusohjelmat tekevät tämän
   yleensä puolestasi. Varmista Windowsissa, että saat `deno`-tiedoston
   etkä `denort`-tiedostoa (eri, siihen liittyvä ohjelma, joka ei toimi
   tässä).
3. Jos et halua muokata PATH-muuttujaa, jätä se ennalleen ja liitä sen
   sijaan koko polku kenttään **Deno-polku:** (*Asetukset ▸ Verkko ▸
   YouTube ▸ Lataa ja toista*).
4. Käynnistä Vivace uudelleen (tai yritä vain lataamista uudelleen)
   asennuksen jälkeen.

**Muista:**

- Tämä on **yt-dlp:n**, ei suoraan Vivacen, riippuvuus — Vivace vain
  suorittaa yt-dlp:tä ulkoisena prosessina eikä koskaan kutsu Denoa
  itse.
- yt-dlp vaatii kohtuullisen tuoreen Deno-version (2.3.0 tai uudemman
  tätä kirjoitettaessa). Jos lataukset yhä näyttävät heikentynyttä
  laatua/muotovirheitä asennuksen jälkeen, tarkista `deno --version` ja
  päivitä se, jos se on vanhempi.
- Tämä vaatimus johtuu muutoksista YouTuben/yt-dlpin puolella, ei
  Vivacesta — sama kenttä **Deno-polku:** on olemassa juuri tästä
  syystä eikä vaadi enää lisäasetuksia, kun Deno itse on asennettu ja
  tavoitettavissa.

## Bittikarttatekstitysten pehmennys

*Asetukset ▸ Tekstitykset ▸ Bittikarttatekstitykset* sisältää
asetuksen **Pehmennys:** (0–3, oletusarvo 1) tekstitykselle, joka
näytetään kuvina eikä tekstinä -- DVD-alikuva-, PGS- ja DVB-raidat.
Tämä koskee sekä oikean DVD-levyn omaa tekstitystä että samantyyppistä
upotettua tekstitysraitaa tavallisessa videotiedostossa (esim. .mp4-
tiedostossa, jossa on `dvd_subtitle`-koodekin raita). Nämä formaatit
ovat valmiiksi renderöityjä bittikarttakuvia, jotka on poltettu
alkuperäisessä vakiotarkkuudessa (SD) lähteen valmistuksen yhteydessä
-- niiden reunat voivat näyttää rosoisilta, kun ne suurennetaan
nykyaikaisen ikkunan kokoon. Vivace voi pehmentää näitä reunoja
kevyellä sumennuksella:

- **0** — pois käytöstä; näyttää alkuperäisen tekstitysbittikartan
  täsmälleen sellaisena kuin se on luotu.
- **1** (oletus) — pehmentää karkeimmat reunat säilyttäen tekstin
  lähes täyden kirkkauden.
- **2** / **3** — asteittain voimakkaampi sumennus.

Tämä asetus vaikuttaa vain bittikarttatekstityksiin — sillä ei ole
vaikutusta Vivacen omaan ulkoiseen tekstitysrenderöijään (SRT/VTT/ASS)
eikä tavallisiin tekstipohjaisiin tekstitysraitoihin, jotka molemmat
käyttävät eri renderöintipolkuja.
