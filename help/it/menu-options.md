# Opzioni

Il menu **Opzioni** contiene le preferenze e la configurazione dell'interfaccia.

- **Preferenze…** (`Ctrl+P`) — la finestra principale delle impostazioni:
  Generale, Interfaccia, Sottotitoli, Tastiera e mouse, Playlist, Unità, TV, Tipi
  di file, Aggiornamenti, Rete e Avanzate. Le modifiche si applicano subito;
  **Annulla** le ripristina.
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

## Esportare i cookie per i download da YouTube

Il campo **File dei cookie:** (*Preferenze ▸ Rete ▸ YouTube*) permette alle
modalità YouTube **Scarica e riproduci** e **Strumento esterno** di
comportarsi come se si avesse effettuato l'accesso — necessario per i video
con restrizioni di età, riservati agli iscritti o altrimenti vincolati a un
account, ed è ciò che sblocca anche i download in piena qualità HD/4K.
Richiede un file di testo semplice `cookies.txt` nel classico formato dei
cookie Netscape (lo stesso formato letto dall'opzione `--cookies` di
yt-dlp); Vivace non legge i cookie direttamente dal profilo di un browser.

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

**Da tenere presente:**

- Un file `cookies.txt` equivale di fatto a una sessione di accesso
  salvata — chiunque abbia il file può agire come il tuo account YouTube
  finché i cookie non scadono o non effettui il logout. Conservalo in un
  luogo privato e non condividerlo.
- I cookie vengono usati solo dal percorso di **download** (Scarica e
  riproduci / Strumento esterno). Vivace non invia mai deliberatamente i
  cookie in modalità **streaming** — un URL di streaming associato
  all'accesso è legato a quella sessione in un modo che il semplice
  lettore video di Vivace non può aprire, quindi lo streaming resta
  anonimo anche se è configurato un file di cookie.
- I cookie scadono. Se i download che prima funzionavano iniziano a
  fallire, o si ricade su un risultato pubblico o di qualità inferiore,
  esporta un nuovo `cookies.txt`.
