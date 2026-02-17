#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <debug/log.h>

#include "debug/led_control.h"
#include "switch/driver/SwitchProDriver.h"
#include "read/ReadStrategyProcess.h"
#include "graph/Graph.h"

void setup() {
    Serial0.begin(3000000);
    resetLed();

    // 挂载 LittleFS
    logPrintf("Mounting LittleFS...");
    if(!LittleFS.begin(true)){
        printf("LittleFS Mount Failed");
        return;
    }

    // 打印一下文件系统信息，确认是否真的有 9MB
    logPrintf("LittleFS Total: %u bytes\n", LittleFS.totalBytes());
    logPrintf("LittleFS Used: %u bytes\n", LittleFS.usedBytes());
    SwitchProDriver::getInstance();
    GraphExecutor::getInstance();

    pinMode(BOOT_PIN, INPUT_PULLUP);
}

int boot_btn_status = HIGH;

void loop() {
    if (digitalRead(BOOT_PIN) != boot_btn_status) {
        if (digitalRead(BOOT_PIN) == LOW) {
            boot_btn_status = LOW;
            // 按下
            GraphExecutor::getInstance().setRunning(true);
        }
        if (digitalRead(BOOT_PIN) == HIGH) {
            boot_btn_status = HIGH;
            // 抬起
            GraphExecutor::getInstance().setRunning(false);
        }
    }

    if (Serial0.available() > 0) {
        const uint8_t inByte = Serial0.read();
        ReadStrategyProcess::getInstance().process(inByte);
    }
}