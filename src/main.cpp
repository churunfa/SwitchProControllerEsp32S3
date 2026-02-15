#include <Arduino.h>

#include "debug/led_control.h"
#include "switch/driver/SwitchProDriver.h"
#include "read/ReadStrategyProcess.h"

void setup() {
    Serial0.begin(3000000);
    resetLed();
    SwitchProDriver::getInstance();
}

void loop() {
    if (Serial0.available() > 0) {
        const uint8_t inByte = Serial0.read();
        ReadStrategyProcess::getInstance().process(inByte);
    }
}