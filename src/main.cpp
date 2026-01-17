#include <Arduino.h>

#include "debug/led_control.h"
#include "switch/driver/SwitchProDriver.h"

SwitchProDriver switchProDriver;

void setup() {
    Serial0.begin(115200);
    resetLed();
    switchProDriver.initialize();
}

SwitchProSerialInput serialInput = {0};
uint8_t readSize = sizeof(SwitchProSerialInput);
uint8_t readyIndex = 0;

uint8_t header[] = {0xAA, 0x55};
// 0 1 -> 查找对应下标的数据
// 2 读数据
uint8_t headerIndex = 0;

void setSerialInput(const uint8_t index, const uint8_t byte) {
    reinterpret_cast<uint8_t *>(&serialInput)[index] = byte;
}

bool verifyCheckSum(const uint8_t checkSum) {
    uint8_t xorResult = 0;
    for (uint8_t i = 0; i < readSize; i++) {
        xorResult ^= reinterpret_cast<uint8_t *>(&serialInput)[i];
    }
    return xorResult == checkSum;
}

unsigned long last_report_timer = millis();
// 打印当前状态
bool btn = false;

void loop() {
    if (Serial0.available() > 0) {
        const uint8_t inByte = Serial0.read();
        if (headerIndex < 2) {
            if (inByte == header[headerIndex]) {
                // 如果匹配上，继续匹配下一个
                headerIndex++;
                return;
            }
            showRedLed();
            // 如果匹配不上，需要从头匹配
            headerIndex = 0;
            return;
        }

        if (readyIndex < readSize) {
            // 还没读完，继续读
            setSerialInput(readyIndex++, inByte);
            return;
        }
        if (readyIndex == readSize) {
            // 检查校验和
            if (!verifyCheckSum(inByte)) {
                // 校验和不合法，重新找header
                headerIndex = 0;
                readyIndex = 0;
                showYellowLed();
                return;
            }
            // 读完数据了，更新输入
            switchProDriver.updateInputReport(&serialInput);
            // 重置index
            headerIndex = 0;
            readyIndex = 0;
        }
    }

    switchProDriver.process();
}