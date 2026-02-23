#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <debug/log.h>

#include "debug/led_control.h"
#include "switch/driver/SwitchProDriver.h"
#include "read/ReadStrategyProcess.h"
#include "graph/Graph.h"
#include "ble/NativeBLEReader.h"
#include "ble/SwitchWakeUp.h"

NativeBLEReader native_ble_reader;

void setup() {
    Serial0.setRxBufferSize(8192);
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

    SwitchWakeUp::getInstance().begin();
    native_ble_reader.begin("vSwitch Pro Controller");
}

int switch_running_boot_btn_status = HIGH;
unsigned long boot_hold_time = 0;

int count = 0;
void loop() {
    if (const int cur_boot_status = digitalRead(BOOT_PIN); cur_boot_status != switch_running_boot_btn_status) {
        if (cur_boot_status == LOW) {
            boot_hold_time = millis();
        } else {
            boot_hold_time = millis() - boot_hold_time;
            if (boot_hold_time > 2000) {
                GraphExecutor::connectGamepad();
            } else if (boot_hold_time > 50) {
                GraphExecutor::getInstance().switchRunning();
            }
        }
        switch_running_boot_btn_status = cur_boot_status;
    }

    while (Serial0.available() > 0) {
        ReadStrategyProcess::getInstance().process(Serial0.read());
    }
}