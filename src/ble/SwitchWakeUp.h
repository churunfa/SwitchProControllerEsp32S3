#ifndef SWITCHPROCONTROLLERESP32S3_SWITCHWAKEUP_H
#define SWITCHPROCONTROLLERESP32S3_SWITCHWAKEUP_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <esp_mac.h>
#include <config/SimpleConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class SwitchWakeUp {
private:
    volatile bool _isWakingUp = false;
    TaskHandle_t _taskHandle = nullptr;
    bool _hasValidConfig = false;

    uint8_t _bleMac[6];
    std::vector<uint8_t> _bleData;
    size_t _advDataLen;

    SwitchWakeUp() {
        auto& config = SimpleConfig::getInstance();
        std::vector<uint8_t> macData;
        if (config.getConfig(ConfigType::MAC_ADDRESS, macData) && macData.size() == 6) {
            memcpy(_bleMac, macData.data(), 6);
            _hasValidConfig = true;
        }

        std::vector<uint8_t> advData;
        if (config.getConfig(ConfigType::NS2_WAKE_DATA, advData)) {
            if (!advData.empty() && advData.size() <= 31) {
                _bleData = std::move(advData);
                _advDataLen = _bleData.size();
                _hasValidConfig = true;
            }
        }
    };

    [[noreturn]] static void wakeUpTask(void* parameter) {
        auto* instance = static_cast<SwitchWakeUp *>(parameter);
        for (;;) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            // 保持发射 3 秒，确保 Switch 能够扫描到
            vTaskDelay(pdMS_TO_TICKS(3000));

            BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
            pAdvertising->stop();

            // 彻底清理：恢复到默认的可连接状态，给小程序使用
            BLEAdvertisementData empty;
            pAdvertising->setAdvertisementData(empty);
            pAdvertising->setScanResponseData(empty);

            // 关键：恢复默认广播参数（允许连接）
            // 注意：这里可能需要根据你的小程序库重新 init 广播参数
            pAdvertising->start();

            instance->_isWakingUp = false;
        }
    }

public:
    static SwitchWakeUp& getInstance() {
        static SwitchWakeUp instance;
        return instance;
    }

    void begin() {
        if (!_hasValidConfig) return;

        setSpoofedMAC(_bleMac);
        if (_taskHandle == nullptr) {
            // 提升栈大小到 4096 避免崩溃
            xTaskCreate(wakeUpTask, "WakeUpTask", 4096, this, 2, &_taskHandle);
        }
    }

    static void setSpoofedMAC(const uint8_t* mac) {
        esp_base_mac_addr_set(mac);
    }

    void trigger() {
        if (!_hasValidConfig) return;
        trigger(reinterpret_cast<char*>(_bleData.data()), _advDataLen);
    }

    void trigger(char *advData, const size_t dataLen) {
        if (_isWakingUp || _taskHandle == nullptr) return;

        _isWakingUp = true;

        BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->stop();

        // 1. 构建广播数据
        BLEAdvertisementData wakeUpData;
        wakeUpData.addData(advData, dataLen);

        // 2. 同时设置广播数据和扫描响应数据（双重保险）
        pAdvertising->setAdvertisementData(wakeUpData);
        pAdvertising->setScanResponseData(wakeUpData);

        // 3. 【核心优化】设置高频、不可连接广播
        // 20ms 的间隔 (0.625ms * 32) 是 BLE 允许的最快频率之一
        pAdvertising->setMinInterval(0x20);
        pAdvertising->setMaxInterval(0x20);

        // 注意：某些版本的 Arduino BLE 库在 start 前会自动设置类型
        // 如果你的库支持，确保它是非连接模式

        pAdvertising->start();

        xTaskNotifyGive(_taskHandle);
    }
};

#endif //SWITCHPROCONTROLLERESP32S3_SWITCHWAKEUP_H