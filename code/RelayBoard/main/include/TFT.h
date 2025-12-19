
#pragma once
#ifdef Has_TFT
#include <TFT_eSPI.h> // Graphics library
#include "IDisplayServiceInterface.h"
#include "Enumerations.h"
#include "Thresholds.h"

namespace CLASSICDIY {

// Fonts
#define FF17 &FreeSans9pt7b
#define FF18 &FreeSans12pt7b
#define FF19 &FreeSans18pt7b
#define FF20 &FreeSans24pt7b

#define FSSB9 &FreeSansBold9pt7b
#define FSSB12 &FreeSansBold12pt7b
#define FSSB18 &FreeSansBold18pt7b
#define FSSB24 &FreeSansBold24pt7b

// Meter geometry
#define METER_WIDTH 320
#define METER_HEIGHT 160
#define METER_FONT 1
#define LED_Y 15
#define LED_WIDTH (METER_WIDTH / 6)
#define TEXT_Y (METER_HEIGHT + LED_Y)
#define TEXT_MARGIN 10
// Derived values
#define METER_XCENTER (METER_WIDTH / 2)     // horizontal center
#define METER_YCENTER (METER_HEIGHT + 14)   // vertical center below box
#define METER_RADIUS ((METER_HEIGHT * 0.8)) // radius scaled to height

#define METER_TICK_LONG 15
#define METER_TICK_SHORT 8

#define METER_MARGIN_X 5
#define METER_MARGIN_Y 3
#define METER_INNER_W (METER_WIDTH - 2 * METER_MARGIN_X)
#define METER_INNER_H (METER_HEIGHT - 2 * METER_MARGIN_Y)

// Zone colour configuration
#define ZONE_START -50
#define ZONE_END 50
const int ZoneColors[] = {TFT_WHITE, TFT_YELLOW, TFT_GREEN, TFT_ORANGE, TFT_CYAN, TFT_PINK, TFT_RED};

#define LEVEL_FONT 5
#define STATUS_FONT 2
#define HDR_FONT 2
#define MODE_FONT 2
#define DETAIL_FONT 1
#define NumChar(font) SCREEN_WIDTH / (6 * font)

#define LEVEL_Y 24
#define STATUS_Y 0
#define BUF_SIZE 32

struct TFTHeaderCache {
   String hdr1;
   String detail1;
   String hdr2;
   String detail2;
   int count = -1; // use -1 to indicate “no previous count”
   String status;
   String levelStr;
};

class TFT : public IDisplayServiceInterface {
 public:
   void Init();
   void Display(const char *hdr1, const char *detail1, const char *hdr2, const char *detail2);
   void Display(const char *hdr1, const char *detail1, const char *hdr2, int count);
   void AnalogMeter(std::vector<Thresholds> &thresholds);
   void Update(const char *state, uint16_t level);

 private:
   void plotNeedle(int value, byte ms_delay);
   uint16_t _hSplit = 70;
   std::vector<Thresholds> _thresholds;
   float ltx = 0;                 // Saved x coord of bottom of needle
   uint16_t osx = 120, osy = 120; // Saved x & y coords
   int value[6] = {0, 0, 0, 0, 0, 0};
   int old_value[6] = {-1, -1, -1, -1, -1, -1};
   int old_analog = -999; // Value last displayed
   void drawIfChanged(const String &newVal, String &oldVal, int x, int y, uint16_t color);
   TFTHeaderCache _headerCache;
};

} // namespace CLASSICDIY
#endif