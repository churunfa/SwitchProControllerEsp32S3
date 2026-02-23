//
// Created by churunfa on 2026/2/24.
//

#ifndef SWITCHPROCONTROLLERESP32S3_WAKEUPTRIGGERREADER_H
#define SWITCHPROCONTROLLERESP32S3_WAKEUPTRIGGERREADER_H

#include <ble/SwitchWakeUp.h>
#include <debug/log.h>
#include <debug/led_control.h>

#include "ReadStrategy.h"

class WakeUpTriggerReader : public ReadStrategy {
    // 唤醒触发命令不需要额外数据，只需要触发信号
    // 可以添加简单的参数验证或确认码
    
public:
    void readData(const uint8_t inByte) override {
        // 唤醒触发不需要读取数据，但可以用来做简单验证
        // 例如：要求发送特定的确认字节
    }

    void exec() override {
        logPrintf("WakeUpTriggerReader: 收到唤醒触发命令\n");
        
        // 调用SwitchWakeUp的trigger方法
        SwitchWakeUp::getInstance().trigger();
        
        logPrintf("WakeUpTriggerReader: 唤醒命令已执行\n");
    }

    void reset() override {
        // 重置状态
    }

    int length() override {
        // 唤醒触发命令长度为0（或者可以设置为1用于简单验证）
        return 0;
    }
};

#endif //SWITCHPROCONTROLLERESP32S3_WAKEUPTRIGGERREADER_H