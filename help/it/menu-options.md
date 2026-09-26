# Opzioni

Il menu **Opzioni** contiene le preferenze e la configurazione dell'interfaccia.

- **Preferenze…** (`Ctrl+P`) — la finestra principale delle impostazioni:
  Generale, Interfaccia, Sottotitoli, Tastiera e mouse, Playlist, Unità, TV, Tipi
  di file, Aggiornamenti, Rete e Avanzate. Le modifiche si applicano subito;
  **Annulla** le ripristina.
  - **Generale ▸ Video** imposta la modalità **Deinterlaccia** predefinita
    (Nessuno / Yadif / Bwdif / Automatico) per i file aperti di recente —
    modificabile per singolo file da *Video ▸ Deinterlaccia* (solo Nessuno/
    Yadif/Bwdif; Automatico non è disponibile lì, poiché ha senso solo
    come valore predefinito). **Automatico** usa Bwdif, ma solo sui
    fotogrammi che il file stesso contrassegna come interlacciati,
    lasciando invariati i fotogrammi progressivi.
  - **Generale ▸ Audio e sottotitoli preferiti** imposta le lingue tra
    cui Vivace sceglie automaticamente tra le tracce incorporate di un
    file. **Mostra i sottotitoli per impostazione predefinita** attiva
    automaticamente un sottotitolo quando disponibile; la sua
    sotto-opzione **...ma non se l'audio è già in una lingua preferita**
    salta questo sottotitolo automatico quando la sua lingua corrisponde
    alla traccia audio effettivamente selezionata — utile se capisci già
    l'audio e non vuoi che un sottotitolo nella stessa lingua distragga.
  - **Rete** include le schede OpenSubtitles, YouTube, Proxy e Cast; **Proxy**
    configura un proxy HTTP o SOCKS5 facoltativo, applicato a tutta
    l'applicazione (ricerca OpenSubtitles, controllo aggiornamenti e, solo per
    HTTP, riproduzione e yt-dlp); **Cast** fissa la porta su cui è in ascolto
    *Riproduci ▸ Cast su ▸ Smartphone/tablet*. La password dell'account
    OpenSubtitles e la password del proxy vengono ora salvate in modo sicuro
    nel gestore delle credenziali del sistema operativo, non nelle
    impostazioni proprie di Vivace.
- **Mostra icona nella barra delle applicazioni** — tenere Vivace raggiungibile
  dall'area di notifica.
- **Barre degli strumenti**
  - **Barra degli strumenti** / **Barra dei controlli** — mostrare o nascondere ciascuna barra.
  - **Modifica barra degli strumenti principale…** / **Modifica barra dei
    controlli…** — scegliere quali pulsanti compaiono, il loro ordine e la dimensione delle icone.
- **Barra di stato**
  - **Mostra barra di stato** e ciò che mostra: **Info video**, **Info audio**,
    **Info formato**, **Info bitrate**, **Contatore fotogrammi**, **Mostra tempo
    totale**, **Mostra tempo rimanente** e **Mostra l'ora corrente con i millisecondi**.

**Suggerimento:** la disposizione generale (Basic / Mini / MPC) si sceglie in
*Preferenze ▸ Interfaccia*.

## Installazione e aggiornamento di yt-dlp

*Preferenze ▸ Rete ▸ YouTube* include la casella di controllo **Usa yt-dlp
gestito**, che determina come Vivace ottiene e mantiene il programma
`yt-dlp` usato per riprodurre i link di YouTube:

- **Attiva** (l'impostazione predefinita) — Vivace può installare `yt-dlp`
  per te e mantenerlo aggiornato. Il campo **Percorso di yt-dlp:** è fissato
  sulla copia propria di Vivace e non può essere modificato direttamente;
  usa il pulsante **Installa / aggiorna yt-dlp…** (accanto alla casella)
  ogni volta che vuoi scaricare l'ultima versione ufficiale. Diventa
  disponibile anche l'impostazione **Aggiorna yt-dlp automaticamente:**,
  che permette a Vivace di eseguire questo aggiornamento da solo — **Mai**,
  oppure **Ogni volta che yt-dlp viene eseguito**, o una volta al
  **giorno** o alla **settimana**. Un aggiornamento automatico viene
  eseguito subito prima che un URL di YouTube venga effettivamente risolto
  o scaricato, quindi la prima riproduzione dopo che diventa dovuto
  richiede un po' più tempo; se l'aggiornamento stesso fallisce (ad esempio
  per mancanza di rete), Vivace prosegue silenziosamente con qualunque
  versione sia già installata, invece di bloccare la riproduzione.
- **Disattiva** — per un yt-dlp che gestisci tu stesso (ad esempio
  installato tramite `pip` o il gestore pacchetti del tuo sistema
  operativo). Il campo **Percorso di yt-dlp:** diventa modificabile, così
  puoi farlo puntare a quella copia, e **Aggiorna yt-dlp automaticamente** è
  disattivato — Vivace non installa né aggiorna mai uno yt-dlp che non
  gestisce. Anche il pulsante **Installa / aggiorna yt-dlp…** è
  disattivato in questa modalità.

## Installazione del provider di token PO di YouTube

I video recenti di YouTube richiedono sempre più spesso un **token PO
(proof-of-origin)** solo per poter essere riprodotti — un requisito
generale di riproducibilità, indipendente dai cookie o dall'accesso
effettuato. Senza di esso, yt-dlp segnala il video come non disponibile
anche per una richiesta normale e anonima. Questo vale per **entrambe** le
modalità, Streaming e Scarica e riproduci, a differenza dei cookie (solo
per il download, vedi sotto) o di Deno (riguarda soprattutto Scarica e
riproduci, vedi più avanti).

*Preferenze ▸ Rete ▸ YouTube* include un pulsante **Installa il provider di
token PO…** che configura il progetto della comunità **"BgUtils POT
Provider"**: un piccolo plugin di yt-dlp più uno script — eseguito su
richiesta tramite Deno, lo stesso programma trattato più avanti — che
consente a yt-dlp di generare un token automaticamente. Fai clic su di
esso, attendi che il breve processo di download e compilazione finisca, ed
è fatta; l'etichetta del pulsante diventa **Reinstalla / aggiorna il
provider di token PO…** una volta installato, nel caso in cui in seguito
sia necessario un aggiornamento.

**Da tenere presente:**

- Questo richiede un'installazione funzionante di Deno (vedi "Installazione
  di Deno per i download da YouTube" più sotto) — il pulsante scarica e
  compila lo script proprio del provider, che yt-dlp esegue poi tramite
  Deno ogni volta che un video ha effettivamente bisogno di un token.
- Non tutti i video richiedono un token PO, quindi la riproduzione può
  funzionare bene anche senza questa installazione — ma se un video si
  segnala come non disponibile mentre altrove viene riprodotto senza
  problemi, vale la pena installarlo.
- **Su Android,** questo è già incluso in Vivace e configurato
  automaticamente — non c'è nulla da installare e nessun pulsante
  equivalente; funziona e basta.

## Cookie per i download da YouTube

Le modalità YouTube **Scarica e riproduci** e **Strumento esterno** possono
comportarsi come se si avesse effettuato l'accesso — necessario per i video
con restrizioni di età, riservati agli iscritti o altrimenti vincolati a un
account, ed è ciò che sblocca anche i download in piena qualità HD/4K.
Vivace supporta tre modi per fornire i cookie (tutti in *Preferenze ▸ Rete
▸ YouTube*): **Recupero cookie dal browser** su Windows/Linux/macOS,
**Accedi a YouTube…** su Android, ed esportare manualmente un file
`cookies.txt` come soluzione alternativa su qualsiasi piattaforma.

### Recupero cookie dal browser (consigliato su Windows/Linux/macOS)

Il menu a tendina **Recupero cookie dal browser:** elenca Firefox, Chrome,
Edge, Brave, Chromium, Opera, Safari, Vivaldi e Whale. Scegli il tuo
browser e Vivace ne legge i cookie in tempo reale, ogni volta — niente da
esportare, niente che diventi obsoleto.

**Da tenere presente:**

- YouTube ha ridotto in modo significativo la durata dei propri cookie,
  quindi un file `cookies.txt` esportato in precedenza (vedi sotto) può
  diventare obsoleto nel giro di pochi giorni — questa opzione evita del
  tutto il problema leggendo l'archivio cookie del browser stesso, sempre
  aggiornato.
- **Su Windows funziona davvero solo con Firefox.** Chrome, Edge e altri
  browser basati su Chromium su Windows cifrano i propri cookie in un modo
  legato all'eseguibile stesso del browser ("App-Bound Encryption", Chrome
  127+) — questo impedisce del tutto a yt-dlp, e a qualsiasi altro
  strumento esterno, di leggerli. Si tratta di una restrizione imposta da
  Chrome; nemmeno gli sviluppatori di yt-dlp possono aggirarla. Chrome/Edge
  su Linux e macOS non sono interessati e funzionano normalmente.
- Selezionare un browser qui ha la priorità sul campo **File dei cookie:**
  più sotto, quando entrambi sono impostati.
- **Non funziona affatto su Android, e non funziona nemmeno con Chrome/Edge
  su Windows** — per entrambi i casi vedi **Accedi a YouTube** più sotto.

### Accedi a YouTube (Android, e soluzione alternativa per Chrome/Edge su Windows)

*Preferenze ▸ Rete ▸ YouTube* ha un pulsante **Accedi a YouTube…**
(accanto al campo del file dei cookie) che apre una vera pagina di accesso
direttamente dentro Vivace — tramite una WebView incorporata e nativa del
sistema operativo su Android, oppure tramite una vista basata su Chromium
inclusa con Vivace (QtWebEngine) su Windows/Linux/macOS. Accedi con
l'account di cui vuoi usare l'accesso, poi tocca **Salva i cookie** —
Vivace legge i cookie di sessione risultanti e li salva automaticamente
come file dei cookie attivo. Tocca **Chiudi** quando hai finito. Su
Android questo è l'unico modo per ottenere cookie aggiornati; sul desktop
serve principalmente come soluzione alternativa per Chrome/Edge su
Windows, dove **Recupero cookie dal browser** sopra non riesce a leggerne
i cookie — in entrambi i casi non c'è nulla da esportare né da
trasferire manualmente da un altro dispositivo.

**Da tenere presente:**

- Android non ha alcun equivalente di un archivio cookie del browser
  desktop leggibile in tempo reale (l'archiviazione di ogni app è isolata
  da quella di tutte le altre) — per questo qui funziona diversamente
  dall'opzione desktop: accedere *dentro* al browser incorporato di Vivace
  è il sostituto pratico.
- Sul desktop si tratta di una vera sessione del browser separata,
  controllata direttamente da Vivace — una funzione decisamente più
  impegnativa rispetto a **Recupero cookie dal browser** (include un
  proprio motore del browser), quindi conviene usarla soprattutto dove
  quell'opzione più semplice non funziona, cioè con Chrome/Edge su
  Windows.

### Esportare i cookie in un file (soluzione alternativa su qualsiasi piattaforma)

Il campo **File dei cookie:** richiede un file di testo semplice
`cookies.txt` nel classico formato dei cookie Netscape (lo stesso formato
letto dall'opzione `--cookies` di yt-dlp) — usalo quando nessuna delle due
opzioni in tempo reale sopra è disponibile o funziona per te (ad es. se
**Accedi a YouTube** non è disponibile nella tua versione di Vivace).

**Per crearne uno:**

1. Accedi a youtube.com nel tuo browser abituale, con l'account di cui
   vuoi usare l'accesso.
2. Installa un'estensione del browser per l'esportazione dei cookie che
   scriva nel formato Netscape — per Chrome, Edge o Brave, cerca nello
   store delle estensioni del tuo browser qualcosa come "Get cookies.txt";
   per Firefox, cerca "cookies.txt". Qualsiasi estensione che dichiari
   chiaramente di esportare nel classico formato Netscape `cookies.txt`
   andrà bene.
3. Con youtube.com aperto in una scheda, usa l'estensione per esportare i
   cookie di quel sito e salva il risultato da qualche parte sul disco come
   file `.txt`.
4. In Vivace, apri *Preferenze ▸ Rete ▸ YouTube* e usa **Sfoglia…** accanto
   a **File dei cookie:** per selezionare quel file.

**Su Android:** preferisci invece **Accedi a YouTube…** sopra — non
richiede alcun passaggio di esportazione/trasferimento. Se vuoi comunque
procedere in questo modo, tieni presente che Chrome per Android non
supporta le estensioni del browser, quindi i passaggi 2–3 sopra non
possono essere eseguiti sul dispositivo stesso. Esporta `cookies.txt` su
un computer desktop o portatile come
descritto sopra, quindi trasferisci quel file sul tuo dispositivo Android
(ad es. tramite archiviazione cloud, un cavo USB o e-mail) prima di usare
**Sfoglia…** al passaggio 4.

**Da tenere presente:**

- Un file `cookies.txt` equivale di fatto a una sessione di accesso
  salvata — chiunque abbia il file può agire come il tuo account YouTube
  finché i cookie non scadono o non effettui il logout. Conservalo in un
  luogo privato e non condividerlo.
- I cookie scadono. Se i download che prima funzionavano iniziano a
  fallire, o si ricade su un risultato pubblico o di qualità inferiore,
  esporta un nuovo `cookies.txt` — oppure passa a **Recupero cookie dal
  browser** sopra, se disponibile, per evitare del tutto il problema.

**Si applica a entrambi i metodi sopra:**

- I cookie vengono usati solo dal percorso di **download** (Scarica e
  riproduci / Strumento esterno). Vivace non invia mai deliberatamente i
  cookie in modalità **streaming** — un URL di streaming associato
  all'accesso è legato a quella sessione in un modo che il semplice
  lettore video di Vivace non può aprire, quindi lo streaming resta
  anonimo indipendentemente da come sono configurati i cookie.

## Installazione di ffmpeg per i download da YouTube

La modalità **Scarica e riproduci** richiede `ffmpeg` per unire i flussi
video e audio separati scaricati da yt-dlp in un unico file riproducibile —
YouTube offre raramente l'HD come flusso combinato unico, quindi una
traccia video e una traccia audio vengono scaricate separatamente e poi
unite. Il campo **Posizione di ffmpeg:** (*Preferenze ▸ Rete ▸ YouTube ▸
Scarica e riproduci*) indica a yt-dlp dove trovarlo; lascialo vuoto per
usare invece `ffmpeg` dal PATH di sistema.

**Per installare ffmpeg:**

1. **Windows** — l'opzione più semplice è un gestore pacchetti:
   `winget install ffmpeg` (oppure `scoop install ffmpeg` / `choco install
   ffmpeg`). In alternativa, scarica un archivio precompilato da
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) o
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) ed
   estrailo da qualche parte.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — installalo tramite il gestore pacchetti della tua
   distribuzione, ad es. `sudo apt install ffmpeg` (Debian/Ubuntu), `sudo
   dnf install ffmpeg` (Fedora), oppure `sudo pacman -S ffmpeg` (Arch).
4. Se hai aggiunto ffmpeg al PATH di sistema, lascia **Posizione di
   ffmpeg:** vuoto. Altrimenti, incolla in quel campo il percorso della
   *cartella* che contiene l'eseguibile `ffmpeg` (non l'eseguibile stesso).
5. Riavvia Vivace (o riprova semplicemente un download) dopo
   l'installazione.

**Da tenere presente:**

- Questa è una dipendenza di **yt-dlp**, come Deno più sotto — Vivace la
  esegue solo come processo esterno.
- La modalità **Streaming** non ha mai bisogno di ffmpeg, poiché riproduce
  un unico flusso già combinato; solo **Scarica e riproduci** ne ha
  bisogno, perché quella modalità recupera video e audio separatamente e
  li unisce localmente.
- Se un download fallisce con un errore legato all'unione, controlla prima
  la posizione di ffmpeg — è la causa più comune, oltre a un Deno mancante
  o obsoleto.

## Installazione di Deno per i download da YouTube

Lo stesso yt-dlp — non solo Vivace — utilizza un runtime JavaScript esterno
separato per risolvere le sfide poste da YouTube prima di fornire l'URL di
download reale di un video. Secondo la documentazione ufficiale di yt-dlp,
funzionare senza uno di questi runtime è "deprecato" ma non comporta un
fallimento immediato: la disponibilità dei formati è semplicemente ridotta,
e **in modo severo per una richiesta con accesso effettuato (cookie)** —
esattamente il tipo di richiesta che effettua la modalità **Scarica e
riproduci** per sbloccare video in HD, riservati agli iscritti e con
restrizioni di età. La modalità **Streaming** non invia mai cookie (vedi
«Cookie per i download da YouTube» sopra), quindi non è il caso
critico e funziona bene senza Deno nella maggior parte dei casi. Per
questo il campo **Percorso di Deno:** si trova in *Preferenze ▸ Rete ▸
YouTube ▸ Scarica e riproduci*, e non come impostazione generale di
YouTube. yt-dlp supporta diversi runtime JavaScript; Deno è quello cercato
per impostazione predefinita.

**Per installare Deno:**

1. Segui le istruzioni di installazione ufficiali su
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   per il tuo sistema operativo (uno script di installazione, oppure un
   gestore pacchetti come winget/scoop/Homebrew/apt, a seconda della
   piattaforma).
2. Assicurati che l'eseguibile `deno` finisca nel PATH di sistema — gli
   installer indicati sopra normalmente lo fanno per te. Su Windows,
   assicurati di ottenere `deno`, non `denort` (un eseguibile diverso,
   correlato, che qui non funziona).
3. Se preferisci non modificare il PATH, lascialo com'è e incolla invece il
   percorso completo in **Percorso di Deno:** (*Preferenze ▸ Rete ▸
   YouTube ▸ Scarica e riproduci*).
4. Riavvia Vivace (o riprova semplicemente un download) dopo
   l'installazione.

**Da tenere presente:**

- Questa è una dipendenza di **yt-dlp**, non di Vivace direttamente —
  Vivace si limita a eseguire yt-dlp come processo esterno e non invoca mai
  Deno direttamente.
- yt-dlp richiede una versione di Deno ragionevolmente recente (2.3.0 o
  successiva al momento della stesura). Se i download continuano a
  mostrare qualità ridotta o errori di formato dopo l'installazione,
  controlla `deno --version` e aggiornalo se è più vecchio.
- Questo requisito deriva da cambiamenti lato YouTube/yt-dlp, non da
  Vivace — lo stesso campo **Percorso di Deno:** esiste proprio per questo
  motivo e non richiede ulteriore configurazione una volta che Deno stesso
  è installato e raggiungibile.

## Levigatura dei sottotitoli bitmap

*Preferenze ▸ Sottotitoli ▸ Sottotitoli bitmap* include un'impostazione
**Levigatura:** (0–3, valore predefinito 1) per i sottotitoli
visualizzati come immagini anziché come testo: tracce di sottotitoli
DVD, PGS e DVB. Questo copre sia i sottotitoli propri di un vero disco
DVD sia una traccia di sottotitoli incorporata dello stesso tipo in un
file video normale (ad esempio un file .mp4 con una traccia con codec
`dvd_subtitle`). Questi formati sono immagini bitmap pre-renderizzate,
incise alla risoluzione nativa in definizione standard (SD) al momento
della creazione della sorgente — i loro bordi possono apparire
frastagliati una volta ingranditi alla dimensione di una finestra
moderna. Vivace può applicare una leggera sfocatura per ammorbidire
questi bordi:

- **0** — disattivato; mostra la bitmap dei sottotitoli originale
  esattamente come creata.
- **1** (predefinito) — ammorbidisce i bordi più marcati mantenendo il
  testo a una luminosità praticamente piena.
- **2** / **3** — sfocatura progressivamente maggiore.

Questa impostazione riguarda solo i sottotitoli bitmap — non ha alcun
effetto sul motore di rendering esterno dei sottotitoli di Vivace
(SRT/VTT/ASS) né sulle normali tracce di sottotitoli testuali, che
seguono entrambe percorsi di rendering diversi.
