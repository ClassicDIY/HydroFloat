#pragma once
#include <Arduino.h>
#include <sstream>
#include <string>

#define SENSOR_R_OHMS          135     // Sense resistor
#define SENSOR_MA_MIN          4.0f    // Nominal 4 mA
#define SENSOR_MA_MAX          20.0f   // Nominal 20 mA

// Convert mA to mV using the sense resistor
#define MA_TO_MV(ma)           ((ma) * SENSOR_R_OHMS)

class AnalogSensor {
 public:
   AnalogSensor(int channel);
   ~AnalogSensor();
   float Level();
   float _calibrationOffset = 0; // default to 4-20mA, Min/Max output voltage from Sensor in mV (135 ohm * 4 mA = 540mV) : (135 ohm * 20mA = 2700mV).
   float _lowerLimit = 4.0; // range limits to calculate percentage
   float _upperLimit = 20.0;

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

