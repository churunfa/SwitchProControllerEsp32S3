#ifndef SWITCHPROCONTROLLERESP32S3_SWITCHWAKEUP_H
#define SWITCHPROCONTROLLERESP32S3_SWITCHWAKEUP_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <esp_mac.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <config/SimpleConfig.h>

// 默认值常量
static const uint8_t DEFAULT_BLE_MAC[6] = {
    0xE0, 0xEF, 0xBF, 0x2B, 0x5B, 0x1E
};

static const uint8_t DEFAULT_BLE_DATA[] = {
    0x02, 0x01, 0x06, 0x1B, 0xFF, 0x53, 0x05, 0x01,
    0x00, 0x03, 0x7E, 0x05, 0x66, 0x20, 0x00, 0x01,
    0x81, 0x9B, 0xFD, 0x54, 0x05, 0x48, 0xC8, 0x0F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const size_t DEFAULT_WAKEUP_ADV_DATA_LEN = sizeof(DEFAULT_BLE_DATA);

class SwitchWakeUp {
private:
    bool _isWakingUp = false;
    TaskHandle_t _taskHandle = nullptr;
    
    // 配置数据成员
    uint8_t _bleMac[6];
    std::vector<uint8_t> _bleData;
    size_t _advDataLen;
    
    SwitchWakeUp() {
        // 从SimpleConfig获取配置，如果不存在则使用默认值
        auto& config = SimpleConfig::getInstance();
        
        // 获取MAC地址配置
        std::vector<uint8_t> macData;
        if (config.getConfig(ConfigType::MAC_ADDRESS, macData) && macData.size() == 6) {
            memcpy(_bleMac, macData.data(), 6);
        } else {
            memcpy(_bleMac, DEFAULT_BLE_MAC, 6);
        }
        
        // 获取广播数据配置
        if (config.getConfig(ConfigType::NS2_WAKE_DATA, _bleData) && !_bleData.empty()) {
            _advDataLen = _bleData.size();
        } else {
            _bleData.assign(DEFAULT_BLE_DATA, DEFAULT_BLE_DATA + DEFAULT_WAKEUP_ADV_DATA_LEN);
            _advDataLen = DEFAULT_WAKEUP_ADV_DATA_LEN;
        }
    };

    [[noreturn]] static void wakeUpTask(void* parameter) {
        auto* instance = static_cast<SwitchWakeUp *>(parameter);

        for (;;) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            vTaskDelay(pdMS_TO_TICKS(3000));
            BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
            pAdvertising->stop();

            auto emptyData = BLEAdvertisementData();
            pAdvertising->setAdvertisementData(emptyData);
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
        setSpoofedMAC(_bleMac);
        if (_taskHandle == nullptr) {
            xTaskCreate(wakeUpTask, "WakeUpTask", 2048, this, 1, &_taskHandle);
        }
    }

    static void setSpoofedMAC(const uint8_t* mac) {
        esp_base_mac_addr_set(mac);
    }

    // 3. 【修补强转】：把 uint8_t* 强转成 char* 以喂给库函数
    void trigger() {
        trigger(reinterpret_cast<char*>(_bleData.data()), _advDataLen);
    }

    void trigger(char *advData, const size_t dataLen) {
        if (_isWakingUp) return;

        if (_taskHandle == nullptr) {
            return;
        }

        _isWakingUp = true;

        BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->stop();

        auto wakeUpData = BLEAdvertisementData();
        wakeUpData.addData(advData, dataLen);
        pAdvertising->setAdvertisementData(wakeUpData);
        pAdvertising->start();
        xTaskNotifyGive(_taskHandle);
    }
};

#endif //SWITCHPROCONTROLLERESP32S3_SWITCHWAKEUP_H