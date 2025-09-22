#include "Display.h"

// Cria a instância global do display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int fontSize = 2;

void DisplayInit(uint8_t size)
{
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println(F("Falha ao iniciar o display OLED"));
    for (;;)
      ; // trava se não encontrar o display
  }

  display.clearDisplay();
  display.display();
  display.setTextSize(size);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
}

void DisplaySetFontSize(uint8_t size)
{
  fontSize = size;
  display.setTextSize(size);
}

void DisplayWrite(std::string text, int16_t x, int16_t y)
{
  display.setTextColor(WHITE);
  display.setCursor(x, y);
  display.println(text.c_str());

  display.display();
}

void DisplayCenteredWrite(std::string text)
{
  display.setTextColor(WHITE);

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text.c_str(), 0, 0, &x1, &y1, &w, &h);

  int16_t x = (SCREEN_WIDTH - w) / 2;
  int16_t y = (SCREEN_HEIGHT - h) / 2;

  display.setCursor(x, y);
  display.println(text.c_str());

  display.display();
}

void DisplayClear()
{
  display.clearDisplay();
  display.display();
}

Rect *DisplayGetTextBounds(std::string text)
{
  Rect *bounds = new Rect;
  display.getTextBounds(text.c_str(), 0, 0, &bounds->x, &bounds->y, &bounds->w, &bounds->h);
  bounds->x = (SCREEN_WIDTH - bounds->w) / 2;
  bounds->y = (SCREEN_HEIGHT - bounds->h) / 2;
  return bounds;
}
