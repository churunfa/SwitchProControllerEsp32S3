//
// Created by churunfa on 2026/2/24.
//

#ifndef SWITCHPROCONTROLLERESP32S3_SIMPLEOPERATEREADER_H
#define SWITCHPROCONTROLLERESP32S3_SIMPLEOPERATEREADER_H

#include <ble/SwitchWakeUp.h>
#include <debug/log.h>
#include <debug/led_control.h>
#include <ESP.h>

#include "ReadStrategy.h"
#include "ble/ProControllerSniffer.h"

class SimpleOperateReader : public ReadStrategy {
private:
    uint8_t operationType = 0;
    uint8_t readIndex = 0;
    
public:
    void readData(const uint8_t inByte) override {
        if (readIndex == 0) {
            operationType = inByte;
            readIndex++;
        }
    }

    void exec() override {
        switch (operationType) {
            case 0: // 唤醒操作
                SwitchWakeUp::getInstance().trigger();
                break;
            case 1: // 重启操作
                ESP.restart();
                break;
            case 2: // 连接手柄
                GraphExecutor::getInstance().connectGamepad();
                break;
            case 3: // 拓扑图循环
                GraphExecutor::getInstance().switchRunning(true);
                break;
            case 4: // 拓扑图停止
                GraphExecutor::getInstance().switchRunning(false);
                break;
            case 5: // 唤醒配置扫描
                ProControllerSniffer::getInstance().pendingScan = true;
                break;
            default:
                break;
        }
    }

    void reset() override {
        readIndex = 0;
        operationType = 0;
    }

    int length() override {
        return 1; // 需要读取1个字节作为操作类型
    }
};

#endif //SWITCHPROCONTROLLERESP32S3_SIMPLEOPERATEREADER_H