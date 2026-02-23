#ifndef SWITCHPROCONTROLLERESP32S3_SWITCHWAKEUP_H
#define SWITCHPROCONTROLLERESP32S3_SWITCHWAKEUP_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <esp_mac.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <config/SimpleConfig.h>

class SwitchWakeUp {
private:
    bool _isWakingUp = false;
    TaskHandle_t _taskHandle = nullptr;
    bool _hasValidConfig = false;
    
    // 配置数据成员
    uint8_t _bleMac[6];
    std::vector<uint8_t> _bleData;
    size_t _advDataLen;
    
    SwitchWakeUp() {
        // 从SimpleConfig获取配置，如果不存在则不进行操作
        auto& config = SimpleConfig::getInstance();
        
        // 获取MAC地址配置
        std::vector<uint8_t> macData;
        if (config.getConfig(ConfigType::MAC_ADDRESS, macData) && macData.size() == 6) {
            memcpy(_bleMac, macData.data(), 6);
            _hasValidConfig = true;
        }
        
        // 获取广播数据配置
        if (config.getConfig(ConfigType::NS2_WAKE_DATA, _bleData) && !_bleData.empty() && _bleData.size() == 31) {
            _advDataLen = _bleData.size();
            _hasValidConfig = true;
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
        // 只有在有有效配置时才进行初始化
        if (!_hasValidConfig) {
            return;
        }
        
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
        // 只有在有有效配置时才触发
        if (!_hasValidConfig) {
            return;
        }
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