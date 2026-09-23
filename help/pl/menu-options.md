# Opcje

Menu **Opcje** zawiera ustawienia i konfigurację interfejsu.

- **Ustawienia…** (`Ctrl+P`) — główne okno ustawień: Ogólne, Interfejs, Napisy,
  Klawiatura i mysz, Lista odtwarzania, Napędy, TV, Typy plików, Aktualizacje,
  Sieć i Zaawansowane. Zmiany są stosowane natychmiast; **Anuluj** je cofa.
  - **Ogólne ▸ Wideo** ustawia domyślny tryb **Usuwanie przeplotu** (Brak /
    Yadif / Bwdif / Automatyczny) dla nowo otwieranych plików — zmień go
    dla pojedynczego pliku w *Wideo ▸ Usuwanie przeplotu* (tylko Brak/
    Yadif/Bwdif; tryb Automatyczny nie jest tam dostępny, bo ma sens tylko
    jako wartość domyślna). Tryb **Automatyczny** używa Bwdif, ale tylko
    dla klatek oznaczonych przez sam plik jako z przeplotem, pozostawiając
    klatki progresywne bez zmian.
  - **Ogólne ▸ Preferowana ścieżka dźwiękowa i napisy** ustawia języki,
    spośród których Vivace automatycznie wybiera wśród osadzonych ścieżek
    pliku. **Domyślnie pokazuj napisy** automatycznie włącza napisy, jeśli
    są dostępne; jego podopcja **...ale nie wtedy, gdy dźwięk jest już w
    preferowanym języku** pomija te automatyczne napisy, gdy ich język
    zgadza się z faktycznie wybraną ścieżką dźwiękową — przydatne, jeśli
    już rozumiesz dźwięk i nie chcesz, aby napisy w tym samym języku
    rozpraszały uwagę.
  - **Sieć** zawiera karty OpenSubtitles, YouTube, Proxy i Przesyłanie;
    **Proxy** konfiguruje opcjonalny serwer proxy HTTP lub SOCKS5, stosowany
    w całej aplikacji (wyszukiwanie OpenSubtitles, sprawdzanie aktualizacji
    oraz — tylko dla HTTP — odtwarzanie i yt-dlp); **Przesyłanie** ustala
    port, na którym nasłuchuje *Odtwarzanie ▸ Prześlij do ▸ Smartfon/tablet*.
    Hasło konta OpenSubtitles i hasło pośrednika są teraz bezpiecznie
    przechowywane w menedżerze poświadczeń systemu operacyjnego, a nie we
    własnych ustawieniach Vivace.
- **Pokaż ikonę w zasobniku systemowym** — pozwala korzystać z Vivace z zasobnika.
- **Paski narzędzi**
  - **Pasek narzędzi** / **Pasek sterowania** — pokaż lub ukryj każdy pasek.
  - **Edytuj główny pasek narzędzi…** / **Edytuj pasek sterowania…** — wybierz,
    które przyciski się pojawiają, ich kolejność i rozmiar ikon.
- **Pasek stanu**
  - **Pokaż pasek stanu** i to, co wyświetla: **Informacje o wideo**,
    **Informacje o dźwięku**, **Informacje o formacie**, **Informacje o
    przepływności**, **Licznik klatek**, **Pokaż czas całkowity**, **Pokaż czas
    pozostały** oraz **Pokaż bieżący czas z milisekundami**.

**Wskazówka:** ogólny układ (Basic / Mini / MPC) wybiera się w *Ustawienia ▸ Interfejs*.

## Instalowanie i aktualizowanie yt-dlp

*Ustawienia ▸ Sieć ▸ YouTube* zawiera pole wyboru **Używaj zarządzanego
yt-dlp**, które określa, w jaki sposób Vivace pozyskuje i utrzymuje program
`yt-dlp` używany do odtwarzania linków YouTube:

- **Włączone** (domyślnie) — Vivace może zainstalować `yt-dlp` za Ciebie i
  utrzymywać go w aktualnej wersji. Pole **Ścieżka yt-dlp:** jest wtedy
  ustawione na własną kopię Vivace i nie można go edytować bezpośrednio;
  użyj przycisku **Zainstaluj / zaktualizuj yt-dlp…** (obok pola wyboru),
  gdy chcesz pobrać najnowsze oficjalne wydanie. Dostępne staje się też
  ustawienie **Automatycznie aktualizuj yt-dlp:**, które pozwala Vivace
  samodzielnie przeprowadzać taką aktualizację — **Nigdy**, **Przy każdym
  uruchomieniu yt-dlp**, albo raz na **dzień**/**tydzień**. Automatyczna
  aktualizacja uruchamia się tuż przed faktycznym rozwiązaniem adresu URL
  lub pobraniem filmu z YouTube, więc pierwsze odtworzenie po jej
  uruchomieniu trwa nieco dłużej; jeśli sama aktualizacja się nie powiedzie
  (np. brak połączenia sieciowego), Vivace po cichu kontynuuje z już
  zainstalowaną wersją, zamiast blokować odtwarzanie.
- **Wyłączone** — dla yt-dlp zarządzanego samodzielnie przez użytkownika
  (np. zainstalowanego przez `pip` lub menedżera pakietów systemu). Pole
  **Ścieżka yt-dlp:** staje się edytowalne, dzięki czemu możesz wskazać tę
  kopię, a **Automatycznie aktualizuj yt-dlp** jest wyłączone — Vivace nigdy
  nie instaluje ani nie aktualizuje yt-dlp, którym nie zarządza. W tym
  trybie przycisk **Zainstaluj / zaktualizuj yt-dlp…** jest również
  wyłączony.

## Instalowanie dostawcy tokenów PO dla YouTube

Coraz więcej najnowszych filmów na YouTube wymaga **tokenu PO
(proof-of-origin)**, aby w ogóle mogły się odtworzyć — jest to ogólny
wymóg odtwarzalności, niezależny od plików cookie czy zalogowania. Bez
niego yt-dlp zgłasza, że film jest niedostępny, nawet przy zwykłym,
anonimowym żądaniu. Dotyczy to **obu** trybów — Strumieniowania oraz
Pobierania i odtwarzania — w przeciwieństwie do plików cookie (tylko
pobieranie, patrz niżej) czy Deno (dotyczy głównie Pobierania i
odtwarzania, patrz dalej).

*Ustawienia ▸ Sieć ▸ YouTube* zawiera przycisk **Zainstaluj dostawcę
tokenów PO…**, który konfiguruje społecznościowy projekt **„BgUtils POT
Provider”**: niewielką wtyczkę do yt-dlp oraz skrypt — uruchamiany na
żądanie przez Deno, ten sam program opisany dalej — dzięki któremu
yt-dlp może wygenerować token automatycznie. Kliknij przycisk, poczekaj
na zakończenie krótkiego procesu pobierania i budowania, i gotowe; po
zainstalowaniu etykieta przycisku zmienia się na **Zainstaluj ponownie /
zaktualizuj dostawcę tokenów PO…**, na wypadek gdyby później potrzebna
była aktualizacja.

**Warto pamiętać:**

- Wymaga to działającej instalacji Deno (zobacz „Instalowanie Deno do
  pobierania z YouTube” poniżej) — przycisk pobiera i buduje własny
  skrypt dostawcy, który yt-dlp uruchamia następnie przez Deno za
  każdym razem, gdy dany film rzeczywiście wymaga tokenu.
- Nie każdy film wymaga tokenu PO, więc odtwarzanie może działać
  poprawnie bez tej instalacji — ale jeśli film zgłasza się jako
  niedostępny, mimo że odtwarza się poprawnie gdzie indziej, warto go
  zainstalować.
- **Na Androidzie** ten komponent jest dołączony do Vivace i
  konfigurowany automatycznie — nie trzeba niczego instalować i nie ma
  odpowiadającego mu przycisku; po prostu działa.

## Pliki cookie do pobierania z YouTube

Tryby YouTube **Pobierz i odtwórz** oraz **narzędzie zewnętrzne** mogą
działać tak, jakbyś był zalogowany — jest to potrzebne w przypadku
filmów z ograniczeniem wiekowym, dostępnych tylko dla członków lub w
inny sposób wymagających konta, i to właśnie ono odblokowuje pełne
pobieranie w HD/4K. Vivace obsługuje trzy sposoby dostarczania plików
cookie (wszystkie w *Ustawienia ▸ Sieć ▸ YouTube*): **Pobierz pliki
cookie z przeglądarki** w systemach Windows/Linux/macOS, **Zaloguj się
do YouTube…** na Androidzie oraz ręczne wyeksportowanie pliku
`cookies.txt` jako rozwiązanie zastępcze wszędzie.

### Pobierz pliki cookie z przeglądarki (zalecane w systemach Windows/Linux/macOS)

Lista rozwijana **Pobierz pliki cookie z przeglądarki:** obejmuje
Firefox, Chrome, Edge, Brave, Chromium, Opera, Safari, Vivaldi i Whale.
Wybierz swoją przeglądarkę, a Vivace będzie odczytywać jej pliki cookie
na żywo, za każdym razem od nowa — nic nie trzeba eksportować, nic nie
traci ważności.

**Warto pamiętać:**

- YouTube znacząco skrócił czas życia własnych plików cookie, przez co
  wcześniej wyeksportowany plik `cookies.txt` (patrz niżej) może stracić
  ważność w ciągu kilku dni — ta opcja całkowicie temu zapobiega,
  odczytując bieżące, zawsze aktualne przechowywanie plików cookie samej
  przeglądarki.
- **W systemie Windows faktycznie działa tu tylko Firefox.** Chrome,
  Edge i inne przeglądarki oparte na Chromium w systemie Windows
  szyfrują swoje pliki cookie w sposób powiązany z własnym plikiem
  wykonywalnym przeglądarki („App-Bound Encryption”, Chrome 127+) — to
  całkowicie uniemożliwia yt-dlp i każdemu innemu zewnętrznemu
  narzędziu ich odczytanie. Jest to ograniczenie po stronie Chrome,
  którego nawet twórcy yt-dlp nie mogą obejść. Chrome/Edge w systemach
  Linux i macOS nie są tym dotknięte i działają normalnie.
- Wybranie tu przeglądarki ma pierwszeństwo przed polem **Plik
  cookie:** poniżej, gdy ustawione są oba warianty.
- **Niedostępne na Androidzie** — zamiast tego zobacz **Zaloguj się do
  YouTube** poniżej.

### Zaloguj się do YouTube (Android, zalecane tam)

*Ustawienia ▸ Sieć ▸ YouTube* zawiera przycisk **Zaloguj się do
YouTube…** (obok pola pliku cookie), który otwiera prawdziwą stronę
logowania wewnątrz samego Vivace, przy użyciu wbudowanej, natywnej dla
systemu przeglądarki WebView. Zaloguj się na konto, z którego dostępu
chcesz skorzystać, a następnie stuknij **Jestem zalogowany** — Vivace
odczyta wynikowe pliki cookie sesji i automatycznie zapisze je jako
aktywny plik cookie. Jest to odpowiednik funkcji **Pobierz pliki cookie
z przeglądarki** powyżej dla Androida: nic nie trzeba eksportować ani
ręcznie przenosić z innego urządzenia.

**Warto pamiętać:**

- Android w ogóle nie ma odpowiednika bieżącego magazynu plików cookie
  przeglądarki na komputerze, z którego można by odczytywać dane
  (pamięć każdej aplikacji jest odizolowana od pozostałych) — dlatego
  działa to inaczej niż opcja na komputerze: logowanie się *wewnątrz*
  własnej wbudowanej przeglądarki Vivace jest praktycznym zamiennikiem.
- Dostępne tylko na Androidzie; na każdej innej platformie zamiast tego
  używana jest funkcja **Pobierz pliki cookie z przeglądarki**.

### Eksportowanie plików cookie do pliku (rozwiązanie zastępcze na każdej platformie)

Pole **Plik cookie:** wymaga zwykłego pliku tekstowego `cookies.txt` w
klasycznym formacie Netscape cookie-jar (tym samym, który odczytuje
własna opcja `--cookies` narzędzia yt-dlp) — użyj go, gdy żadna z
powyższych opcji na żywo nie jest dostępna lub nie działa (np.
Chrome/Edge w systemie Windows).

**Aby go utworzyć:**

1. Zaloguj się na youtube.com w swojej codziennej przeglądarce, używając
   konta, z którego dostępu chcesz skorzystać.
2. Zainstaluj rozszerzenie przeglądarki do eksportu plików cookie, które
   zapisuje w formacie Netscape — dla Chrome, Edge lub Brave wyszukaj w
   sklepie z rozszerzeniami swojej przeglądarki coś w rodzaju „Get
   cookies.txt”; dla Firefoksa wyszukaj „cookies.txt”. Zadziała każde
   rozszerzenie, które wyraźnie deklaruje eksport w klasycznym formacie
   Netscape `cookies.txt`.
3. Mając otwartą kartę z youtube.com, użyj rozszerzenia, aby wyeksportować
   pliki cookie dla tej strony, i zapisz wynik gdzieś na dysku jako plik
   `.txt`.
4. W Vivace otwórz *Ustawienia ▸ Sieć ▸ YouTube* i użyj przycisku
   **Przeglądaj…** obok pola **Plik cookie:**, aby wybrać ten plik.

**Na Androidzie:** zamiast tego lepiej skorzystaj z **Zaloguj się do
YouTube…** powyżej — nie wymaga ono w ogóle kroku eksportu/przenoszenia.
Jeśli mimo to chcesz zrobić to w ten sposób: Chrome na Androida nie
obsługuje rozszerzeń
przeglądarki, więc kroków 2–3 powyżej nie da się wykonać na samym
urządzeniu. Wyeksportuj `cookies.txt` w sposób opisany powyżej na
komputerze stacjonarnym lub laptopie, a następnie przenieś ten plik na
urządzenie z Androidem (np. za pomocą chmury, kabla USB lub e-maila),
zanim użyjesz przycisku **Przeglądaj…** w kroku 4.

**Warto pamiętać:**

- Plik `cookies.txt` jest w praktyce zapisaną sesją logowania — każdy, kto
  ma ten plik, może działać jako Twoje konto YouTube, dopóki pliki cookie
  nie wygasną lub się nie wylogujesz. Przechowuj go w bezpiecznym miejscu i
  nie udostępniaj go nikomu.
- Pliki cookie wygasają. Jeśli pobieranie, które wcześniej działało, zaczyna
  się nie powodzić lub zwraca wynik o niższej jakości/publiczny, wyeksportuj
  nowy plik `cookies.txt` — albo, jeśli to możliwe, przełącz się na **Pobierz
  pliki cookie z przeglądarki** powyżej, aby całkowicie tego uniknąć.

**Dotyczy obu powyższych metod:**

- Pliki cookie są używane wyłącznie na ścieżce **pobierania** (Pobierz i
  odtwórz / narzędzie zewnętrzne). Vivace celowo nigdy nie wysyła plików
  cookie w trybie **strumieniowania** — adres URL strumienia powiązanego z
  zalogowaną sesją jest związany z tą sesją w sposób, którego zwykły
  odtwarzacz wideo Vivace nie jest w stanie otworzyć, więc strumieniowanie
  pozostaje anonimowe, niezależnie od tego, czy pliki cookie są
  skonfigurowane w ten czy inny sposób.

## Instalowanie ffmpeg do pobierania z YouTube

Tryb **Pobierz i odtwórz** potrzebuje `ffmpeg`, aby połączyć osobne strumienie
wideo i audio pobierane przez yt-dlp w jeden odtwarzalny plik — YouTube
rzadko oferuje HD jako jeden połączony strumień, więc ścieżka wideo i ścieżka
audio są pobierane osobno, a następnie łączone. Pole **Lokalizacja ffmpeg:**
(*Ustawienia ▸ Sieć ▸ YouTube ▸ Pobierz i odtwórz*) informuje yt-dlp, gdzie
go znaleźć; pozostaw je puste, aby zamiast tego użyć `ffmpeg` z systemowej
zmiennej PATH.

**Aby zainstalować ffmpeg:**

1. **Windows** — najprostszą opcją jest menedżer pakietów:
   `winget install ffmpeg` (albo `scoop install ffmpeg` / `choco install
   ffmpeg`). Można też pobrać gotowe archiwum z
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) lub
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) i rozpakować
   je gdzieś.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — zainstaluj go z menedżera pakietów swojej dystrybucji, np.
   `sudo apt install ffmpeg` (Debian/Ubuntu), `sudo dnf install ffmpeg`
   (Fedora) lub `sudo pacman -S ffmpeg` (Arch).
4. Jeśli dodałeś ffmpeg do systemowej zmiennej PATH, zostaw pole
   **Lokalizacja ffmpeg:** puste. W przeciwnym razie wklej do tego pola
   ścieżkę do *folderu* zawierającego plik wykonywalny `ffmpeg` (nie sam
   plik wykonywalny).
5. Uruchom Vivace ponownie (albo po prostu spróbuj ponownie pobrać film) po
   zainstalowaniu.

**Warto pamiętać:**

- To zależność samego **yt-dlp**, podobnie jak Deno poniżej — Vivace zawsze
  uruchamia go jako proces zewnętrzny.
- Tryb **Strumieniowanie** nigdy nie potrzebuje ffmpeg, ponieważ odtwarza
  pojedynczy, już połączony strumień; potrzebuje go tylko tryb **Pobierz i
  odtwórz**, ponieważ pobiera on wideo i audio osobno i łączy je lokalnie.
- Jeśli pobieranie kończy się błędem związanym z łączeniem, sprawdź najpierw
  lokalizację ffmpeg — to najczęstsza przyczyna, obok brakującego lub
  nieaktualnego Deno.

## Instalowanie Deno do pobierania z YouTube

Samo yt-dlp — nie tylko Vivace — korzysta z osobnego, zewnętrznego środowiska
uruchomieniowego JavaScript, aby rozwiązywać zabezpieczenia, jakie YouTube
stawia przed udostępnieniem rzeczywistego adresu URL pobierania filmu.
Zgodnie z własną dokumentacją yt-dlp, uruchamianie bez takiego środowiska
jest „przestarzałe”, ale nie kończy się od razu niepowodzeniem: dostępność
formatów jest po prostu ograniczona, i to **poważnie w przypadku zapytań z
zalogowaną sesją (plikami cookie)** — czyli dokładnie takich, jakie wykonuje
tryb **Pobierz i odtwórz**, aby odblokować materiały w jakości HD, dostępne
tylko dla członków oraz z ograniczeniem wiekowym. Tryb **Strumieniowanie**
nigdy nie wysyła plików cookie (zobacz sekcję „Pliki cookie do
pobierania z YouTube” powyżej), więc nie dotyczy go ten poważny przypadek i w
większości sytuacji działa poprawnie bez Deno. Dlatego pole **Ścieżka do
Deno:** znajduje się w *Ustawienia ▸ Sieć ▸ YouTube ▸ Pobierz i odtwórz*, a
nie jako ogólne ustawienie YouTube. yt-dlp obsługuje kilka środowisk JS;
Deno jest tym, którego szuka domyślnie.

**Aby zainstalować Deno:**

1. Postępuj zgodnie z oficjalną instrukcją instalacji na stronie
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   dla swojego systemu operacyjnego (skrypt instalacyjny albo menedżer
   pakietów, np. winget/scoop/Homebrew/apt, zależnie od platformy).
2. Upewnij się, że plik wykonywalny `deno` trafia do systemowej zmiennej
   PATH — powyższe instalatory zwykle robią to automatycznie. W systemie
   Windows upewnij się, że pobierasz `deno`, a nie `denort` (inny, powiązany
   plik wykonywalny, który tutaj nie zadziała).
3. Jeśli wolisz nie modyfikować PATH, zostaw wszystko bez zmian i zamiast
   tego wklej pełną ścieżkę do pliku w polu **Ścieżka do Deno:**
   (*Ustawienia ▸ Sieć ▸ YouTube ▸ Pobierz i odtwórz*).
4. Uruchom Vivace ponownie (albo po prostu spróbuj ponownie pobrać film) po
   zainstalowaniu.

**Warto pamiętać:**

- To zależność samego **yt-dlp**, a nie bezpośrednio Vivace — Vivace zawsze
  uruchamia yt-dlp jako proces zewnętrzny i nigdy nie wywołuje Deno
  samodzielnie.
- yt-dlp wymaga stosunkowo nowej wersji Deno (w chwili pisania tego tekstu —
  2.3.0 lub nowszej). Jeśli po instalacji pobrania nadal pokazują ograniczoną
  jakość lub błędy formatu, sprawdź `deno --version` i zaktualizuj Deno,
  jeśli jest starsze.
- Ten wymóg wynika ze zmian po stronie YouTube/yt-dlp, a nie Vivace —
  właśnie dlatego istnieje pole **Ścieżka do Deno:** i nie wymaga ono żadnej
  dalszej konfiguracji, gdy tylko Deno jest zainstalowane i osiągalne.

## Wygładzanie napisów bitmapowych

*Ustawienia ▸ Napisy ▸ Napisy bitmapowe* zawiera opcję
**Wygładzanie:** (0–3, domyślnie 1) dla napisów wyświetlanych jako
obrazy, a nie tekst — ścieżek podobrazów DVD, PGS i DVB. Obejmuje to
zarówno własne napisy prawdziwej płyty DVD, jak i osadzoną ścieżkę
napisów tego samego typu w zwykłym pliku wideo (np. pliku .mp4 ze
ścieżką w kodeku `dvd_subtitle`). Te formaty to wcześniej
wyrenderowane obrazy bitmapowe, zapisane w natywnej rozdzielczości
standardowej (SD) podczas tworzenia źródła — ich krawędzie mogą
wyglądać na postrzępione po powiększeniu do rozmiaru współczesnego
okna. Vivace może zastosować lekkie rozmycie, aby wygładzić te
krawędzie:

- **0** — wyłączone; pokazuje oryginalną bitmapę napisów dokładnie tak,
  jak została utworzona.
- **1** (domyślnie) — wygładza najbardziej szorstkie krawędzie,
  zachowując przy tym niemal pełną jasność tekstu.
- **2** / **3** — stopniowo silniejsze rozmycie.

Ta opcja dotyczy wyłącznie napisów bitmapowych — nie ma wpływu na
własny, zewnętrzny mechanizm renderowania napisów Vivace (SRT/VTT/ASS)
ani na zwykłe tekstowe ścieżki napisów, które korzystają z innych
ścieżek renderowania.
