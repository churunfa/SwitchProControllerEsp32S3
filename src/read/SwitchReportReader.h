//
// Created by churunfa on 2026/2/4.
//

#ifndef SWITCHPROCONTROLLERESP32S3_SWITCHREPORTREADER_H
#define SWITCHPROCONTROLLERESP32S3_SWITCHREPORTREADER_H

#include "ReadStrategy.h"

class SwitchReportReader : public ReadStrategy {
    SwitchProSerialInput serialInput = {};
    uint8_t readIndex = 0;

    void setSerialInput(const uint8_t index, const uint8_t byte) {
        reinterpret_cast<uint8_t *>(&serialInput)[index] = byte;
    }

public:
    void readData(const uint8_t inByte) override {
        setSerialInput(readIndex++, inByte);
    }

    void exec() override {
        // 读完数据了，更新输入
        if (SwitchProDriver::getInstance().updateInputReport(&serialInput)) {
            // 如果有更新，立刻发送一次数据
            SwitchProDriver::getInstance().process(true);
        }
        readIndex = 0;
    }

    void reset() override {
        readIndex = 0;
    }

    int8_t length() override {
        return sizeof(SwitchProSerialInput);
    }
};
#endif //SWITCHPROCONTROLLERESP32S3_SWITCHREPORTREADER_H