# Komut satırı seçenekleri

Vivace bir terminalden bir ortam dosyası ve birkaç seçenekle başlatılabilir.
GNU tarzı uzun seçenekler kullanın (kısa takma adlar parantez içinde gösterilir).

```
vivace [seçenekler] [ortam]
```

- **ortam** — oynatılacak bir dosya yolu veya URL. Vivace'nin normalde
  açabildiği her şey (yerel dosya, klasör, `VIDEO_TS`, ağ akışı, YouTube URL'si)
  burada çalışır.

## Seçenekler

- **`--help` (`-h`)** — seçenek listesini gösterir ve çıkar.
- **`--version` (`-v`)** — sürümü gösterir ve çıkar.
- **`--fullscreen` (`-f`)** — tam ekran başlatır.
- **`--ontop` (`-t`)** — pencereyi diğer pencerelerin üstünde tutar.
- **`--close-at-end` (`-c`)** — oynatma bittiğinde Vivace'yi kapatır.
- **`--sub <dosya>` (`-s`)** — verilen altyazı dosyasını yükler.
- **`--start <süre>` (`-b`)** — oynatmayı `<süre>` konumundan başlatır; `h:m:s`,
  `m:s` veya saniye sayısı olarak verilir.
- **`--pos <x,y>`** — pencereyi ekran konumu `x,y` 'ye yerleştirir.
- **`--size <g,y>`** — pencere boyutunu `g`×`y` olarak ayarlar.

## Örnekler

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Not (Windows):** Vivace bir grafik arayüz uygulamasıdır, bu nedenle `--help` /
`--version` çıktısı onu başlattığınız terminale yazılır (kabuk istemi önce
dönebilir, dolayısıyla metin sonrasında görünebilir). Tek örnek kipi açıkken
(*Tercihler ▸ Arayüz ▸ Örnekler*), bir ortam dosyası ve bu seçeneklerle yeniden
başlatmak onları zaten çalışan pencereye uygular.
