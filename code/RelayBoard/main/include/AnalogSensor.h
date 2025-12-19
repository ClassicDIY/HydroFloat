#pragma once
#include <Arduino.h>
#include <sstream>
#include <string>
#include "defines.h"

namespace CLASSICDIY {
class AnalogSensor {
 public:
   AnalogSensor(int channel);
   ~AnalogSensor();
   float Level();

 private:
   int _channel; // Defines the pin that the sensor is connected to
 private:
   float ReadOversampledADC();
   float FilterEMA(float val);
   float FilterMedian(float val);
   // --- Filtering state ---
   float _emaFiltered = 0.0;
   float _medianBuf[3] = {0, 0, 0};
   int _medianIndex = 0;
};
} // namespace CLASSICDIY
