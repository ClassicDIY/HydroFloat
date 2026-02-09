#pragma once
#include <Arduino.h>
#include <sstream>
#include <string>

#define SENSOR_R_OHMS          135     // Sense resistor
#define SENSOR_MA_MIN          4.0f    // Nominal 4 mA
#define SENSOR_MA_MAX          20.0f   // Nominal 20 mA

// Convert mA to mV using the sense resistor
#define MA_TO_MV(ma)           ((ma) * SENSOR_R_OHMS)

// Compute calibrated endpoints
#define SENSOR_MIN_MV(offset_mA)  MA_TO_MV(SENSOR_MA_MIN + (offset_mA))
#define SENSOR_MAX_MV(offset_mA)  MA_TO_MV(SENSOR_MA_MAX + (offset_mA))

class AnalogSensor {
 public:
   AnalogSensor(int channel);
   ~AnalogSensor();
   float Level();
   float _calibrationOffset = 0; // default to 4-20mA, Min/Max output voltage from Sensor in mV (135 ohm * 4 mA = 540mV) : (135 ohm * 20mA = 2700mV).

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

