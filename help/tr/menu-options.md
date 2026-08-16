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

## yt-dlp'yi kurma ve güncelleme

*Tercihler ▸ Ağ ▸ YouTube*'da, Vivace'nin YouTube bağlantılarını oynatmak
için kullandığı `yt-dlp` programını nasıl edindiğini ve güncel tuttuğunu
denetleyen bir **Yönetilen yt-dlp kullan** onay kutusu vardır:

- **Açık** (varsayılan) — Vivace `yt-dlp`'yi sizin için kurabilir ve güncel
  tutabilir. **yt-dlp yolu:** alanı Vivace'nin kendi kopyasına sabitlenir ve
  doğrudan düzenlenemez; en son resmi sürümü almak istediğinizde onay
  kutusunun yanındaki **yt-dlp'yi kur / güncelle…** düğmesini kullanın.
  **yt-dlp'yi otomatik güncelle:** ayarı da kullanılabilir hale gelir ve
  Vivace'nin bu güncellemeyi kendi başına çalıştırmasını sağlar —
  **Asla**, **yt-dlp her çalıştığında**, **Günde bir kez** veya **Haftada
  bir kez**. Otomatik güncelleme, bir YouTube URL'si gerçekten çözülmeden
  veya indirilmeden hemen önce çalışır, bu yüzden güncelleme zamanı
  geldikten sonraki ilk oynatma biraz daha uzun sürer; güncellemenin
  kendisi başarısız olursa (örn. ağ yoksa), Vivace oynatmayı engellemeden
  sessizce zaten kurulu olan sürümle devam eder.
- **Kapalı** — kendi yönettiğiniz bir yt-dlp için (örn. `pip` veya işletim
  sisteminizin paket yöneticisiyle kurulmuş). **yt-dlp yolu:** alanı
  düzenlenebilir hale gelir, böylece o kopyayı gösterebilirsiniz ve
  **yt-dlp'yi otomatik güncelle** devre dışı bırakılır — Vivace, kendisinin
  yönetmediği bir yt-dlp'yi asla kurmaz veya güncellemez. **yt-dlp'yi kur /
  güncelle…** düğmesi de bu modda devre dışıdır.

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

## YouTube indirmeleri için ffmpeg kurulumu

**İndir ve oynat** modu, yt-dlp'nin indirdiği ayrı video ve ses akışlarını
tek bir oynatılabilir dosyada birleştirmek için `ffmpeg`'e ihtiyaç duyar —
YouTube, HD'yi tek bir birleşik akış olarak nadiren sunar, bu yüzden bir
video parçası ve bir ses parçası ayrı ayrı indirilip sonra birleştirilir.
**ffmpeg konumu:** alanı (*Tercihler ▸ Ağ ▸ YouTube ▸ İndir ve oynat*)
yt-dlp'ye onu nerede bulacağını söyler; bunun yerine sistem PATH'inizdeki
`ffmpeg`'i kullanmak için boş bırakın.

**ffmpeg'i kurmak için:**

1. **Windows** — en kolay yöntem bir paket yöneticisi kullanmaktır:
   `winget install ffmpeg` (veya `scoop install ffmpeg` /
   `choco install ffmpeg`). Alternatif olarak,
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) veya
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) adresinden
   önceden derlenmiş bir arşiv indirip bir yere açabilirsiniz.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — dağıtımınızın paket yöneticisinden kurun, örneğin
   `sudo apt install ffmpeg` (Debian/Ubuntu), `sudo dnf install ffmpeg`
   (Fedora) veya `sudo pacman -S ffmpeg` (Arch).
4. ffmpeg'i sistem PATH'inize eklediyseniz **ffmpeg konumu:** alanını boş
   bırakın. Aksi hâlde, `ffmpeg` çalıştırılabilir dosyasının kendisini
   değil, onu içeren *klasörün* yolunu bu alana yapıştırın.
5. Kurulumdan sonra Vivace'yi yeniden başlatın (veya sadece bir indirmeyi
   yeniden deneyin).

**Aklınızda bulunsun:**

- Bu, aşağıdaki Deno gibi, **yt-dlp**'nin bir bağımlılığıdır — Vivace onu
  yalnızca harici bir işlem olarak çalıştırır.
- **Akış (streaming)** modu, zaten birleştirilmiş tek bir akışı oynattığı
  için ffmpeg'e hiç ihtiyaç duymaz; yalnızca **İndir ve oynat** ihtiyaç
  duyar, çünkü bu mod videoyu ve sesi ayrı ayrı alıp yerel olarak
  birleştirir.
- Bir indirme birleştirmeyle ilgili bir hatayla başarısız olursa, önce
  ffmpeg konumunu kontrol edin — eksik veya güncel olmayan bir Deno'nun
  ardından en yaygın nedendir.

## YouTube indirmeleri için Deno kurulumu

Yalnızca Vivace değil, yt-dlp'nin kendisi de, YouTube'un bir videonun
gerçek indirme URL'sini vermeden önce sunduğu zorlukları çözmek için ayrı
bir harici JavaScript çalışma zamanı kullanır. yt-dlp'nin kendi
belgelerine göre, bir çalışma zamanı olmadan çalıştırmak "kullanımdan
kaldırılmış" sayılır ama tamamen başarısız olmaz: yalnızca kullanılabilir
biçim sayısı azalır ve bu azalma **oturum açmış (çerez) bir istek için
ciddi biçimde** gerçekleşir — tam olarak **İndir ve oynat** modunun HD,
yalnızca üyelere özel ve yaş sınırlı videoların kilidini açmak için
yaptığı istek türü budur. **Akış (streaming)** modu asla çerez göndermez
(yukarıdaki "YouTube indirmeleri için çerezleri dışa aktarma" bölümüne
bakın), bu yüzden ciddi durum bu değildir ve çoğu durumda Deno olmadan da
iyi çalışır. **Deno yolu:** alanının genel bir YouTube ayarı olarak değil
de *Tercihler ▸ Ağ ▸ YouTube ▸ İndir ve oynat* altında bulunmasının
nedeni budur. yt-dlp birden fazla JS çalışma zamanını destekler; Deno
varsayılan olarak aradığı çalışma zamanıdır.

**Deno'yu kurmak için:**

1. İşletim sisteminize uygun olarak
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   adresindeki resmi kurulum talimatlarını izleyin (platforma bağlı olarak
   bir kurulum betiği veya winget/scoop/Homebrew/apt gibi bir paket
   yöneticisi).
2. `deno` çalıştırılabilir dosyasının sistem PATH'inize eklendiğinden emin
   olun — yukarıdaki kurulum programları genellikle bunu sizin için
   yapar. Windows'ta `denort`'u (burada işe yaramayan, farklı ama ilgili
   bir çalıştırılabilir dosya) değil, `deno`'yu aldığınızdan emin olun.
3. PATH'i değiştirmek istemiyorsanız, olduğu gibi bırakın ve bunun yerine
   tam yolunu **Deno yolu:** alanına yapıştırın (*Tercihler ▸ Ağ ▸
   YouTube ▸ İndir ve oynat*).
4. Kurulumdan sonra Vivace'yi yeniden başlatın (veya sadece bir indirmeyi
   yeniden deneyin).

**Aklınızda bulunsun:**

- Bu, Vivace'nin değil **yt-dlp**'nin bir bağımlılığıdır — Vivace, yt-dlp'yi
  yalnızca harici bir işlem olarak çalıştırır ve Deno'yu asla kendisi
  çağırmaz.
- yt-dlp, makul ölçüde güncel bir Deno sürümü gerektirir (bu yazının
  yazıldığı sırada 2.3.0 veya üzeri). Kurulumdan sonra indirmeler hâlâ
  düşük kalite/biçim hataları gösteriyorsa, `deno --version`'ı kontrol
  edin ve eskiyse güncelleyin.
- Bu gereklilik Vivace'den değil, YouTube'un/yt-dlp'nin tarafındaki
  değişikliklerden kaynaklanır — tam da bu nedenle **Deno yolu:** alanı
  vardır ve Deno'nun kendisi kurulup erişilebilir olduğunda başka bir
  yapılandırmaya gerek kalmaz.

## Bit eşlemli altyazı yumuşatma

*Tercihler ▸ Altyazılar ▸ Bit eşlemli altyazılar*'da, metin yerine
görüntü olarak işlenen altyazılar için bir **Yumuşatma:** ayarı (0–3,
varsayılan 1) bulunur — DVD alt resmi, PGS ve DVB parçaları. Bu, hem
gerçek bir DVD diskin kendi altyazılarını hem de sıradan bir video
dosyasındaki (ör. `dvd_subtitle` codec'li bir parçaya sahip bir .mp4
dosyası) aynı türden gömülü bir altyazı parçasını kapsar. Bu biçimler,
kaynak oluşturulurken yerel standart tanım (SD) çözünürlüğünde önceden
oluşturulmuş bit eşlemi görüntülerdir — modern bir pencere boyutuna
büyütüldüğünde kenarları pürüzlü görünebilir. Vivace, bu kenarları
yumuşatmak için hafif bir bulanıklık uygulayabilir:

- **0** — kapalı; orijinal altyazı bit eşlemini tam olarak
  oluşturulduğu gibi gösterir.
- **1** (varsayılan) — metnin parlaklığını neredeyse tam korurken en
  pürüzlü kenarları yumuşatır.
- **2** / **3** — giderek daha fazla bulanıklaştırır.

Bu ayar yalnızca bit eşlemli altyazıları etkiler — Vivace'nin kendi
harici altyazı işleyicisini (SRT/VTT/ASS) veya sıradan metin tabanlı
altyazı parçalarını etkilemez; bunların ikisi de farklı işleme yolları
kullanır.
