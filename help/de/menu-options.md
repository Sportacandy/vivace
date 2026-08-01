# Optionen

Das Menü **Optionen** enthält Einstellungen und die Oberflächenkonfiguration.

- **Einstellungen…** (`Strg+P`) — der Hauptdialog für Einstellungen: Allgemein,
  Oberfläche, Untertitel, Tastatur und Maus, Wiedergabeliste, Laufwerke, TV,
  Dateitypen, Updates, Netzwerk und Erweitert. Änderungen werden sofort wirksam;
  **Abbrechen** setzt sie zurück.
  - **Netzwerk** enthält die Reiter OpenSubtitles, YouTube, Proxy und
    Übertragung; **Proxy** richtet einen optionalen HTTP- oder SOCKS5-Proxy
    ein, der anwendungsweit gilt (OpenSubtitles-Suche, Update-Prüfung sowie
    — nur bei HTTP — Medienwiedergabe und yt-dlp); **Übertragung** legt den
    festen Port fest, auf dem *Wiedergabe ▸ Übertragen zum ▸ Smartphone/
    Tablet* wartet. Das OpenSubtitles-Kontopasswort und das Proxy-Passwort
    werden sicher in der Anmeldeinformationsverwaltung Ihres Betriebssystems
    gespeichert, nicht in den eigenen Einstellungen von Vivace.
- **Symbol im Systemtray anzeigen** — Vivace über den Infobereich erreichbar halten.
- **Werkzeugleisten**
  - **Werkzeugleiste** / **Steuerleiste** — jede Leiste ein- oder ausblenden.
  - **Haupt-Werkzeugleiste bearbeiten…** / **Steuerleiste bearbeiten…** —
    festlegen, welche Schaltflächen erscheinen, sowie deren Reihenfolge und Symbolgröße.
- **Statusleiste**
  - **Statusleiste anzeigen** und was sie anzeigt: **Videoinfo**, **Audioinfo**,
    **Formatinfo**, **Bitrateninfo**, **Bildzähler**, **Gesamtzeit anzeigen**,
    **Restzeit anzeigen** und **Aktuelle Zeit mit Millisekunden anzeigen**.

**Tipp:** Das Gesamtlayout (Basic / Mini / MPC) wählen Sie unter
*Einstellungen ▸ Oberfläche*.

## Installation und Aktualisierung von yt-dlp

*Einstellungen ▸ Netzwerk ▸ YouTube* enthält das Kontrollkästchen
**Verwaltetes yt-dlp verwenden**, das steuert, wie Vivace das Programm
`yt-dlp` bezieht und aktuell hält, das zum Abspielen von YouTube-Links
verwendet wird:

- **Aktiviert** (Standard) — Vivace kann `yt-dlp` für Sie installieren und
  aktuell halten. Das Feld **yt-dlp-Pfad:** ist fest auf Vivaces eigene Kopie
  gesetzt und kann nicht direkt bearbeitet werden; verwenden Sie die
  Schaltfläche **yt-dlp installieren/aktualisieren…** (neben dem
  Kontrollkästchen), wann immer Sie die neueste offizielle Version abrufen
  möchten. Die Einstellung **yt-dlp automatisch aktualisieren:** wird
  ebenfalls verfügbar und lässt Vivace diese Aktualisierung selbstständig
  durchführen — **Niemals**, davor **Bei jeder Ausführung von yt-dlp**, oder
  einmal **täglich**/**wöchentlich**. Eine automatische Aktualisierung läuft
  unmittelbar bevor eine YouTube-URL tatsächlich aufgelöst oder
  heruntergeladen wird, sodass die erste Wiedergabe, nachdem sie fällig
  wird, etwas länger dauert; schlägt die Aktualisierung selbst fehl (z. B.
  keine Netzwerkverbindung), fährt Vivace stillschweigend mit der bereits
  installierten Version fort, statt die Wiedergabe zu blockieren.
- **Deaktiviert** — für ein yt-dlp, das Sie selbst verwalten (z. B.
  installiert über `pip` oder den Paketmanager Ihres Betriebssystems). Das
  Feld **yt-dlp-Pfad:** wird bearbeitbar, sodass Sie es auf diese Kopie
  verweisen lassen können, und **yt-dlp automatisch aktualisieren** ist
  deaktiviert — Vivace installiert oder aktualisiert nie ein yt-dlp, das es
  nicht selbst verwaltet. Auch die Schaltfläche **yt-dlp
  installieren/aktualisieren…** ist in diesem Modus deaktiviert.

## Cookies für YouTube-Downloads exportieren

Das Feld **Cookie-Datei:** (*Einstellungen ▸ Netzwerk ▸ YouTube*) lässt die
YouTube-Modi **Herunterladen & abspielen** und **Externes Werkzeug** so
funktionieren, als wären Sie angemeldet — nötig für altersbeschränkte,
mitgliederexklusive oder anderweitig kontogebundene Videos, und genau das
schaltet auch vollständige HD-/4K-Downloads frei. Erwartet wird eine
Klartextdatei `cookies.txt` im klassischen Netscape-Cookie-Format (demselben
Format, das auch yt-dlps eigene Option `--cookies` einliest); Vivace liest
Cookies nicht direkt aus dem Profil eines Browsers.

**So erstellen Sie eine:**

1. Melden Sie sich in Ihrem gewohnten Browser bei youtube.com an, mit dem
   Konto, dessen Zugriff Sie nutzen möchten.
2. Installieren Sie eine Browser-Erweiterung zum Cookie-Export, die das
   Netscape-Format schreibt — suchen Sie für Chrome, Edge oder Brave im
   Erweiterungs-Store Ihres Browsers nach so etwas wie „Get cookies.txt“;
   für Firefox suchen Sie nach „cookies.txt“. Jede Erweiterung, die
   ausdrücklich angibt, das klassische Netscape-Format `cookies.txt` zu
   exportieren, funktioniert.
3. Exportieren Sie bei geöffnetem youtube.com-Tab mit der Erweiterung die
   Cookies für diese Seite und speichern Sie das Ergebnis irgendwo auf der
   Festplatte als `.txt`-Datei.
4. Öffnen Sie in Vivace *Einstellungen ▸ Netzwerk ▸ YouTube* und wählen Sie
   mit **Durchsuchen…** neben **Cookie-Datei:** diese Datei aus.

**Zu beachten:**

- Eine `cookies.txt`-Datei ist praktisch eine gespeicherte Anmeldesitzung —
  wer immer die Datei besitzt, kann sich bis zum Ablauf der Cookies oder bis
  Sie sich abmelden als Ihr YouTube-Konto ausgeben. Bewahren Sie sie an
  einem privaten Ort auf und geben Sie sie nicht weiter.
- Cookies werden nur vom **Download**-Pfad verwendet (Herunterladen &
  abspielen / Externes Werkzeug). Vivace sendet im **Streaming**-Modus
  bewusst nie Cookies — eine angemeldete Stream-URL ist so eng an diese
  Sitzung gebunden, dass Vivaces einfacher Videoplayer sie nicht öffnen
  kann; das Streaming bleibt daher auch bei konfigurierter Cookie-Datei
  anonym.
- Cookies laufen ab. Wenn zuvor funktionierende Downloads plötzlich
  fehlschlagen oder auf ein Ergebnis mit geringerer Qualität bzw. eine
  öffentliche Version zurückfallen, exportieren Sie eine neue `cookies.txt`.

## Installation von Deno für YouTube-Downloads

yt-dlp selbst — nicht nur Vivace — verwendet eine separate externe
JavaScript-Laufzeitumgebung, um die Herausforderungen zu lösen, die YouTube
stellt, bevor es die tatsächliche Download-URL eines Videos herausgibt. Laut
yt-dlps eigener Dokumentation ist der Betrieb ohne eine solche
Laufzeitumgebung „veraltet“, schlägt aber nicht grundsätzlich fehl: Die
Formatverfügbarkeit ist lediglich verringert, und zwar **stark bei einer
angemeldeten (Cookie-)Anfrage** — genau die Art von Anfrage, die der Modus
**Herunterladen und abspielen** stellt, um HD-, mitgliederexklusive und
altersbeschränkte Videos freizuschalten. Der **Streaming**-Modus sendet nie
Cookies (siehe „Cookies für YouTube-Downloads exportieren“ oben), daher ist
er nicht der kritische Fall und funktioniert in den meisten Fällen auch ohne
Deno einwandfrei. Deshalb befindet sich das Feld **Deno-Pfad:** unter
*Einstellungen ▸ Netzwerk ▸ YouTube ▸ Herunterladen und abspielen* und nicht
als allgemeine YouTube-Einstellung. yt-dlp unterstützt mehrere
JavaScript-Laufzeitumgebungen; Deno ist diejenige, nach der standardmäßig
gesucht wird.

**So installieren Sie Deno:**

1. Folgen Sie den offiziellen Installationsanweisungen unter
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   für Ihr Betriebssystem (ein Installationsskript oder ein Paketmanager wie
   winget/scoop/Homebrew/apt, je nach Plattform).
2. Stellen Sie sicher, dass die ausführbare Datei `deno` letztlich in Ihrem
   System-PATH liegt — die oben genannten Installationsprogramme erledigen
   das normalerweise für Sie. Achten Sie unter Windows darauf, `deno` zu
   erhalten, nicht `denort` (eine andere, verwandte ausführbare Datei, die
   hier nicht funktioniert).
3. Wenn Sie PATH lieber nicht ändern möchten, lassen Sie es unverändert und
   fügen Sie stattdessen den vollständigen Pfad in **Deno-Pfad:** ein
   (*Einstellungen ▸ Netzwerk ▸ YouTube ▸ Herunterladen und abspielen*).
4. Starten Sie Vivace nach der Installation neu (oder versuchen Sie einfach
   einen Download erneut).

**Zu beachten:**

- Dies ist eine Abhängigkeit von **yt-dlp**, nicht von Vivace direkt — Vivace
  führt yt-dlp lediglich als externen Prozess aus und ruft Deno selbst nie
  auf.
- yt-dlp benötigt eine einigermaßen aktuelle Deno-Version (zum Zeitpunkt der
  Erstellung dieses Textes 2.3.0 oder neuer). Wenn Downloads nach der
  Installation weiterhin eine verringerte Qualität zeigen oder Formatfehler
  auftreten, prüfen Sie `deno --version` und aktualisieren Sie Deno, falls es
  älter ist.
- Diese Anforderung ergibt sich aus Änderungen auf Seiten von YouTube/yt-dlp,
  nicht von Vivace — genau aus diesem Grund existiert das Feld
  **Deno-Pfad:** und benötigt keine weitere Konfiguration, sobald Deno
  selbst installiert und erreichbar ist.
