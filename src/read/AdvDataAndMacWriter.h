//
// Created by churunfa on 2026/2/24.
//

#ifndef SWITCHPROCONTROLLERESP32S3_ADVDATAANDMACWRITER_H
#define SWITCHPROCONTROLLERESP32S3_ADVDATAANDMACWRITER_H

#include <config/SimpleConfig.h>
#include <debug/log.h>
#include <debug/led_control.h>

#include "AbstractLongReader.h"

class AdvDataAndMacWriter : public AbstractLongReader {
    // 数据格式：前6字节为MAC地址，后面为广播数据
    uint8_t macAddress[6] = {0};
    std::vector<uint8_t> advDataBuffer;
    bool macReceived = false;
    int advDataLength = 0;
    
public:
    void read_completed_exec(std::vector<uint8_t> buffer) override {
        if (buffer.size() < 6) {
            logPrintf("AdvDataAndMacWriter: 数据长度不足，至少需要6字节MAC地址\n");
            showRedLed();
            return;
        }
        
        // 提取MAC地址（前6字节）
        memcpy(macAddress, buffer.data(), 6);
        macReceived = true;
        
        // 提取广播数据（剩余部分）
        if (buffer.size() > 6) {
            advDataLength = buffer.size() - 6;
            advDataBuffer.assign(buffer.begin() + 6, buffer.end());
        } else {
            advDataLength = 0;
            advDataBuffer.clear();
        }
        
        // 写入配置到SimpleConfig
        auto& config = SimpleConfig::getInstance();
        bool configSuccess = true;
        std::string errorMsg = "";
                
        try {
            // 写入MAC地址配置
            std::vector<uint8_t> macVector(macAddress, macAddress + 6);
            if (!config.setConfig(ConfigType::MAC_ADDRESS, macVector)) {
                errorMsg = "MAC地址配置保存失败";
                configSuccess = false;
            } else {
                logPrintf("AdvDataAndMacWriter: MAC地址写入成功: %02X:%02X:%02X:%02X:%02X:%02X\n",
                         macAddress[0], macAddress[1], macAddress[2], 
                         macAddress[3], macAddress[4], macAddress[5]);
            }
                    
            // 写入广播数据配置
            if (advDataLength == 31) {
                if (!config.setConfig(ConfigType::NS2_WAKE_DATA, advDataBuffer)) {
                    errorMsg = "广播数据配置保存失败";
                    configSuccess = false;
                } else {
                    logPrintf("AdvDataAndMacWriter: 广播数据写入成功，长度: %d\n", advDataLength);

                    // 打印广播数据内容
                    Serial0.printf("AdvData内容: ");
                    for (int i = 0; i < advDataLength; i++) {
                        Serial0.printf("%02X ", advDataBuffer[i]);
                    }
                    Serial0.printf("\n");
                }
            } else {
                errorMsg = "广播数据长度异常: " + std::to_string(advDataLength) + " (应为31字节)";
                configSuccess = false;
            }

            // 验证写入结果
            if (configSuccess) {
                // 验证配置是否正确保存
                std::vector<uint8_t> savedMac;
                std::vector<uint8_t> savedAdvData;
                bool macVerified = config.getConfig(ConfigType::MAC_ADDRESS, savedMac);
                bool advDataVerified = (advDataLength == 0) || config.getConfig(ConfigType::NS2_WAKE_DATA, savedAdvData);
                logPrintf("AdvDataAndMacWriter: 配置验证结果: macVerified=%d,advDataVerified=%d\n", macVerified, advDataVerified);
                if (macVerified && advDataVerified) {
                    // 检查数据一致性
                    if (savedMac.size() == 6 && memcmp(savedMac.data(), macAddress, 6) == 0) {
                        if (advDataLength == 0 || (savedAdvData.size() == advDataLength && 
                            memcmp(savedAdvData.data(), advDataBuffer.data(), advDataLength) == 0)) {
                            logPrintf("AdvDataAndMacWriter: 配置验证成功，所有数据已正确保存\n");
                            // 显示成功状态（可以考虑用不同颜色的LED表示成功）
                        } else {
                            errorMsg = "配置数据验证失败：保存的数据与原始数据不一致";
                            configSuccess = false;
                        }
                    } else {
                        errorMsg = "MAC地址验证失败：保存的MAC地址与原始数据不一致";
                        configSuccess = false;
                    }
                    logPrintf("AdvDataAndMacWriter: adv data写入成功：");
                                for(int i = 0; i < advDataLength; i++) {
                                    logPrintf("%02X,", advDataBuffer[i]);
                                }
                                logPrintf("\n");
                } else {
                    errorMsg = "配置验证失败：无法读取已保存的配置";
                    configSuccess = false;
                }
            }
                    
            // 根据结果设置LED状态
            if (configSuccess) {
                // 可以考虑添加绿色LED表示完全成功
                logPrintf("AdvDataAndMacWriter: 所有配置写入并验证成功\n");
            } else {
                logPrintf("AdvDataAndMacWriter: 配置写入失败: %s\n", errorMsg.c_str());
                showRedLed();
            }

        } catch (const std::exception& e) {
            logPrintf("AdvDataAndMacWriter: 配置写入异常: %s\n", e.what());
            showRedLed();
        } catch (...) {
            logPrintf("AdvDataAndMacWriter: 配置写入发生未知异常\n");
            showRedLed();
        }
    }
    
    void reset() override {
        AbstractLongReader::reset();
        macReceived = false;
        advDataLength = 0;
        advDataBuffer.clear();
        memset(macAddress, 0, sizeof(macAddress));
    }
};

#endif //SWITCHPROCONTROLLERESP32S3_ADVDATAANDMACWRITER_H