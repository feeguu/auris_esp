#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string>

// ===== Configurações do display =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 8
#define SCL_PIN 9
#define OLED_ADDR 0x3C

extern Adafruit_SSD1306 display;

void DisplayInit();
void DisplayWrite(std::string text);

#endif
