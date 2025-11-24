
#pragma once
#ifdef Has_OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Enumerations.h"

namespace CLASSICDIY
{
    #define LEVEL_FONT 5
    #define STATUS_FONT 2

    #define LEVEL_Y 24
    #define STATUS_Y 0

    class Oled
    {
    public:
        void update(uint16_t level, const char* state);

    private:
        uint8_t xOffset(uint8_t textSize, uint8_t numberOfCharaters);
    };

}
#endif