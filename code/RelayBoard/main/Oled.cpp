#ifdef Has_OLED
#include "Oled.h"
#include "Log.h"
#include "defines.h"

extern Adafruit_SSD1306 oled_display;

namespace CLASSICDIY {

void Oled::update(uint16_t level, const char* pch) {
   oled_display.clearDisplay();
   oled_display.setTextSize(STATUS_FONT);
   oled_display.setTextColor(SSD1306_WHITE);
   oled_display.setCursor(32, STATUS_Y);
   String state = pch;
   while (state.length() < 10) { // pad right to 10 characters to fill the display line
      state += ' ';
   }
   oled_display.setCursor(xOffset(2, state.length()), STATUS_Y); // center it 
   oled_display.print(state.c_str());
   char buffer[64];
   sprintf(buffer, "%d%%", level);
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

#endif