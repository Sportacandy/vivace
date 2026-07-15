# 命令列選項

Vivace 可以從終端機啟動，並帶一個媒體檔案與一些選項。使用 GNU 風格的長選項（括號中給出
短別名）。

```
vivace [選項] [媒體]
```

- **媒體** — 要播放的檔案路徑或 URL。Vivace 通常能開啟的任何內容（本機檔案、資料夾、
  `VIDEO_TS`、網路串流、YouTube URL）在此均可使用。

## 選項

- **`--help`（`-h`）** — 顯示選項清單並結束。
- **`--version`（`-v`）** — 顯示版本並結束。
- **`--fullscreen`（`-f`）** — 以全螢幕啟動。
- **`--ontop`（`-t`）** — 使視窗保持在其他視窗之上。
- **`--close-at-end`（`-c`）** — 播放結束時關閉 Vivace。
- **`--sub <檔案>`（`-s`）** — 載入指定的字幕檔案。
- **`--start <時間>`（`-b`）** — 從 `<時間>` 開始播放，格式為 `h:m:s`、`m:s` 或秒數。
- **`--pos <x,y>`** — 將視窗置於螢幕位置 `x,y`。
- **`--size <w,h>`** — 將視窗大小設為 `w`×`h`。

## 範例

```
vivace movie.mkv
vivace --fullscreen --start 1:30 movie.mkv
vivace -f -b 90 -s movie.en.srt movie.mkv
vivace --pos 100,100 --size 1280,720 movie.mp4
```

**注意（Windows）：** Vivace 是圖形介面應用程式，因此 `--help` / `--version` 會列印到
您啟動它的終端機（shell 提示字元可能先返回，因此文字可能在其後出現）。啟用單一執行個體
模式後（*偏好設定 ▸ 介面 ▸ 執行個體*），再次以媒體檔案與這些選項啟動會將它們套用到
已在執行的視窗。
