#ifndef SWITCHPROCONTROLLERESP32S3_PROCONTROLLERSNIFFER_H
#define SWITCHPROCONTROLLERESP32S3_PROCONTROLLERSNIFFER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <config/SimpleConfig.h>
#include "debug/led_control.h"

class ProControllerSniffer {
private:
    ProControllerSniffer() = default;

    class SnifferCallbacks : public BLEAdvertisedDeviceCallbacks {
        void onResult(BLEAdvertisedDevice advertisedDevice) override {
            uint8_t* payload = advertisedDevice.getPayload();
            size_t payloadLength = advertisedDevice.getPayloadLength();

            // 只要扫到设备就打个点，确认扫描器活着
            logPrintf(".");

            bool isNintendoWakeUp = false;
            for(int i = 0; i < (int)payloadLength - 2; i++) {
                if(payload[i] == 0xFF && payload[i+1] == 0x53 && payload[i+2] == 0x05) {
                    isNintendoWakeUp = true;
                    break;
                }
            }

            if (isNintendoWakeUp) {
                logPrintf("\n🎯 [MATCH] 抓到手柄包！ MAC: %s\n", advertisedDevice.getAddress().toString().c_str());

                // 1. 拿到原始字节 (1F 5B 2B BF EF E0)
                const uint8_t* nativeMac = reinterpret_cast<const uint8_t*>(advertisedDevice.getAddress().getNative());

                // 2. 反转顺序存储到临时数组 (变为 E0 EF BF 2B 5B 1F)
                uint8_t spoofedMac[6];
                for (int i = 0; i < 6; i++) {
                    spoofedMac[i] = nativeMac[5 - i];
                }

                // 3. 现在执行 "-1" 偏移量计算 (针对正确的尾号 1F 进行减法)
                // 此时 spoofedMac[5] 才是真正的尾部 (1F)
                for (int i = 5; i >= 0; i--) {
                    if (spoofedMac[i] > 0) {
                        spoofedMac[i] -= 1;
                        break;
                    } else {
                        spoofedMac[i] = 0xFF;
                    }
                }

                // 打印一下确认顺序是否正确
                logPrintf("📝 修正并计算后的 MAC: %02X-%02X-%02X-%02X-%02X-%02X\n",
                          spoofedMac[0], spoofedMac[1], spoofedMac[2],
                          spoofedMac[3], spoofedMac[4], spoofedMac[5]);

                // 4. 保存到配置
                auto& config = SimpleConfig::getInstance();
                config.setConfig(ConfigType::MAC_ADDRESS, std::vector<uint8_t>(spoofedMac, spoofedMac + 6));
                config.setConfig(ConfigType::NS2_WAKE_DATA, std::vector<uint8_t>(payload, payload + payloadLength));
                showBlueLed();
                logPrintf("✅ 自动配置完成，3秒后重启系统...\n");

                BLEDevice::getScan()->stop();
                delay(3000);
                ESP.restart();
            }
        }
    };

public:
    volatile bool pendingScan = false;
    static ProControllerSniffer& getInstance() {
        static ProControllerSniffer instance;
        return instance;
    }

    void startDetection(uint32_t durationSec = 15) {
            logPrintf("\n[ProControllerSniffer] 正在切换至扫描模式...\n");

            // 1. 停止广播，防止新的连接进来
            BLEDevice::getAdvertising()->stop();

            // 2. 获取服务器实例并优雅地断开所有已连接的手机
            BLEServer* pServer = BLEDevice::getServer();
            if (pServer) {
                // 传入 false 是因为我们是 Server，需要获取连接到我们的 Client 列表
                std::map<uint16_t, conn_status_t> peerList = pServer->getPeerDevices(false);

                if (!peerList.empty()) {
                    logPrintf("检测到 %d 个活动连接，正在断开...\n", peerList.size());
                    for (auto const& [connId, status] : peerList) {
                        pServer->disconnect(connId);
                    }
                    // 重要：给手机端和协议栈 500ms 时间处理“分手”事务，防止内存野指针
                    delay(500);
                }
            }

            // 3. 准备扫描器 (不调 deinit，直接复用当前堆栈)
            BLEScan* pBLEScan = BLEDevice::getScan();

            // 重新挂载回调（原生库会自动覆盖旧回调）
            pBLEScan->setAdvertisedDeviceCallbacks(new SnifferCallbacks(), true);

            // 采用被动扫描以节省 CPU 并提高稳定性
            pBLEScan->setActiveScan(false);
            pBLEScan->setInterval(150);
            pBLEScan->setWindow(120);

            showBlueLed();
            logPrintf("👉 扫描启动 (持续 %d 秒)，请按下手柄 Home 键...\n", durationSec);

            // 4. 启动扫描 (由于没有 deinit，这里不会 Panic)
            pBLEScan->start(durationSec, false);

            logPrintf("\n[ProControllerSniffer] 扫描周期结束。\n");
            showRedLed();
            pBLEScan->clearResults();

            // 如果扫描完想恢复小程序连接，可以在这里重启广播
            // BLEDevice::getAdvertising()->start();
        }
};

#endif