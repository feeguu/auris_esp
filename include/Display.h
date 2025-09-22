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

typedef struct Rect
{
  int16_t x;
  int16_t y;
  uint16_t w;
  uint16_t h;
} Rect;

void DisplayInit(uint8_t size);
void DisplayWrite(std::string text, int16_t x, int16_t y);
void DisplayCenteredWrite(std::string text);
Rect *DisplayGetTextBounds(std::string text);
void DisplayClear();

#endif
