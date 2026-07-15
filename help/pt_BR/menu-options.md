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
