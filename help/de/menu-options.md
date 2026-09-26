# Optionen

Das Menü **Optionen** enthält Einstellungen und die Oberflächenkonfiguration.

- **Einstellungen…** (`Strg+P`) — der Hauptdialog für Einstellungen: Allgemein,
  Oberfläche, Untertitel, Tastatur und Maus, Wiedergabeliste, Laufwerke, TV,
  Dateitypen, Updates, Netzwerk und Erweitert. Änderungen werden sofort wirksam;
  **Abbrechen** setzt sie zurück.
  - **Allgemein ▸ Video** legt den Standard-Modus für **Zeilenentflechtung**
    (- / Yadif / Bwdif / Automatisch) für neu geöffnete Dateien fest — pro
    Datei änderbar unter *Video ▸ Zeilenentflechtung* (dort nur - / Yadif /
    Bwdif; Automatisch wird dort nicht angeboten, da es nur als Standardwert
    sinnvoll ist). **Automatisch** verwendet Bwdif, aber nur bei Bildern,
    die die Datei selbst als interlaced markiert; progressive Bilder
    bleiben unverändert.
  - **Allgemein ▸ Bevorzugter Ton und Untertitel** legt die Sprachen fest,
    unter denen Vivace automatisch aus den eingebetteten Spuren einer Datei
    auswählt. **Untertitel standardmäßig anzeigen** schaltet automatisch
    einen Untertitel ein, wenn verfügbar; die Unteroption **...aber nicht,
    wenn die Audiospur bereits in einer bevorzugten Sprache ist**
    überspringt diesen automatischen Untertitel, wenn seine Sprache mit der
    tatsächlich ausgewählten Audiospur übereinstimmt — nützlich, wenn Sie
    die Audiospur bereits verstehen und keinen ablenkenden Untertitel in
    derselben Sprache möchten.
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

## Installation des YouTube-PO-Token-Anbieters

Neuere YouTube-Videos benötigen zunehmend einen **PO-Token
(Proof-of-Origin-Token)**, nur um überhaupt abgespielt zu werden — eine
allgemeine Voraussetzung für die Wiedergabe, unabhängig von Cookies oder
einer Anmeldung. Ohne ihn meldet yt-dlp das Video selbst bei einer
gewöhnlichen, anonymen Anfrage als nicht verfügbar. Dies gilt für **beide**
Modi, Streaming und Herunterladen und abspielen — anders als Cookies (nur
beim Download, siehe unten) oder Deno (betrifft vor allem Herunterladen und
abspielen, siehe weiter unten).

*Einstellungen ▸ Netzwerk ▸ YouTube* enthält eine Schaltfläche **PO-Token-
Anbieter installieren…**, die das Community-Projekt **„BgUtils POT
Provider“** einrichtet: ein kleines yt-dlp-Plugin plus ein Skript — bei
Bedarf über Deno ausgeführt, dasselbe Programm, das weiter unten beschrieben
wird —, mit dem yt-dlp automatisch einen Token erzeugen kann. Klicken Sie
darauf, warten Sie, bis der kurze Download- und Erstellungsvorgang
abgeschlossen ist, und fertig; die Beschriftung der Schaltfläche ändert
sich nach der Installation zu **PO-Token-Anbieter neu installieren /
aktualisieren…**, falls später einmal eine Aktualisierung nötig ist.

**Zu beachten:**

- Dies erfordert eine funktionierende Deno-Installation (siehe „Installation
  von Deno für YouTube-Downloads“ unten) — die Schaltfläche lädt das eigene
  Skript des Anbieters herunter und erstellt es, das yt-dlp dann über Deno
  ausführt, sobald ein Video tatsächlich einen Token benötigt.
- Nicht jedes Video benötigt einen PO-Token, sodass die Wiedergabe auch ohne
  diese Installation problemlos funktionieren kann — aber wenn ein Video
  sich selbst als nicht verfügbar meldet, obwohl es anderswo einwandfrei
  abgespielt wird, lohnt sich die Installation.
- **Unter Android** ist dies bereits in Vivace enthalten und wird
  automatisch eingerichtet — es gibt nichts zu installieren und keine
  entsprechende Schaltfläche; es funktioniert einfach.

## Cookies für YouTube-Downloads

Die YouTube-Modi **Herunterladen & abspielen** und **Externes Werkzeug**
können sich so verhalten, als wären Sie angemeldet — nötig für
altersbeschränkte, mitgliederexklusive oder anderweitig kontogebundene
Videos, und genau das schaltet auch vollständige HD-/4K-Downloads frei.
Vivace unterstützt drei Wege, Cookies bereitzustellen (alle unter
*Einstellungen ▸ Netzwerk ▸ YouTube*): **Cookies aus Browser abrufen**
unter Windows/Linux/macOS, **Bei YouTube anmelden…** unter Android, und
das manuelle Exportieren einer `cookies.txt`-Datei als Ausweichlösung auf
jeder Plattform.

### Cookies aus dem Browser abrufen (empfohlen unter Windows/Linux/macOS)

Das Kombinationsfeld **Cookies aus Browser abrufen:** listet Firefox,
Chrome, Edge, Brave, Chromium, Opera, Safari, Vivaldi und Whale auf. Wählen
Sie Ihren Browser aus, und Vivace liest dessen Cookies jedes Mal live aus
— nichts zu exportieren, nichts, das veraltet.

**Zu beachten:**

- YouTube hat die eigene Gültigkeitsdauer von Cookies deutlich verkürzt,
  sodass eine zuvor exportierte `cookies.txt`-Datei (siehe unten) innerhalb
  weniger Tage veralten kann — diese Option umgeht das vollständig, indem
  sie den eigenen, stets aktuellen Cookie-Speicher des Browsers ausliest.
- **Unter Windows funktioniert hier tatsächlich nur Firefox.** Chrome, Edge
  und andere Chromium-basierte Browser unter Windows verschlüsseln ihre
  Cookies auf eine Weise, die an die Binärdatei des Browsers selbst
  gebunden ist („App-Bound Encryption“, Chrome 127+) — dies hindert yt-dlp
  und jedes andere externe Tool vollständig daran, sie zu lesen. Es
  handelt sich um eine Einschränkung seitens Chrome; die Entwickler von
  yt-dlp können sie nicht umgehen. Linux und macOS Chrome/Edge sind davon
  nicht betroffen und funktionieren normal.
- Die Auswahl eines Browsers hier hat Vorrang vor dem Feld **Cookie-Datei:**
  weiter unten, wenn beide eingestellt sind.
- **Funktioniert unter Android überhaupt nicht und auch nicht mit
  Chrome/Edge unter Windows** — siehe für beide Fälle **Bei YouTube
  anmelden** weiter unten.

### Bei YouTube anmelden (Android, und als Ausweichlösung für Chrome/Edge unter Windows)

*Einstellungen ▸ Netzwerk ▸ YouTube* enthält eine Schaltfläche **Bei
YouTube anmelden…** (neben dem Feld für die Cookie-Datei), die eine echte
Anmeldeseite direkt in Vivace selbst öffnet — unter Android über eine
eingebettete, betriebssystemeigene WebView, unter Windows/Linux/macOS über
eine mitgelieferte Chromium-basierte Ansicht (QtWebEngine). Melden Sie
sich mit dem Konto an, dessen Zugriff Sie nutzen möchten, und tippen Sie
dann auf **Cookies speichern** — Vivace liest die entstehenden
Sitzungs-Cookies aus und speichert sie automatisch als aktive
Cookie-Datei. Tippen Sie danach auf **Schließen**. Unter Android ist dies
die einzige Möglichkeit, überhaupt aktuelle Cookies zu erhalten; auf dem
Desktop dient es vor allem als Ausweichlösung für Chrome/Edge unter
Windows, wo **Cookies aus Browser abrufen** oben deren Cookies nicht
lesen kann — in beiden Fällen gibt es nichts zu exportieren und nichts von
Hand von einem anderen Gerät zu übertragen.

**Zu beachten:**

- Android hat überhaupt kein Gegenstück zu einem live auslesbaren
  Desktop-Browser-Cookie-Speicher (der Speicher jeder App ist von dem
  jeder anderen App abgeschottet) — deshalb funktioniert dies anders als
  die Desktop-Option: Sich *innerhalb* von Vivaces eigener eingebetteter
  Browseransicht anzumelden ist der praktische Ersatz dafür.
- Auf dem Desktop handelt es sich um eine echte, separate Browsersitzung,
  die Vivace selbst steuert — eine deutlich aufwendigere Funktion als
  **Cookies aus Browser abrufen** (sie bringt eine eigene Browser-Engine
  mit), daher lohnt sie sich vor allem dort, wo die einfachere Option
  nicht funktioniert, also bei Chrome/Edge unter Windows.

### Cookies in eine Datei exportieren (Ausweichlösung auf jeder Plattform)

Das Feld **Cookie-Datei:** erwartet eine Klartextdatei `cookies.txt` im
klassischen Netscape-Cookie-Format (demselben Format, das auch yt-dlps
eigene Option `--cookies` einliest) — verwenden Sie dies, wenn keine der
beiden Live-Optionen oben verfügbar ist oder für Sie funktioniert (z. B.
wenn **Bei YouTube anmelden** in Ihrer Vivace-Version nicht verfügbar
ist).

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

**Unter Android:** Bevorzugen Sie stattdessen **Bei YouTube anmelden…**
oben — das erfordert keinerlei Export-/Übertragungsschritt. Wenn Sie es
dennoch auf diese Weise tun möchten: Chrome für Android unterstützt keine
Browser-Erweiterungen, sodass die Schritte 2–3 oben nicht auf dem Gerät
selbst ausgeführt werden können. Exportieren Sie `cookies.txt` wie oben
beschrieben auf einem
Desktop- oder Laptop-Computer und übertragen Sie diese Datei anschließend
auf Ihr Android-Gerät (z. B. über einen Cloud-Speicher, ein USB-Kabel oder
per E-Mail), bevor Sie in Schritt 4 **Durchsuchen…** verwenden.

**Zu beachten:**

- Eine `cookies.txt`-Datei ist praktisch eine gespeicherte Anmeldesitzung —
  wer immer die Datei besitzt, kann sich bis zum Ablauf der Cookies oder bis
  Sie sich abmelden als Ihr YouTube-Konto ausgeben. Bewahren Sie sie an
  einem privaten Ort auf und geben Sie sie nicht weiter.
- Cookies laufen ab. Wenn zuvor funktionierende Downloads plötzlich
  fehlschlagen oder auf ein Ergebnis mit geringerer Qualität bzw. eine
  öffentliche Version zurückfallen, exportieren Sie eine neue `cookies.txt`
  — oder wechseln Sie, falls verfügbar, zu **Cookies aus Browser abrufen**
  oben, um dies vollständig zu vermeiden.

**Gilt für beide oben genannten Methoden:**

- Cookies werden nur vom **Download**-Pfad verwendet (Herunterladen &
  abspielen / Externes Werkzeug). Vivace sendet im **Streaming**-Modus
  bewusst nie Cookies — eine angemeldete Stream-URL ist so eng an diese
  Sitzung gebunden, dass Vivaces einfacher Videoplayer sie nicht öffnen
  kann; das Streaming bleibt daher so oder so anonym, auch wenn Cookies
  konfiguriert sind.

## Installation von ffmpeg für YouTube-Downloads

Der Modus **Herunterladen und abspielen** benötigt `ffmpeg`, um die
separaten Video- und Audio-Streams, die yt-dlp herunterlädt, zu einer
abspielbaren Datei zusammenzuführen — YouTube bietet HD selten als
einzelnen kombinierten Stream an, sodass eine Videospur und eine Audiospur
getrennt heruntergeladen und anschließend zusammengeführt werden. Das Feld
**ffmpeg-Speicherort:** (*Einstellungen ▸ Netzwerk ▸ YouTube ▸
Herunterladen und abspielen*) teilt yt-dlp mit, wo es zu finden ist; lassen
Sie es leer, um stattdessen `ffmpeg` aus dem PATH Ihres Systems zu
verwenden.

**So installieren Sie ffmpeg:**

1. **Windows** — die einfachste Möglichkeit ist ein Paketmanager:
   `winget install ffmpeg` (oder `scoop install ffmpeg` / `choco install
   ffmpeg`). Alternativ können Sie ein vorkompiliertes Archiv von
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) oder
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) herunterladen
   und irgendwo entpacken.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — installieren Sie es über den Paketmanager Ihrer
   Distribution, z. B. `sudo apt install ffmpeg` (Debian/Ubuntu), `sudo dnf
   install ffmpeg` (Fedora) oder `sudo pacman -S ffmpeg` (Arch).
4. Wenn Sie ffmpeg zum PATH Ihres Systems hinzugefügt haben, lassen Sie
   **ffmpeg-Speicherort:** leer. Andernfalls fügen Sie den Pfad zu dem
   *Ordner*, der die ausführbare Datei `ffmpeg` enthält (nicht die
   ausführbare Datei selbst), in dieses Feld ein.
5. Starten Sie Vivace nach der Installation neu (oder versuchen Sie einfach
   einen Download erneut).

**Zu beachten:**

- Dies ist eine Abhängigkeit von **yt-dlp**, wie Deno weiter unten — Vivace
  führt es lediglich als externen Prozess aus.
- Der **Streaming**-Modus benötigt nie ffmpeg, da er einen einzelnen bereits
  kombinierten Stream abspielt; nur **Herunterladen und abspielen**
  benötigt es, weil dieser Modus Video und Audio getrennt abruft und lokal
  zusammenführt.
- Wenn ein Download mit einem Fehler bei der Zusammenführung fehlschlägt,
  prüfen Sie zuerst den ffmpeg-Speicherort — das ist neben einem fehlenden
  oder veralteten Deno die häufigste Ursache.

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
Cookies (siehe „Cookies für YouTube-Downloads“ oben), daher ist
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

## Bitmap-Untertitel-Glättung

*Einstellungen ▸ Untertitel ▸ Bitmap-Untertitel* enthält eine Einstellung
**Glättung:** (0–3, Standard 1) für Untertitel, die als Bilder statt als
Text dargestellt werden — DVD-Subpicture-, PGS- und DVB-Spuren. Das gilt
sowohl für die eigenen Untertitel einer echten DVD-Disc als auch für eine
eingebettete Untertitelspur derselben Art in einer gewöhnlichen
Videodatei (z. B. eine .mp4-Datei mit einer Spur im
`dvd_subtitle`-Codec). Diese Formate sind vorgerenderte Bitmap-Bilder,
die bei der Erstellung des Quellmaterials in nativer Standardauflösung
(SD) eingebrannt wurden — ihre Kanten können bei Skalierung auf eine
moderne Fenstergröße gezackt aussehen. Vivace kann eine leichte
Unschärfe anwenden, um diese Kanten zu glätten:

- **0** — aus; zeigt die ursprüngliche Untertitel-Bitmap genau so, wie
  sie erstellt wurde.
- **1** (Standard) — glättet die gröbsten Kanten, während der Text
  nahezu volle Helligkeit behält.
- **2** / **3** — zunehmend stärkere Unschärfe.

Diese Einstellung wirkt sich nur auf bitmap-basierte Untertitel aus —
sie hat keinen Einfluss auf Vivaces eigenen externen
Untertitel-Renderer (SRT/VTT/ASS) oder auf gewöhnliche textbasierte
Untertitelspuren, die beide über andere Rendering-Pfade laufen.
