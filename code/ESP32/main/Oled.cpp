#include "Oled.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Log.h"
#include "defines.h"

extern Adafruit_SSD1306 oled_display;

namespace CLASSICDIY {

void Oled::update(uint16_t level, relayStatus status) {
   oled_display.clearDisplay();
   oled_display.setTextSize(STATUS_FONT);
   oled_display.setTextColor(SSD1306_WHITE);
   oled_display.setCursor(32, STATUS_Y);
   char buffer[64];
   if (status == off)
      strcpy(buffer, "Stop      ");
   else if (status == stop)
      strcpy(buffer, "Run       ");
   else if (status == slead)
      strcpy(buffer, "Start lead");
   else if (status == slag)
      strcpy(buffer, "Start lag ");
   else if (status == overflow)
      strcpy(buffer, "Overflow");
   oled_display.setCursor(xOffset(2, strlen(buffer)), STATUS_Y);
   oled_display.print(buffer);
   // logd("State: %s", buffer);
   sprintf(buffer, "%d%%", level);
   // logd("Level: %s", buffer);
   oled_display.setCursor(xOffset(5, strlen(buffer)), LEVEL_Y);
   oled_display.setTextSize(LEVEL_FONT);
   oled_display.setTextColor(SSD1306_WHITE);
   oled_display.print(buffer);
   oled_display.display();
}

uint8_t Oled::xOffset(uint8_t textSize, uint8_t numberOfCharaters) {
   uint8_t textPixels = textSize * 6;
   uint8_t rVal = (SCREEN_WIDTH - (numberOfCharaters * textPixels)) / 2;
   return rVal;
}
} // namespace CLASSICDIY