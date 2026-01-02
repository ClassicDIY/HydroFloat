#ifdef Has_TFT
#include "TFT.h"
#include "Log.h"

TFT_eSPI tft = TFT_eSPI(); // Invoke library

void TFT::Init() {
   tft.init();
   tft.setRotation(1); // Landscape
   tft.fillScreen(TFT_BLACK);
   logd("Screen init: Width %d, Height %d", tft.width(), tft.height());
}

void TFT::Display(const char *hdr1, const char *detail1, const char *hdr2, int count)
{
    tft.setTextFont(1);
    int y = 0;
    // hdr1
    tft.setTextSize(HDR_FONT);
    drawIfChanged(String(hdr1), _headerCache.hdr1, 0, y, TFT_GREEN);
    y += 18;
    // detail1
    tft.setTextSize(DETAIL_FONT);
    drawIfChanged(String(detail1), _headerCache.detail1, 0, y, TFT_GREEN);
    y += 18;
    // hdr2 + count
    tft.setTextSize(MODE_FONT);
    String hdr2Full = String(hdr2);
    if (count > 0) {
        hdr2Full += ":" + String(count);
    }
    drawIfChanged(hdr2Full, _headerCache.hdr2, 0, y, TFT_GREEN);
    _headerCache.count = count;
}

void TFT::Display(const char *hdr1, const char *detail1, const char *hdr2, const char *detail2)
{
    tft.setTextFont(1);
    int y = 0;
    // hdr1
    tft.setTextSize(HDR_FONT);
    drawIfChanged(String(hdr1), _headerCache.hdr1, 0, y, TFT_GREEN);
    y += 18;
    // detail1
    tft.setTextSize(DETAIL_FONT);
    drawIfChanged(String(detail1), _headerCache.detail1, 0, y, TFT_GREEN);
    y += 18;
    // hdr2
    tft.setTextSize(MODE_FONT);
    drawIfChanged(String(hdr2), _headerCache.hdr2, 0, y, TFT_GREEN);
    y += 18;
    // detail2
    tft.setTextSize(DETAIL_FONT);
    drawIfChanged(String(detail2), _headerCache.detail2, 0, y, TFT_GREEN);
}

void TFT::drawIfChanged(const String& newVal, String& oldVal, int x, int y, uint16_t color)
{
    if (newVal != oldVal) {
        tft.setTextColor(TFT_BLACK, TFT_BLACK); // erases old text
        tft.drawString(oldVal, x, y);
        tft.setTextColor(color, TFT_BLACK); // new text
        tft.drawString(newVal, x, y);
        oldVal = newVal;
    }
}

void TFT::Update(const char *state, uint16_t level) {
   if (_thresholds.size() > 0) { // displaying analog meter?
      plotNeedle(level, 0);
      tft.setTextColor(TFT_YELLOW);
      tft.setFreeFont(FF20);
      tft.fillRect(0, TEXT_Y, METER_WIDTH, TFT_HEIGHT - TEXT_Y, TFT_DARKGREY);
      tft.drawCentreString(state, METER_XCENTER, TEXT_Y + TEXT_MARGIN, METER_FONT);
   }
}

void TFT::plotNeedle(int value, byte ms_delay) {
   tft.setTextColor(TFT_BLACK, TFT_WHITE);
   char buf[8];
   dtostrf(value, 4, 0, buf);
   strcat(buf, "%");
   // Move the needle until new value reached
   while (old_analog != value) {
      if (old_analog < value)
         old_analog++;
      else
         old_analog--;
      if (ms_delay == 0)
         old_analog = value; // Update immediately if delay is 0
      // Map value to angle (-10..110 → -150..-30 degrees)
      float sdeg = map(old_analog, -10, 110, -150, -30);
      // Needle tip coords
      float sx = cos(sdeg * 0.0174532925);
      float sy = sin(sdeg * 0.0174532925);
      // Needle base offset (not pivoting exactly at center)
      float tx = tan((sdeg + 90) * 0.0174532925);
      // Erase old needle
      tft.drawLine(METER_XCENTER + 20 * ltx - 1, METER_YCENTER - 20, osx - 1, osy, TFT_WHITE);
      tft.drawLine(METER_XCENTER + 20 * ltx, METER_YCENTER - 20, osx, osy, TFT_WHITE);
      tft.drawLine(METER_XCENTER + 20 * ltx + 1, METER_YCENTER - 20, osx + 1, osy, TFT_WHITE);
      // Re‑plot text under needle
      tft.setTextColor(TFT_DARKCYAN);
      tft.setFreeFont(FSSB18);
      uint16_t cwidth = tft.textWidth(buf, METER_FONT);
      uint16_t cheight = tft.fontHeight(METER_FONT);
      tft.fillRect(METER_XCENTER - (cwidth / 2) + 5, METER_YCENTER - METER_RADIUS + cheight, cwidth, cheight, TFT_WHITE);
      tft.drawCentreString(buf, METER_XCENTER, METER_YCENTER - METER_RADIUS + cheight, METER_FONT);
      // Store new needle coords
      ltx = tx;
      osx = sx * (METER_RADIUS - 2) + METER_XCENTER;
      osy = sy * (METER_RADIUS - 2) + METER_YCENTER;
      // Draw new needle (3 lines for thickness)
      tft.drawLine(METER_XCENTER + 20 * ltx - 1, METER_YCENTER - 20, osx - 1, osy, TFT_RED);
      tft.drawLine(METER_XCENTER + 20 * ltx, METER_YCENTER - 20, osx, osy, TFT_MAGENTA);
      tft.drawLine(METER_XCENTER + 20 * ltx + 1, METER_YCENTER - 20, osx + 1, osy, TFT_RED);
      // Slow needle slightly near target
      if (abs(old_analog - value) < 10)
         ms_delay += ms_delay / 5;
      delay(ms_delay);
   }
   // Update Threshold label and led color
   int index = 0;
   for (auto &rule : _thresholds) {
      int ledx = LED_WIDTH * index++;
      int th = rule.threshold;
      tft.fillRect(ledx, METER_HEIGHT, LED_WIDTH, LED_Y, th > value ? TFT_GREEN : TFT_RED);
      tft.drawRect(ledx, METER_HEIGHT, LED_WIDTH, LED_Y, TFT_BLACK);
   }
}

void TFT::AnalogMeter(std::vector<Thresholds> &thresholds) {
   if (_thresholds.size() > 0) {
      // already displayed
      return;
   }
   _thresholds = thresholds;
   // Meter outline
   tft.fillRect(0, 0, METER_WIDTH, METER_HEIGHT, TFT_SILVER);
   tft.fillRect(METER_MARGIN_X, METER_MARGIN_Y, METER_INNER_W, METER_INNER_H, TFT_WHITE);
   tft.setTextColor(TFT_BLACK);
   // Draw ticks every 5 degrees from -50 to +50
   for (int i = -50; i < 51; i += 5) {
      int tl = METER_TICK_LONG;
      float sx = cos((i - 90) * 0.0174532925);
      float sy = sin((i - 90) * 0.0174532925);
      uint16_t x0 = sx * (METER_RADIUS + tl) + METER_XCENTER;
      uint16_t y0 = sy * (METER_RADIUS + tl) + METER_YCENTER;
      uint16_t x1 = sx * METER_RADIUS + METER_XCENTER;
      uint16_t y1 = sy * METER_RADIUS + METER_YCENTER;
      float sx2 = cos((i + 5 - 90) * 0.0174532925);
      float sy2 = sin((i + 5 - 90) * 0.0174532925);
      int x2 = sx2 * (METER_RADIUS + tl) + METER_XCENTER;
      int y2 = sy2 * (METER_RADIUS + tl) + METER_YCENTER;
      int x3 = sx2 * METER_RADIUS + METER_XCENTER;
      int y3 = sy2 * METER_RADIUS + METER_YCENTER;
      // Zone fill based on thresholds

      int index = 0;
      int t1 = _thresholds[index].threshold - 50;
      if (i >= ZONE_START && i < t1) {
         tft.fillTriangle(x0, y0, x1, y1, x2, y2, ZoneColors[index]);
         tft.fillTriangle(x1, y1, x2, y2, x3, y3, ZoneColors[index]);
      }
      index++;
      int t2 = _thresholds[index].threshold - 50;
      if (i >= t1 && i < t2) {
         tft.fillTriangle(x0, y0, x1, y1, x2, y2, ZoneColors[index]);
         tft.fillTriangle(x1, y1, x2, y2, x3, y3, ZoneColors[index]);
      }
      index++;
      int t3 = _thresholds[index].threshold - 50;
      if (i >= t2 && i < t3) {
         tft.fillTriangle(x0, y0, x1, y1, x2, y2, ZoneColors[index]);
         tft.fillTriangle(x1, y1, x2, y2, x3, y3, ZoneColors[index]);
      }
      index++;
      int t4 = _thresholds[index].threshold - 50;
      if (i >= t3 && i < t4) {
         tft.fillTriangle(x0, y0, x1, y1, x2, y2, ZoneColors[index]);
         tft.fillTriangle(x1, y1, x2, y2, x3, y3, ZoneColors[index]);
      }
      index++;
      int t5 = _thresholds[index].threshold - 50;
      if (i >= t4 && i < t5) {
         tft.fillTriangle(x0, y0, x1, y1, x2, y2, ZoneColors[index]);
         tft.fillTriangle(x1, y1, x2, y2, x3, y3, ZoneColors[index]);
      }
      index++;
      int t6 = _thresholds[index].threshold - 50;
      if (i >= t5 && i < t6) {
         tft.fillTriangle(x0, y0, x1, y1, x2, y2, ZoneColors[index]);
         tft.fillTriangle(x1, y1, x2, y2, x3, y3, ZoneColors[index]);
      }
      index++;
      if (i >= t6 && i < ZONE_END) {
         tft.fillTriangle(x0, y0, x1, y1, x2, y2, ZoneColors[index]);
         tft.fillTriangle(x1, y1, x2, y2, x3, y3, ZoneColors[index]);
      }
      // Short tick length for non‑major ticks
      if (i % 25 != 0)
         tl = METER_TICK_SHORT;
      // Recalculate coords
      x0 = sx * (METER_RADIUS + tl) + METER_XCENTER;
      y0 = sy * (METER_RADIUS + tl) + METER_YCENTER;
      x1 = sx * METER_RADIUS + METER_XCENTER;
      y1 = sy * METER_RADIUS + METER_YCENTER;
      // Draw tick
      tft.drawLine(x0, y0, x1, y1, TFT_BLACK);
      // Labels
      if (i % 25 == 0) {
         x0 = sx * (METER_RADIUS + tl + 10) + METER_XCENTER;
         y0 = sy * (METER_RADIUS + tl + 10) + METER_YCENTER;
         switch (i / 25) {
         case -2:
            tft.drawCentreString("0", x0, y0 - 12, 2);
            break;
         case -1:
            tft.drawCentreString("25", x0, y0 - 9, 2);
            break;
         case 0:
            tft.drawCentreString("50", x0, y0 - 6, 2);
            break;
         case 1:
            tft.drawCentreString("75", x0, y0 - 9, 2);
            break;
         case 2:
            tft.drawCentreString("100", x0, y0 - 12, 2);
            break;
         }
      }
      // Arc of scale
      sx = cos((i + 5 - 90) * 0.0174532925);
      sy = sin((i + 5 - 90) * 0.0174532925);
      x0 = sx * METER_RADIUS + METER_XCENTER;
      y0 = sy * METER_RADIUS + METER_YCENTER;
      if (i < 50)
         tft.drawLine(x0, y0, x1, y1, TFT_BLACK);
   }
   tft.drawRect(METER_MARGIN_X, METER_MARGIN_Y, METER_INNER_W, METER_INNER_H, TFT_BLACK);
   plotNeedle(0, 0); // Initial needle
}

#endif