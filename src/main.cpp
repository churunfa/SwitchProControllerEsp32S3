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
#include "config/SimpleConfig.h"
#include "ble/ProControllerSniffer.h"

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

    logPrintf("LittleFS Total: %u bytes\n", LittleFS.totalBytes());
    logPrintf("LittleFS Used: %u bytes\n", LittleFS.usedBytes());

    SimpleConfig::getInstance().initialize();
    SwitchProDriver::getInstance();
    GraphExecutor::getInstance();

    SwitchWakeUp::getInstance().begin();
    native_ble_reader.begin("vSwitch Pro Controller");
}

int count = 0;
void loop() {
    if (ProControllerSniffer::getInstance().pendingScan) {
        ProControllerSniffer::getInstance().startDetection(10);
        ESP.restart();
    }
    while (Serial0.available() > 0) {
        ReadStrategyProcess::getInstance().process(Serial0.read());
    }
}