# Opcje

Menu **Opcje** zawiera ustawienia i konfigurację interfejsu.

- **Ustawienia…** (`Ctrl+P`) — główne okno ustawień: Ogólne, Interfejs, Napisy,
  Klawiatura i mysz, Lista odtwarzania, Napędy, TV, Typy plików, Aktualizacje,
  Sieć i Zaawansowane. Zmiany są stosowane natychmiast; **Anuluj** je cofa.
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

## Eksportowanie plików cookie do pobierania z YouTube

Pole **Plik cookie:** (*Ustawienia ▸ Sieć ▸ YouTube*) pozwala trybom YouTube
**Pobierz i odtwórz** oraz **narzędzie zewnętrzne** działać tak, jakbyś był
zalogowany — jest to potrzebne w przypadku filmów z ograniczeniem wiekowym,
dostępnych tylko dla członków lub w inny sposób wymagających konta, i to
właśnie ono odblokowuje pełne pobieranie w HD/4K. Wymagany jest zwykły plik
tekstowy `cookies.txt` w klasycznym formacie Netscape cookie-jar (tym samym,
który odczytuje własna opcja `--cookies` narzędzia yt-dlp); Vivace nie
odczytuje plików cookie bezpośrednio z profilu przeglądarki.

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

**Warto pamiętać:**

- Plik `cookies.txt` jest w praktyce zapisaną sesją logowania — każdy, kto
  ma ten plik, może działać jako Twoje konto YouTube, dopóki pliki cookie
  nie wygasną lub się nie wylogujesz. Przechowuj go w bezpiecznym miejscu i
  nie udostępniaj go nikomu.
- Pliki cookie są używane wyłącznie na ścieżce **pobierania** (Pobierz i
  odtwórz / narzędzie zewnętrzne). Vivace celowo nigdy nie wysyła plików
  cookie w trybie **strumieniowania** — adres URL strumienia powiązanego z
  zalogowaną sesją jest związany z tą sesją w sposób, którego zwykły
  odtwarzacz wideo Vivace nie jest w stanie otworzyć, więc strumieniowanie
  pozostaje anonimowe, nawet jeśli skonfigurowano plik cookie.
- Pliki cookie wygasają. Jeśli pobieranie, które wcześniej działało, zaczyna
  się nie powodzić lub zwraca wynik o niższej jakości/publiczny, wyeksportuj
  nowy plik `cookies.txt`.
