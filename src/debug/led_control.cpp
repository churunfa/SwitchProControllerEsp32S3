// --- 引脚与硬件配置 ---
#define PIN_RGB 48  
#define NUMPIXELS 1
#include "led_control.h"

Adafruit_NeoPixel pixels(NUMPIXELS, PIN_RGB, NEO_GRB + NEO_KHZ800);

uint32_t lastColor = -1;

void setupLeds() {
    pixels.begin();
    pixels.setBrightness(5);
}
void showRedLed() {
    if (const uint32_t newColor = Adafruit_NeoPixel::Color(64, 0, 0); lastColor != newColor) {
        pixels.setPixelColor(0, newColor);
        pixels.show();
        lastColor = newColor;
    }
}


void showYellowLed() {
    if (const uint32_t newColor = Adafruit_NeoPixel::Color(64, 64, 0); lastColor != newColor) {
        pixels.setPixelColor(0, newColor);
        pixels.show();
        lastColor = newColor;
    }
}

void showBlueLed() {
    if (const uint32_t newColor = Adafruit_NeoPixel::Color(0, 0, 64); lastColor != newColor) {
        pixels.setPixelColor(0, newColor);
        pixels.show();
        lastColor = newColor;
    }
}

void resetLed() {
    if (const uint32_t newColor = Adafruit_NeoPixel::Color(0, 0, 0); lastColor != newColor) {
        pixels.setPixelColor(0, newColor);
        pixels.show();
        lastColor = newColor;
    }
}