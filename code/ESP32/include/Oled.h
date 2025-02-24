
#pragma once
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Enumerations.h"

namespace HydroFloat
{
    #define LEVEL_FONT 5
    #define STATUS_FONT 2

    #define LEVEL_Y 24
    #define STATUS_Y 0

    class Oled
    {
    public:
        Oled();
        bool begin();
        void update(uint16_t level, relayStatus status);

    private:
        Adafruit_SSD1306 _oled;
        bool _initialized;
        uint8_t xOffset(uint8_t textSize, uint8_t numberOfCharaters);
    };

}