# Visa

Menyn **Visa** visar information och sekundära paneler.

- **Information och egenskaper…** — en detaljerad dialogruta om det aktuella
  mediet: allmän info, demuxer och video-/ljudkodekar (med de format som Vivaces
  backend stöder markerade). Den är endast informativ.
- **Spellista** (`F9`) — visa eller dölj spellistan. Beroende på
  *Inställningar ▸ Gränssnitt* visas den som en dockad panel eller ett separat
  fönster. Se [Öppna](menu-open.md) för att ladda spellistor.
  - Varje rad visar en miniatyrbild — en bifogad bildfil bredvid mediet
    (en "<name>.jpg" som redan ligger bredvid det) om en sådan finns,
    annars en bildruta som hämtas automatiskt i bakgrunden. Knappen
    längst upp till höger i spellistan växlar rad-/miniatyrstorleken
    (**Liten**/**Medel**/**Stor**); vid Liten och Medel förstoras både
    den markerade raden och raden för det som spelas, så att en större
    förhandsvisning visas. *Inställningar ▸ Spellista ▸ Misc* har en
    oberoende inställning för var och en av de två ("Miniatyr för det
    markerade objektet" / "Miniatyr för det som spelas"): ingen
    förstoring, förstora bara den raden, eller låt även de närmaste
    raderna avta mot samma storlek (standard för båda). Genererade
    miniatyrer cachas (den som visats minst nyligen tas bort när cachen
    är full); cachestorleken kan ställas in under *Inställningar ▸
    Spellista ▸ Misc ▸ Maximalt antal cachade miniatyrer*.
- **OSD** — skärmvisningen över videon:
  - detaljnivån (endast undertexter; volym + spolning; volym + spolning + timer)
    utökas; OSD-varaktighet och teckenstorlek finns i
    *Inställningar ▸ Gränssnitt ▸ Text*.
  - Felmeddelanden (en misslyckad nedladdning, en mapp utan DVD-video, en
    oläsbar genvägsfil osv.) visas alltid i fasta 20 sekunder, oavsett den
    inställda OSD-varaktigheten — tillräckligt länge för att läsa även ett
    längre felmeddelande.
