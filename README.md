# SwitchProControllerEsp32S3——基于Adafruit_TinyUSB_Arduino实现的Switch Pro手柄模拟器

[![Hardware](https://img.shields.io/badge/Hardware-ESP32--S3-orange)]()
[![Function](https://img.shields.io/badge/Mode-USB%20Bridge-blue)]()

这是基于 **ESP32-S3** 芯片的 **USB Bridge 固件**。

它的核心功能是将 ESP32-S3 模拟为一个 **原生Nintendo Switch Pro 有线手柄**，同时监听 **UART 串口**。它可以接收来自 PC 端的操作指令，并将其实时转化为 Switch 识别的 HID 信号。

## ✨ 功能特性 (Features)

*   **📡 USB HID**: 适用于 ESP32-S3 (需配合 [SwitchProControllerLibrary](https://github.com/churunfa/SwitchProControllerLibrary) 项目)，实现低延迟有线连接。
*   **🎮 全键位映射**：支持 A/B/X/Y, L/R/ZL/ZR, D-Pad, Home, Capture 等所有标准按键。
*   **🕹️ 模拟摇杆**：高精度的左右摇杆控制 (摇杆坐标范围0~4096，2048表示居中)。
*   **体感(IMU)**: 支持体感模拟（手动设置角速度和加速度模拟体感）
*   **支持性**: 模拟Switch Pro1手柄，支持Nintendo Switch 1、Nintendo Switch 2

## 🔌 硬件接线指南

由于需要同时连接 PC (接收指令) 和 Switch (发送信号)，推荐使用带有 **双 type-c 接口** 的 ESP32-S3 开发板 (如 ESP32-S3-DevKitC-1)。

| 接口名称 | 连接对象 | 作用 |
| :--- | :--- | :--- |
| **UART / COM** | **电脑 (PC)** | 接收 按键操作指令  |
| **USB / OTG** | **Switch 底座/主机** | 模拟 Pro 手柄输入 |

> **⚠️ 注意**: 请勿将 Switch 连接到 UART(COM) 口，否则无法识别为手柄。

## ⚙️ Switch 设置 (必做!)

为了让 Switch 识别该模拟器，**必须**在主机上开启有线通讯功能：

1.  进入 Switch 主界面 -> **设置**
2.  选择 **手柄与外设**
3.  将 **Switch Pro手柄的有线连接** 设置为 **开启**

## 💻 编译与烧录 (PlatformIO)

请使用 PlatformIO 进行编译，`platformio.ini` 关键配置如下：


```ini
[env:switch-pre-controller-esp32-s3]
; Arduino Release v3.3.5 based on ESP-IDF v5.5.1.251215
platform = https://github.com/pioarduino/platform-espressif32/releases/download/55.03.35/platform-espressif32.zip
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200
build_flags =
    -D ARDUINO_USB_MODE=0
    -D ARDUINO_USB_CDC_ON_BOOT=1
    -D ARDUINO_USB_MSC_ON_BOOT=0
    -D ARDUINO_USB_DFU_ON_BOOT=0
lib_deps =
    adafruit/Adafruit TinyUSB Library @ 3.7.3
    adafruit/Adafruit NeoPixel@^1.15.2
; lib_archive = no
monitor_filters = esp32_exception_decoder
```

## 参考项目
[Nintendo_Switch_Reverse_Engineering](https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering)

[GP2040-CE](https://github.com/OpenStickCommunity/GP2040-CE)

[Adafruit_TinyUSB_Arduino](https://github.com/adafruit/Adafruit_TinyUSB_Arduino)

