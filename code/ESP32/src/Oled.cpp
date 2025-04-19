#include "Oled.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Log.h"
#include "defines.h"

namespace HydroFloat
{

    Oled::Oled()    {
        Wire.begin(SDA_PIN, SLC_PIN);
        _oled = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    }

    bool Oled::begin()    {
        _initialized = true;
        if(!_oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
        {
            loge("OLED Display - SSD1306 allocation failed");
            _initialized = false;
        }
        return _initialized;
    }
    void Oled::update(uint16_t level, relayStatus status)    {
        if (!_initialized)
        {
            loge("OLED Display - Not initialized");
            return;
        }
        _oled.clearDisplay();
        _oled.setTextSize(STATUS_FONT);         
        _oled.setTextColor(SSD1306_WHITE);       
        _oled.setCursor(32,STATUS_Y);  
        char buffer[64];
        if(status == off)
            strcpy(buffer, "Stop      ");
        else if(status == stop)
            strcpy(buffer, "Run       ");
        else if(status == slead)
            strcpy(buffer, "Start lead");
        else if(status == slag)
            strcpy(buffer, "Start lag ");        
        else if(status == overflow)
            strcpy(buffer, "Overflow");
        _oled.setCursor(xOffset(2, strlen(buffer)), STATUS_Y);
        _oled.print(buffer);
        // logd("State: %s", buffer);
        sprintf(buffer, "%d%%", level);
        // logd("Level: %s", buffer);
        _oled.setCursor(xOffset(5, strlen(buffer)), LEVEL_Y); 
        _oled.setTextSize(LEVEL_FONT);         
        _oled.setTextColor(SSD1306_WHITE);
        _oled.print(buffer);
        _oled.display();
    }

    uint8_t Oled::xOffset(uint8_t textSize, uint8_t numberOfCharaters) {
        uint8_t textPixels = textSize * 6;
        uint8_t rVal =  (SCREEN_WIDTH - (numberOfCharaters * textPixels)) / 2;
        // logd("Offset: %d", rVal);
        return rVal;

    }
}