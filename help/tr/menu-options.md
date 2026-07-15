# Seçenekler

**Seçenekler** menüsü tercihleri ve arayüz yapılandırmasını içerir.

- **Tercihler…** (`Ctrl+P`) — ana ayarlar iletişim kutusu: Genel, Arayüz, Altyazı,
  Klavye ve fare, Oynatma listesi, Sürücüler, TV, Dosya türleri, Güncellemeler, Ağ
  ve Gelişmiş. Değişiklikler hemen uygulanır; **İptal** onları geri alır.
  - **Ağ** sekmesinde OpenSubtitles, YouTube, Vekil Sunucu ve Yayınlama
    sekmeleri bulunur; **Vekil Sunucu** sekmesi, uygulama genelinde geçerli
    isteğe bağlı bir HTTP veya SOCKS5 vekil sunucusu ayarlar (OpenSubtitles
    araması, güncelleme denetimi ve yalnızca HTTP için ortam oynatma ile
    yt-dlp); **Yayınlama** sekmesi, *Oynat ▸ Şuraya yayınla ▸ Akıllı telefon/
    tablet* özelliğinin dinlediği bağlantı noktasını sabitler. OpenSubtitles
    hesap parolası ve vekil sunucu parolası artık Vivace'nin kendi
    ayarlarında değil, işletim sisteminizin kimlik bilgisi yöneticisinde
    güvenli bir şekilde saklanır.
- **Sistem tepsisinde simge göster** — Vivace'yi tepsiden erişilebilir tutar.
- **Araç çubukları**
  - **Araç çubuğu** / **Denetim çubuğu** — her çubuğu gösterir veya gizler.
  - **Ana araç çubuğunu düzenle…** / **Denetim çubuğunu düzenle…** — hangi
    düğmelerin görüneceğini, sıralarını ve simge boyutunu seçin.
- **Durum çubuğu**
  - **Durum çubuğunu göster** ve gösterdikleri: **Video bilgisi**, **Ses
    bilgisi**, **Biçim bilgisi**, **Bit hızı bilgisi**, **Kare sayacı**, **Toplam
    süreyi göster**, **Kalan süreyi göster** ve **Geçerli zamanı milisaniyeyle göster**.

**İpucu:** genel düzen (Basic / Mini / MPC) *Tercihler ▸ Arayüz* bölümünde seçilir.

## YouTube indirmeleri için çerezleri dışa aktarma

**Çerez dosyası:** alanı (*Tercihler ▸ Ağ ▸ YouTube*), **İndir ve oynat** ve
**dış araç** YouTube modlarının sanki oturum açmışsınız gibi davranmasını
sağlar — yaş sınırlı, yalnızca üyelere özel veya başka şekilde hesaba bağlı
videolar için gereklidir ve tam HD/4K indirmelerin kilidini açan da budur.
Bu alan, klasik Netscape çerez kavanozu biçiminde (yt-dlp'nin kendi
`--cookies` seçeneğinin okuduğu biçimin aynısı) düz metin bir `cookies.txt`
dosyası bekler; Vivace çerezleri doğrudan bir tarayıcı profilinden okumaz.

**Bir tane oluşturmak için:**

1. Kullanmak istediğiniz erişime sahip hesapla, günlük kullandığınız
   tarayıcıda youtube.com'da oturum açın.
2. Netscape biçiminde yazan bir çerez dışa aktarma tarayıcı uzantısı
   yükleyin — Chrome, Edge veya Brave için tarayıcınızın uzantı mağazasında
   "Get cookies.txt" gibi bir şey arayın; Firefox için "cookies.txt" arayın.
   Klasik Netscape `cookies.txt` biçiminde dışa aktardığını açıkça belirten
   herhangi bir uzantı işe yarar.
3. youtube.com bir sekmede açıkken, uzantıyı kullanarak o site için
   çerezleri dışa aktarın ve sonucu diskte bir yere `.txt` dosyası olarak
   kaydedin.
4. Vivace'de *Tercihler ▸ Ağ ▸ YouTube* bölümünü açın ve **Çerez dosyası:**
   yanındaki **Gözat…** düğmesini kullanarak bu dosyayı seçin.

**Aklınızda bulunsun:**

- Bir `cookies.txt` dosyası, aslında kaydedilmiş bir oturum açma oturumudur
  — bu dosyaya sahip olan herkes, çerezler süresi dolana veya siz oturumu
  kapatana kadar YouTube hesabınız gibi davranabilir. Onu özel bir yerde
  saklayın ve kimseyle paylaşmayın.
- Çerezler yalnızca **indirme** yolunda (İndir ve oynat / dış araç)
  kullanılır. Vivace, **akış (streaming)** modunda çerezleri kasıtlı olarak
  asla göndermez — oturum açılmış bir akış URL'si, Vivace'nin sade video
  oynatıcısının açamayacağı bir şekilde o oturuma bağlıdır, bu yüzden bir
  çerez dosyası yapılandırılmış olsa bile akış anonim kalır.
- Çerezlerin süresi dolar. Daha önce çalışan indirmeler başarısız olmaya
  başlarsa veya daha düşük kaliteli/herkese açık bir sonuca geri düşerse,
  yeni bir `cookies.txt` dışa aktarın.
