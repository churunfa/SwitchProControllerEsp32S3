#ifndef SWITCHPROCONTROLLERESP32S3_SWITCHWAKEUP_H
#define SWITCHPROCONTROLLERESP32S3_SWITCHWAKEUP_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <esp_mac.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

inline uint8_t DEFAULT_BLE_MAC[6] = {
    0xE0, 0xEF, 0xBF, 0x2B, 0x5B, 0x1E
};

inline uint8_t DEFAULT_BLE_DATA[] = {
    0x02, 0x01, 0x06, 0x1B, 0xFF, 0x53, 0x05, 0x01,
    0x00, 0x03, 0x7E, 0x05, 0x66, 0x20, 0x00, 0x01,
    0x81, 0x9B, 0xFD, 0x54, 0x05, 0x48, 0xC8, 0x0F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// 现在的长度是纯正的 31 字节，完美符合 BLE 广播极限！
constexpr size_t WAKEUP_ADV_DATA_LEN = sizeof(DEFAULT_BLE_DATA);

class SwitchWakeUp {
    bool _isWakingUp = false;
    TaskHandle_t _taskHandle = nullptr;

    SwitchWakeUp() = default;

    [[noreturn]] static void wakeUpTask(void* parameter) {
        auto* instance = static_cast<SwitchWakeUp *>(parameter);

        for (;;) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            vTaskDelay(pdMS_TO_TICKS(3000));

            logPrintf("[SwitchWakeUp] 3秒已到，唤醒完毕，恢复小程序连接服务...\n");

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
        setSpoofedMAC(DEFAULT_BLE_MAC);
        if (_taskHandle == nullptr) {
            xTaskCreate(wakeUpTask, "WakeUpTask", 2048, this, 1, &_taskHandle);
        }
    }

    static void setSpoofedMAC(const uint8_t* mac) {
        esp_base_mac_addr_set(mac);
    }

    // 3. 【修补强转】：把 uint8_t* 强转成 char* 以喂给库函数
    void trigger() {
        trigger((char*)DEFAULT_BLE_DATA, WAKEUP_ADV_DATA_LEN);
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