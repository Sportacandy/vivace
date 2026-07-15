# Valinnat

**Valinnat**-valikko sisältää asetukset ja käyttöliittymän määrityksen.

- **Asetukset…** (`Ctrl+P`) — pääasetusikkuna: Yleiset, Käyttöliittymä, Tekstitys,
  Näppäimistö ja hiiri, Soittolista, Asemat, TV, Tiedostotyypit, Päivitykset,
  Verkko ja Lisäasetukset. Muutokset tulevat voimaan heti; **Peruuta** kumoaa ne.
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

## Evästeiden vieminen YouTube-latauksia varten

Kenttä **Evästetiedosto:** (*Asetukset ▸ Verkko ▸ YouTube*) antaa
YouTube-tiloille **Lataa ja toista** ja **ulkoinen työkalu**
mahdollisuuden toimia ikään kuin olisit kirjautunut sisään — tarpeen
ikärajoitetuille, vain jäsenille tarkoitetuille tai muuten tiliin
sidotuille videoille, ja se on myös se, mikä avaa täydet HD/4K-lataukset.
Se odottaa pelkkää tekstitiedostoa, `cookies.txt`, klassisessa
Netscape-evästemuodossa (sama muoto, jota yt-dlpin oma valitsin
`--cookies` lukee); Vivace ei lue evästeitä suoraan selaimen profiilista.

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

**Muista:**

- `Cookies.txt`-tiedosto on käytännössä tallennettu kirjautumisistunto —
  kuka tahansa, jolla on tiedosto, voi toimia YouTube-tilinäsi, kunnes
  evästeet vanhenevat tai kirjaudut ulos. Säilytä sitä yksityisessä
  paikassa äläkä jaa sitä.
- Evästeitä käytetään vain **lataus**-polulla (Lataa ja toista / ulkoinen
  työkalu). Vivace ei koskaan lähetä evästeitä **suoratoisto**tilassa
  tarkoituksella — kirjautuneen suoratoisto-URL:n on sidottu tähän
  istuntoon tavalla, jota Vivacen yksinkertainen videosoitin ei pysty
  avaamaan, joten suoratoisto pysyy nimettömänä, vaikka evästetiedosto
  olisi määritetty.
- Evästeet vanhenevat. Jos aiemmin toimineet lataukset alkavat
  epäonnistua tai palata alempilaatuiseen/julkiseen tulokseen, vie uusi
  `cookies.txt`.
