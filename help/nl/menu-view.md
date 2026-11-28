# Weergave

Het menu **Weergave** toont informatie en secundaire panelen.

- **Informatie en eigenschappen…** — een gedetailleerd venster over de huidige
  media: algemene info, demuxer en de video-/audiocodecs (met de formaten die de
  backend van Vivace ondersteunt gemarkeerd). Het is alleen informatief.
- **Afspeellijst** (`F9`) — de afspeellijst tonen of verbergen. Afhankelijk van
  *Voorkeuren ▸ Interface* verschijnt ze als vastgezet paneel of apart venster.
  Zie [Openen](menu-open.md) voor het laden van afspeellijsten.
  - Elke rij toont een miniatuur — een naast het mediabestand staand
    afbeeldingsbestand (een "`<name>.jpg`" die al ernaast staat) als dat
    bestaat, anders een frame dat automatisch op de achtergrond wordt
    vastgelegd. De knop rechtsboven in de afspeellijst schakelt tussen
    rij-/miniatuurgroottes (**Klein**/**Gemiddeld**/**Groot**); bij Klein en
    Gemiddeld worden de geselecteerde rij en de rij van het afspelende item
    elk vergroot voor een groter voorbeeld. *Voorkeuren ▸ Afspeellijst ▸
    Diversen* heeft voor elk van de twee een eigen instelling ("Miniatuur
    van het geselecteerde item" / "Miniatuur van het afspelende item"): geen
    vergroting, alleen die ene rij vergroten, of ook de paar rijen ernaast
    geleidelijk naar dezelfde grootte laten toelopen (de
    standaardinstelling voor beide). Gegenereerde miniaturen worden gecachet
    (de langst niet getoonde wordt verwijderd zodra de cache vol is); de
    cachegrootte is instelbaar bij *Voorkeuren ▸ Afspeellijst ▸ Diversen ▸
    Maximum aantal gecachte miniaturen*.
  - Een rij zonder miniatuur toont een pictogram in plaats van een leeg vak.
    Een muziekbestand toont een muzieknoot in een van tien kleuren; de kleur
    hangt af van het rijnummer. Andere items waarvoor geen miniatuur kan
    worden gemaakt, zoals een stream of een bestand dat ffmpeg niet kan lezen,
    tonen een grijs afspeelpictogram. Ingesloten albumhoezen worden nog steeds
    getoond als Vivace ze kan lezen.
- **OSD** — de weergave op het scherm boven de video:
  - het detailniveau (alleen ondertitels; volume + zoeken; volume + zoeken +
    timer) wordt uitgebreid; de OSD-duur en lettergrootte staan bij
    *Voorkeuren ▸ Interface ▸ Tekst*.
  - Foutmeldingen (een mislukte download, een map zonder dvd-video, een
    onleesbaar snelkoppelingsbestand, enz.) worden altijd 20 seconden lang
    getoond, ongeacht de ingestelde OSD-duur — lang genoeg om ook een langere
    foutmelding te lezen.
