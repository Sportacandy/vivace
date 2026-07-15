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
