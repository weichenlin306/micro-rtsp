# Micro-RTSP 可執行碼選粹

### 程式碼來源
- https://github.com/rzeldent/esp32cam-rtsp 經編譯後於 .pio 目錄下之Micro-RTSP目錄

### 原始碼
- https://github.com/geeksville/Micro-RTSP

### License
- 依各程式碼各自之原License (MIT License)

### 程式修飾與增強(使用Arduino IDE 2.x.x)
- 程式名稱: ESP32-CAM Demo.ino
- 去註解可實現畫面上下翻轉、左右鏡像、亮度調整、飽和度調整
- 可調整時鐘振盪頻率(.xclk_freq_hz與軟排線接點發熱相關)、畫面大小(.frame_size)、與兩幀畫面間之時間間隔
    (msecPerFrame, frame rate相關)

### 用法
- 將Micro-RTSP整個目錄複製到Arduino IDE安裝目錄下的libraries目錄下
- 將主程式ESP32-CAM Demo.ino更名為自己喜歡的檔名，與wifikeys.h一起複製到同名新Arduino專案目錄下
- 修改前述程式相關設定；wifikeys.h填入WiFi帳號密碼
- 以VLC測試執行狀態，須調整前述幾個參數至不出現"error sending udp packet"

![Serial monitor messages in Arduino IDE](https://github.com/weichenlin306/micro-rtsp/assets/133075659/c234700a-0e69-4f38-93a0-153de66970f4) ![RTSP screen in VLC](https://github.com/weichenlin306/micro-rtsp/assets/133075659/7beb3aaf-da99-40c3-81a4-0bc1bb9795aa)
