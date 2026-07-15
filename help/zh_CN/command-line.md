# 命令行选项

Vivace 可以从终端启动，并带一个媒体文件和一些选项。使用 GNU 风格的长选项（括号中给出
短别名）。

```
vivace [选项] [媒体]
```

- **媒体** — 要播放的文件路径或 URL。Vivace 通常能打开的任何内容（本地文件、文件夹、
  `VIDEO_TS`、网络流、YouTube URL）在此均可使用。

## 选项

- **`--help`（`-h`）** — 显示选项列表并退出。
- **`--version`（`-v`）** — 显示版本并退出。
- **`--fullscreen`（`-f`）** — 以全屏启动。
- **`--ontop`（`-t`）** — 使窗口保持在其他窗口之上。
- **`--close-at-end`（`-c`）** — 播放结束时关闭 Vivace。
- **`--sub <文件>`（`-s`）** — 加载指定的字幕文件。
- **`--start <时间>`（`-b`）** — 从 `<时间>` 开始播放，格式为 `h:m:s`、`m:s` 或秒数。
- **`--pos <x,y>`** — 将窗口置于屏幕位置 `x,y`。
- **`--size <w,h>`** — 将窗口大小设为 `w`×`h`。

## 示例

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**注意（Windows）：** Vivace 是图形界面应用程序，因此 `--help` / `--version` 会打印到
您启动它的终端（shell 提示符可能先返回，因此文本可能在其后出现）。启用单实例模式后
（*首选项 ▸ 界面 ▸ 实例*），再次以媒体文件和这些选项启动会将它们应用到已在运行的窗口。
