# Vis

Menuen **Vis** viser oplysninger og sekundære paneler.

- **Oplysninger og egenskaber…** — en detaljeret dialog om det aktuelle medie:
  generel info, demuxer og video-/lydcodecs (med de formater, Vivaces backend
  understøtter, fremhævet). Den er kun til information.
- **Afspilningsliste** (`F9`) — vis eller skjul afspilningslisten. Afhængigt af
  *Indstillinger ▸ Grænseflade* vises den som et forankret panel eller et separat
  vindue. Se [Åbn](menu-open.md) for at indlæse afspilningslister.
  - Hver række viser en miniature — en tilhørende billedfil ved siden
    af mediet (en "<name>.jpg", der allerede ligger ved siden af det),
    hvis en sådan findes, ellers et billede, der automatisk hentes i
    baggrunden. Knappen øverst til højre i afspilningslisten skifter
    række-/miniaturestørrelsen (**Lille**/**Mellem**/**Stor**); ved
    Lille og Mellem forstørres både den valgte række og den, der
    aktuelt afspilles, for at vise en større forhåndsvisning.
    *Indstillinger ▸ Afspilningsliste ▸ Diverse* har en uafhængig
    indstilling for hver af de to ("Miniature for den valgte post" /
    "Miniature for det aktuelt afspillede"): ingen forstørrelse,
    forstør kun den ene række, eller lad også de nærmeste par rækker
    aftage mod samme størrelse (standard for begge). Genererede
    miniaturer caches (den, der senest er blevet vist mindst for
    nylig, fjernes, når cachen er fuld); cachestørrelsen kan
    konfigureres under *Indstillinger ▸ Afspilningsliste ▸ Diverse ▸
    Maks. antal cachelagrede miniaturer*.
- **OSD** — skærmvisningen over videoen:
  - detaljeringsgraden (kun undertekster; lydstyrke + søgning; lydstyrke +
    søgning + timer) udvides; OSD-varighed og skriftstørrelse findes i
    *Indstillinger ▸ Grænseflade ▸ Tekst*.
  - Fejlmeddelelser (en mislykket download, en mappe uden dvd-video, en
    ulæselig genvejsfil osv.) vises altid i faste 20 sekunder, uanset den
    indstillede OSD-varighed — længe nok til at læse selv en længere
    fejlmeddelelse.
