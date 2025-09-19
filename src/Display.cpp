#include "Display.h"

// Cria a instância global do display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void DisplayInit()
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
}

void DisplayWrite(std::string text)
{
  display.clearDisplay();

  display.setTextSize(2);
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
