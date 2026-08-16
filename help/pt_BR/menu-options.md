# Opções

O menu **Opções** contém as preferências e a configuração da interface.

- **Preferências…** (`Ctrl+P`) — a caixa de diálogo principal de configurações:
  Geral, Interface, Legendas, Teclado e mouse, Lista de reprodução, Unidades, TV,
  Tipos de arquivo, Atualizações, Rede e Avançado. As mudanças se aplicam na
  hora; **Cancelar** as reverte.
  - **Rede** tem as abas OpenSubtitles, YouTube, Proxy e Transmissão; **Proxy**
    configura um proxy HTTP ou SOCKS5 opcional, aplicado a todo o aplicativo
    (busca no OpenSubtitles, verificação de atualizações e, somente com HTTP,
    reprodução e yt-dlp); **Transmissão** fixa a porta em que *Reproduzir ▸
    Enviar para ▸ Smartphone/tablet* escuta. A senha da conta do OpenSubtitles
    e a senha do proxy agora são armazenadas com segurança no gerenciador de
    credenciais do sistema operacional, e não nas próprias configurações do
    Vivace.
- **Mostrar ícone na bandeja do sistema** — manter o Vivace acessível pela bandeja.
- **Barras de ferramentas**
  - **Barra de ferramentas** / **Barra de controle** — mostrar ou ocultar cada barra.
  - **Editar barra de ferramentas principal…** / **Editar barra de controle…** —
    escolher quais botões aparecem, sua ordem e o tamanho dos ícones.
- **Barra de status**
  - **Mostrar barra de status** e o que ela exibe: **Info de vídeo**, **Info de
    áudio**, **Info de formato**, **Info de taxa de bits**, **Contador de
    quadros**, **Mostrar tempo total**, **Mostrar tempo restante** e **Mostrar a
    hora atual com milissegundos**.

**Dica:** o layout geral (Basic / Mini / MPC) se escolhe em *Preferências ▸ Interface*.

## Instalando e atualizando o yt-dlp

*Preferências ▸ Rede ▸ YouTube* tem uma caixa de seleção **Usar yt-dlp
gerenciado** que controla como o Vivace obtém e mantém o programa `yt-dlp`
que ele usa para reproduzir links do YouTube:

- **Ativada** (o padrão) — o Vivace pode instalar o `yt-dlp` para você e
  mantê-lo atualizado. O campo **Caminho do yt-dlp:** fica fixo na própria
  cópia do Vivace e não pode ser editado diretamente; use o botão
  **Instalar / atualizar o yt-dlp…** (ao lado da caixa de seleção) sempre
  que quiser buscar a versão oficial mais recente. A configuração
  **Atualizar o yt-dlp automaticamente:** também fica disponível, permitindo
  que o Vivace execute essa atualização por conta própria — **Nunca**, ou
  então **Toda vez que o yt-dlp for executado**, ou uma vez por
  **dia**/**semana**. Uma atualização automática é executada pouco antes de
  uma URL do YouTube ser de fato resolvida ou baixada, então a primeira
  reprodução depois que ela vence demora um pouco mais; se a própria
  atualização falhar (por exemplo, sem rede), o Vivace segue discretamente
  com a versão já instalada em vez de bloquear a reprodução.
- **Desativada** — para um yt-dlp que você mesmo gerencia (por exemplo,
  instalado via `pip` ou o gerenciador de pacotes do seu sistema
  operacional). O campo **Caminho do yt-dlp:** se torna editável, para que
  você possa apontá-lo para essa cópia, e **Atualizar o yt-dlp
  automaticamente** fica desativado — o Vivace nunca instala nem atualiza um
  yt-dlp que não gerencia. O botão **Instalar / atualizar o yt-dlp…** também
  fica desativado nesse modo.

## Exportando cookies para downloads do YouTube

O campo **Arquivo de cookies:** (*Preferências ▸ Rede ▸ YouTube*) permite
que os modos do YouTube **Baixar e reproduzir** e **Ferramenta externa**
ajam como se você estivesse conectado — necessário para vídeos com
restrição de idade, exclusivos para membros ou vinculados de outra forma a
uma conta, e é isso que também libera downloads em HD/4K completo. Ele
espera um arquivo de texto simples `cookies.txt` no formato clássico de
cookies do Netscape (o mesmo formato que a própria opção `--cookies` do
yt-dlp lê); o Vivace não lê cookies diretamente do perfil de um navegador.

**Para criar um:**

1. Faça login em youtube.com no seu navegador do dia a dia, usando a conta
   cujo acesso você quer usar.
2. Instale uma extensão de navegador para exportar cookies que grave no
   formato Netscape — no Chrome, Edge ou Brave, procure na loja de
   extensões do seu navegador por algo como "Get cookies.txt"; no Firefox,
   procure por "cookies.txt". Qualquer extensão que declare claramente
   exportar no formato clássico Netscape `cookies.txt` vai funcionar.
3. Com youtube.com aberto em uma aba, use a extensão para exportar os
   cookies desse site e salve o resultado em algum lugar do disco como um
   arquivo `.txt`.
4. No Vivace, abra *Preferências ▸ Rede ▸ YouTube* e use **Procurar…** ao
   lado de **Arquivo de cookies:** para selecionar esse arquivo.

**Vale lembrar:**

- Um arquivo `cookies.txt` é, na prática, uma sessão de login salva —
  qualquer pessoa que tenha o arquivo pode agir como sua conta do YouTube
  até os cookies expirarem ou você sair da conta. Guarde-o em um lugar
  privado e não o compartilhe.
- Os cookies são usados apenas pelo caminho de **download** (Baixar e
  reproduzir / Ferramenta externa). O Vivace propositalmente nunca envia
  cookies no modo de **streaming** — uma URL de streaming autenticada fica
  vinculada a essa sessão de um jeito que o reprodutor de vídeo simples do
  Vivace não consegue abrir, então o streaming permanece anônimo mesmo com
  um arquivo de cookies configurado.
- Os cookies expiram. Se downloads que antes funcionavam começarem a
  falhar, ou caírem para um resultado público/de qualidade inferior,
  exporte um novo `cookies.txt`.

## Instalando o ffmpeg para downloads do YouTube

O modo **Baixar e reproduzir** precisa do `ffmpeg` para combinar os fluxos
separados de vídeo e áudio que o yt-dlp baixa em um único arquivo
reproduzível — o YouTube raramente oferece HD como um único fluxo
combinado, então uma faixa de vídeo e uma faixa de áudio são baixadas
separadamente e depois combinadas. O campo **Local do ffmpeg:**
(*Preferências ▸ Rede ▸ YouTube ▸ Baixar e reproduzir*) informa ao yt-dlp
onde encontrá-lo; deixe-o em branco para usar o `ffmpeg` do PATH do seu
sistema.

**Para instalar o ffmpeg:**

1. **Windows** — a opção mais fácil é um gerenciador de pacotes:
   `winget install ffmpeg` (ou `scoop install ffmpeg` / `choco install
   ffmpeg`). Como alternativa, baixe um pacote pré-compilado em
   [gyan.dev](https://www.gyan.dev/ffmpeg/builds/) ou
   [BtbN/FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds) e
   descompacte-o em algum lugar.
2. **macOS** — `brew install ffmpeg` (Homebrew).
3. **Linux** — instale-o pelo gerenciador de pacotes da sua distribuição,
   por exemplo `sudo apt install ffmpeg` (Debian/Ubuntu), `sudo dnf
   install ffmpeg` (Fedora), ou `sudo pacman -S ffmpeg` (Arch).
4. Se você adicionou o ffmpeg ao PATH do sistema, deixe **Local do
   ffmpeg:** em branco. Caso contrário, cole nesse campo o caminho da
   *pasta* que contém o executável `ffmpeg` (não o executável em si).
5. Reinicie o Vivace (ou apenas tente o download de novo) depois de
   instalar.

**Vale lembrar:**

- Essa é uma dependência do **yt-dlp**, assim como o Deno logo abaixo — o
  Vivace apenas o executa como um processo externo.
- O modo **Transmissão** nunca precisa do ffmpeg, já que reproduz um único
  fluxo já combinado; só o **Baixar e reproduzir** precisa, porque esse
  modo busca vídeo e áudio separadamente e os combina localmente.
- Se um download falhar com um erro relacionado à combinação, verifique
  primeiro o local do ffmpeg — essa é a causa mais comum, além de um Deno
  ausente ou desatualizado.

## Instalando o Deno para downloads do YouTube

O próprio yt-dlp — não apenas o Vivace — usa um ambiente de execução
JavaScript externo separado para resolver os desafios que o YouTube impõe
antes de fornecer a URL de download real de um vídeo. Segundo a própria
documentação do yt-dlp, funcionar sem um deles é "descontinuado"
(deprecated), mas não falha diretamente: a disponibilidade de formatos
simplesmente é reduzida, e **de forma severa para uma solicitação com login
(cookies)** — exatamente o tipo de solicitação que o modo **Baixar e
reproduzir** faz para desbloquear vídeos em HD, exclusivos para membros e
com restrição de idade. O modo **Transmissão** nunca envia cookies (veja
"Exportando cookies para downloads do YouTube" acima), então não é o caso
mais grave e funciona bem sem o Deno na maioria dos casos. É por isso que o
campo **Caminho do Deno:** fica em *Preferências ▸ Rede ▸ YouTube ▸ Baixar
e reproduzir*, e não como uma configuração geral do YouTube. O yt-dlp
oferece suporte a vários ambientes de execução JavaScript; o Deno é o que
ele procura por padrão.

**Para instalar o Deno:**

1. Siga as instruções de instalação oficiais em
   [docs.deno.com](https://docs.deno.com/runtime/getting_started/installation/)
   para o seu sistema operacional (um script de instalação, ou um
   gerenciador de pacotes como winget/scoop/Homebrew/apt, dependendo da
   plataforma).
2. Certifique-se de que o executável `deno` acabe no PATH do seu sistema —
   os instaladores acima normalmente fazem isso para você. No Windows,
   certifique-se de obter o `deno`, e não o `denort` (um executável
   diferente e relacionado que não funciona aqui).
3. Se preferir não modificar o PATH, deixe-o como está e, em vez disso,
   cole o caminho completo em **Caminho do Deno:** (*Preferências ▸ Rede ▸
   YouTube ▸ Baixar e reproduzir*).
4. Reinicie o Vivace (ou apenas tente o download novamente) depois de
   instalar.

**Vale lembrar:**

- Isso é uma dependência do **yt-dlp**, não do Vivace diretamente — o
  Vivace apenas executa o yt-dlp como um processo externo e nunca invoca o
  Deno por conta própria.
- O yt-dlp exige uma versão do Deno razoavelmente recente (2.3.0 ou
  posterior no momento em que isso foi escrito). Se os downloads
  continuarem mostrando qualidade reduzida ou erros de formato depois da
  instalação, verifique `deno --version` e atualize-o se estiver
  desatualizado.
- Essa exigência vem de mudanças do lado do YouTube/yt-dlp, não do Vivace
  — o mesmo campo **Caminho do Deno:** existe exatamente por esse motivo e
  não precisa de nenhuma configuração adicional depois que o próprio Deno
  estiver instalado e acessível.

## Suavização de legendas em bitmap

*Preferências ▸ Legendas ▸ Legendas em bitmap* tem uma configuração
**Suavização:** (0–3, padrão 1) para legendas exibidas como imagens em
vez de texto: faixas de subimagem de DVD, PGS e DVB. Isso abrange tanto
as legendas próprias de um disco DVD real quanto uma faixa de legenda
incorporada do mesmo tipo em um arquivo de vídeo comum (por exemplo, um
arquivo .mp4 com uma faixa no codec `dvd_subtitle`). Esses formatos são
imagens bitmap pré-renderizadas, gravadas na resolução nativa em
definição padrão (SD) quando a fonte foi criada — suas bordas podem
parecer serrilhadas ao serem ampliadas para o tamanho de uma janela
moderna. O Vivace pode aplicar um leve desfoque para suavizar essas
bordas:

- **0** — desativado; mostra o bitmap de legenda original exatamente
  como foi criado.
- **1** (padrão) — suaviza as bordas mais ásperas mantendo o texto com
  um brilho praticamente total.
- **2** / **3** — desfoque progressivamente maior.

Essa configuração afeta apenas as legendas em bitmap — não tem nenhum
efeito sobre o renderizador de legendas externo próprio do Vivace
(SRT/VTT/ASS) nem sobre faixas de legenda de texto comuns, que usam
caminhos de renderização diferentes.
