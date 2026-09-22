# Seçenekler

**Seçenekler** menüsü tercihleri ve arayüz yapılandırmasını içerir.

- **Tercihler…** (`Ctrl+P`) — ana ayarlar iletişim kutusu: Genel, Arayüz, Altyazı,
  Klavye ve fare, Oynatma listesi, Sürücüler, TV, Dosya türleri, Güncellemeler, Ağ
  ve Gelişmiş. Değişiklikler hemen uygulanır; **İptal** onları geri alır.
  - **Genel ▸ Video** yeni açılan dosyalar için varsayılan **Görüntü
    ayrıştırma** kipini (Hiçbiri / Yadif / Bwdif / Otomatik) ayarlar — dosya
    başına değiştirmek için *Video ▸ Görüntü ayrıştırma* bölümünü kullanın
    (yalnızca Hiçbiri/Yadif/Bwdif sunulur; Otomatik yalnızca varsayılan
    değer olarak anlam kazandığından burada sunulmaz). **Otomatik**, Bwdif'i
    yalnızca dosyanın kendisinin taramalı olarak işaretlediği karelerde
    kullanır; aşamalı taramalı kareler değişmeden bırakılır.
  - **Genel ▸ Tercih edilen ses ve altyazılar** Vivace'nin bir dosyanın
    gömülü parçaları arasından otomatik olarak seçtiği dilleri ayarlar.
    **Altyazıları varsayılan olarak göster** kullanılabilir olduğunda
    otomatik olarak bir altyazı açar; alt seçeneği olan **...ancak ses
    zaten tercih edilen bir dildeyse hariç**, dili gerçekten seçilen ses
    parçasıyla eşleştiğinde bu otomatik altyazıyı atlar — sesi zaten
    anlıyorsanız ve aynı dildeki bir altyazının dikkatinizi dağıtmasını
    istemiyorsanız kullanışlıdır.
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

## YouTube PO belirteci sağlayıcısını kurma

Son zamanlarda YouTube videoları, yalnızca oynatılabilmek için bile
giderek daha sık bir **PO (kaynak kanıtı, proof-of-origin) belirteci**
istiyor — bu, çerezlerden veya oturum açmış olmaktan bağımsız, genel bir
oynatılabilirlik gereksinimidir. Bu belirteç olmadan, sıradan, anonim bir
istekte bile yt-dlp videoyu kullanılamaz olarak bildirir. Bu, yalnızca
indirmeyi ilgilendiren çerezlerden (aşağıya bakın) veya çoğunlukla İndir
ve oynat'ı ilgilendiren Deno'dan (daha aşağıya bakın) farklı olarak, hem
Akış (streaming) hem de İndir ve oynat modlarının **her ikisi** için de
geçerlidir.

*Tercihler ▸ Ağ ▸ YouTube*'da, topluluk projesi **"BgUtils POT
Provider"**'ı kuran bir **PO belirteci sağlayıcısını kur…** düğmesi
vardır: küçük bir yt-dlp eklentisi ile, gerektiğinde Deno (daha aşağıda
ele alınan aynı program) aracılığıyla çalıştırılan bir betik, böylece
yt-dlp bir belirteci otomatik olarak oluşturabilir. Düğmeye tıklayın,
kısa indirme ve derleme işleminin tamamlanmasını bekleyin, işte bu kadar;
kurulduktan sonra, ileride bir güncelleme gerekirse diye düğmenin etiketi
**PO belirteci sağlayıcısını yeniden kur / güncelle…** olarak değişir.

**Aklınızda bulunsun:**

- Bu, çalışan bir Deno kurulumu gerektirir (aşağıdaki "YouTube
  indirmeleri için Deno kurulumu" bölümüne bakın) — düğme, sağlayıcının
  kendi betiğini indirip derler; yt-dlp da bir video gerçekten bir
  belirtece ihtiyaç duyduğunda bu betiği Deno aracılığıyla çalıştırır.
- Her video bir PO belirtecine ihtiyaç duymaz, bu yüzden bunu kurmadan da
  oynatma sorunsuz çalışabilir — ancak bir video başka bir yerde sorunsuz
  oynarken burada kullanılamaz olarak bildiriliyorsa, bunu kurmaya değer.
- **Android'de** bu, Vivace ile birlikte paketlenir ve otomatik olarak
  kurulur — kurulacak bir şey ve buna karşılık gelen bir düğme yoktur;
  sadece çalışır.

## YouTube indirmeleri için çerezler

**İndir ve oynat** ve **dış araç** YouTube modları sanki oturum açmışsınız
gibi davranabilir — yaş sınırlı, yalnızca üyelere özel veya başka şekilde
hesaba bağlı videolar için gereklidir ve tam HD/4K indirmelerin kilidini
açan da budur. Vivace, çerez sağlamak için iki yol destekler (ikisi de
*Tercihler ▸ Ağ ▸ YouTube ▸ İndir ve oynat* altındadır); Windows/Linux/
macOS'ta, sizin kurulumunuzda işe yaramadığı sürece kullanılması gereken
yöntem **Çerezleri tarayıcıdan al**'dır.

### Çerezleri tarayıcıdan al (önerilir)

**Çerezleri tarayıcıdan al:** açılır kutusu Firefox, Chrome, Edge, Brave,
Chromium, Opera, Safari, Vivaldi ve Whale'i listeler. Tarayıcınızı seçin;
Vivace her seferinde çerezlerini canlı olarak okur — dışa aktarılacak
hiçbir şey, eskiyecek hiçbir şey yoktur.

**Aklınızda bulunsun:**

- YouTube kendi çerez ömürlerini önemli ölçüde kısalttı, bu yüzden daha
  önce dışa aktarılmış bir `cookies.txt` dosyası (aşağıya bakın) birkaç
  gün içinde eskiyebilir — bu seçenek, tarayıcının kendi, her zaman
  güncel çerez deposunu okuyarak bunu tamamen ortadan kaldırır.
- **Windows'ta burada gerçekten yalnızca Firefox çalışır.** Windows'ta
  Chrome, Edge ve diğer Chromium tabanlı tarayıcılar, çerezleri
  tarayıcının kendi ikili dosyasına bağlı bir şekilde şifreler
  ("App-Bound Encryption", Chrome 127+) — bu, yt-dlp'nin (ve diğer tüm
  harici araçların) çerezleri hiç okuyamamasına neden olur. Bu,
  yt-dlp'nin kendi geliştiricilerinin aşamadığı Chrome tarafındaki bir
  kısıtlamadır. Linux ve macOS'taki Chrome/Edge bundan etkilenmez ve
  normal şekilde çalışır.
- Burada bir tarayıcı seçmek, ikisi de ayarlandığında aşağıdaki
  **Çerez dosyası:** alanına göre önceliklidir.
- **Android'de kullanılamaz** — bunun yerine aşağıdaki elle dışa aktarma
  yöntemini kullanın.

### Çerezleri bir dosyaya dışa aktarma (yedek yöntem, ve Android'deki tek seçenek)

**Çerez dosyası:** alanı, klasik Netscape çerez kavanozu biçiminde
(yt-dlp'nin kendi `--cookies` seçeneğinin okuduğu biçimin aynısı) düz
metin bir `cookies.txt` dosyası bekler — yukarıdaki canlı tarayıcı
seçeneği kullanılamadığında (Android) veya tarayıcınızda işe yaramadığında
(Windows'ta Chrome/Edge) bunu kullanın.

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

**Android'de:** Android için Chrome tarayıcı uzantılarını desteklemez, bu
yüzden yukarıdaki 2–3. adımlar cihazın kendisinde yapılamaz. `cookies.txt`
dosyasını yukarıda açıklandığı gibi bir masaüstü veya dizüstü bilgisayarda
dışa aktarın, ardından 4. adımda **Gözat…**'ı kullanmadan önce bu dosyayı
Android cihazınıza aktarın (örn. bulut depolama, bir USB kablosu veya
e-posta yoluyla).

**Aklınızda bulunsun:**

- Bir `cookies.txt` dosyası, aslında kaydedilmiş bir oturum açma oturumudur
  — bu dosyaya sahip olan herkes, çerezler süresi dolana veya siz oturumu
  kapatana kadar YouTube hesabınız gibi davranabilir. Onu özel bir yerde
  saklayın ve kimseyle paylaşmayın.
- Çerezlerin süresi dolar. Daha önce çalışan indirmeler başarısız olmaya
  başlarsa veya daha düşük kaliteli/herkese açık bir sonuca geri düşerse,
  yeni bir `cookies.txt` dışa aktarın — veya mümkünse bunu tamamen önlemek
  için yukarıdaki **Çerezleri tarayıcıdan al** seçeneğine geçin.

**Yukarıdaki her iki yöntem için de geçerlidir:**

- Çerezler yalnızca **indirme** yolunda (İndir ve oynat / dış araç)
  kullanılır. Vivace, **akış (streaming)** modunda çerezleri kasıtlı olarak
  asla göndermez — oturum açılmış bir akış URL'si, Vivace'nin sade video
  oynatıcısının açamayacağı bir şekilde o oturuma bağlıdır, bu yüzden
  çerezler hangi şekilde yapılandırılmış olursa olsun akış anonim kalır.

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
(yukarıdaki "YouTube indirmeleri için çerezler" bölümüne
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
