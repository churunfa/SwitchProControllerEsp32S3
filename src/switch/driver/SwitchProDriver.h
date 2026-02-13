/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef SWITCHPROCONTROLLERESP32S3_SWITCH_PRO_DRIVER_H
#define SWITCHPROCONTROLLERESP32S3_SWITCH_PRO_DRIVER_H

#include <mutex>
#include <thread>
#include <random>
#include <map>
#include <vector>
#include <Adafruit_TinyUSB.h>
#include "switch/driver/SwitchProDescriptors.h"

#define SWITCH_PRO_KEEPALIVE_TIMER 15

typedef struct __attribute((packed, aligned(1))) {
    SwitchInputReport inputs;
    SwitchAnalog leftStick;
    SwitchAnalog rightStick;
    ImuData imuData[3];
} SwitchProSerialInput;

class SwitchProDriver {
public:

    virtual ~SwitchProDriver() = default;

    virtual bool process(bool force);
    virtual void initialize();
    virtual void set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize);
    virtual void resetSwitchReport();
    virtual bool updateInputReport(SwitchProSerialInput* serialInput);
    static SwitchProDriver& getInstance();

private:
    uint8_t report[SWITCH_PRO_ENDPOINT_SIZE] = { };
    uint8_t last_report[SWITCH_PRO_ENDPOINT_SIZE] = { };
    SwitchProReport switchReport{};
    uint8_t last_report_counter{};
    uint32_t last_report_timer{};
    bool is_init = false;
    bool isReady = false;
    bool isInitialized = false;
    bool isReportQueued = false;
    uint8_t queuedReportID = 0;

    uint8_t handshakeCounter = 0;
    std::mutex reportMtx;


    std::map<uint32_t, const uint8_t*> spiFlashData = {
        {0x6000, factoryConfigData},
        {0x8000, userCalibrationData}
    };

    SwitchDeviceInfo deviceInfo{};
    uint8_t playerID = 0;
    uint8_t inputMode = 0x21;
    bool isIMUEnabled = false;
    bool isVibrationEnabled = false;

    std::random_device rd;  // 硬件随机数生成器
    std::mt19937 gen;       // 梅森旋转算法生成器
    std::uniform_int_distribution<uint8_t> dist;

    SwitchProDriver();
    void sendIdentify();

    bool sendReport(uint8_t reportID, const void* reportData, bool addCount);

    void readSPIFlash(uint8_t* dest, uint32_t address, uint8_t size);

    void handleConfigReport(uint8_t switchReportID, uint8_t switchReportSubID, const uint8_t *reportData, uint16_t reportLength);
    void handleFeatureReport(uint8_t switchReportID, const uint8_t *reportData, uint16_t reportLength);

    static void set_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize);

    Adafruit_USBD_HID usb_hid;

    // config data
    SwitchFactoryConfig* factoryConfig = (SwitchFactoryConfig*)factoryConfigData;
    const uint8_t factoryConfigData[0xEFF] = {
        // serial number
        0x58, 0x41, 0x57, 0x37, 0x30, 0x30, 0x31, 0x38,
        0x33, 0x30, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00,

        0xFF, 0xFF,

        // device type
        SwitchControllerType::SWITCH_TYPE_PRO_CONTROLLER,

        // unknown
        0xA0,

        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

        // color options
        0x02,

        0xFF, 0xFF, 0xFF, 0xFF,

        // ------------------------------------------------------------
        // Config & Calibration 1 (主校准块)
        // ------------------------------------------------------------
        // [行1] Accel Origin (X,Y,Z全是0) + Accel Coeff X (标准值0x4000)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40,

        // [行2] Accel Coeff Y,Z (标准值0x4000) + Gyro Origin X,Y (全是0)
        0x00, 0x40, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,

        // [行3] Gyro Origin Z (全是0) + Gyro Coeff X,Y,Z (标准值0x343B)
        0x00, 0x00, 0x3B, 0x34, 0x3B, 0x34, 0x3B, 0x34,

        0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

        // ------------------------------------------------------------
        // Config & Calibration 2 (备份/用户块)
        // ------------------------------------------------------------

        // left stick
        0xFF, 0xF7, 0x7F, 0x00, 0x08, 0x80,
        0xFF, 0xF7, 0x7F,

        // right stick
        0x00, 0x08, 0x80, 0xFF, 0xF7, 0x7F,
        0xFF, 0xF7, 0x7F,

        0xFF,

        // body color
        0x2D, 0x2D, 0x2D,

        // button color
        0xE6, 0xE6, 0xE6,

        // left grip color
        0x64, 0x55, 0xF5,

        // right grip color
        0xC3, 0xFA, 0x05,

        0x01,

        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF,

        0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x40,

        0x00, 0x40, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,

        // [行3] Gyro Origin Z (全是 0) + Gyro Coeff X,Y,Z (0x343B)
        0x00, 0x00, 0x3B, 0x34, 0x3B, 0x34, 0x3B, 0x34,

        // 剩余的填充字节 (全部置为 0xFF)
        0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF
};

    SwitchUserCalibration* userCalibration = (SwitchUserCalibration*)userCalibrationData;
    const uint8_t userCalibrationData[0x3F] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        
        // Left Stick
        0xB2, 0xA1, 0xFF, 0xF7, 0x7F, 0x00, 0x08, 0x80,
        0xFF, 0xF7, 0x7F,

        // Right Stick
        0xB2, 0xA1, 0x00, 0x08, 0x80, 0xFF, 0xF7, 0x7F,
        0xFF, 0xF7, 0x7F,

        // Motion
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
};

inline SwitchProDriver& SwitchProDriver::getInstance() {
    static SwitchProDriver instance;
    if (!instance.is_init) {
        instance.initialize();
    }
    return instance;
}

#endif