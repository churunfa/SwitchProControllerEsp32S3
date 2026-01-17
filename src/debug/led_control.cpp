// --- 引脚与硬件配置 ---
#define PIN_RGB 48  
#define NUMPIXELS 1
#include "led_control.h"

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_RGB, NEO_GRB + NEO_KHZ800);

void setupLeds() {
    pixels.begin();
    pixels.setBrightness(50); // 建议设置亮度，防止过亮
}
void showRedLed() {
    pixels.setPixelColor(0, Adafruit_NeoPixel::Color(64, 0, 0)); // 红色
    pixels.show();
}


void showYellowLed() {
    pixels.setPixelColor(0, Adafruit_NeoPixel::Color(64, 64, 0)); // 黄色
    pixels.show();
}

void showBlueLed() {
    pixels.setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 64)); // 蓝色
    pixels.show();
}

void resetLed() {
    pixels.setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 0));
    pixels.show();
}