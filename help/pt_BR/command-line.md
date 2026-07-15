# Opções de linha de comando

O Vivace pode ser iniciado a partir de um terminal com um arquivo de mídia e
algumas opções. Use opções longas no estilo GNU (os aliases curtos são mostrados
entre parênteses).

```
vivace [opções] [mídia]
```

- **mídia** — um caminho de arquivo ou URL para reproduzir. Tudo o que o Vivace
  pode abrir normalmente (arquivo local, pasta, `VIDEO_TS`, transmissão de rede,
  URL do YouTube) funciona aqui.

## Opções

- **`--help` (`-h`)** — mostrar a lista de opções e sair.
- **`--version` (`-v`)** — mostrar a versão e sair.
- **`--fullscreen` (`-f`)** — iniciar em tela cheia.
- **`--ontop` (`-t`)** — manter a janela acima das outras.
- **`--close-at-end` (`-c`)** — fechar o Vivace quando a reprodução terminar.
- **`--sub <arquivo>` (`-s`)** — carregar o arquivo de legenda indicado.
- **`--start <tempo>` (`-b`)** — iniciar a reprodução em `<tempo>`, indicado como
  `h:m:s`, `m:s` ou um número de segundos.
- **`--pos <x,y>`** — posicionar a janela na posição de tela `x,y`.
- **`--size <l,a>`** — definir o tamanho da janela como `l`×`a`.

## Exemplos

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**Nota (Windows):** o Vivace é um aplicativo gráfico, então `--help` /
`--version` imprimem no terminal de onde você o iniciou (o prompt do shell pode
retornar primeiro, então o texto pode aparecer depois). Quando o modo de
instância única está ativado (*Preferências ▸ Interface ▸ Instâncias*), iniciá-lo
novamente com um arquivo de mídia e essas opções as aplica à janela já em execução.
