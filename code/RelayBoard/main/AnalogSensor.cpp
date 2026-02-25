#include <Arduino.h>
#include "Log.h"
#include "AnalogSensor.h"

AnalogSensor::AnalogSensor(int channel) {
   _channel = channel;
   pinMode(channel, ANALOG);
}

AnalogSensor::~AnalogSensor() {}

float AnalogSensor::Level() {
   float rVal = 0;
   // --- Oversampled ADC read ---
   float sensorVoltage = ReadOversampledADC() + MA_TO_MV(_calibrationOffset);

   float vMin = MA_TO_MV(_lowerLimit); // (4 mA * 135 Ω) = min mV
   float vMax = MA_TO_MV(_upperLimit); // (20 mA * 135 Ω) = max mV
   if (sensorVoltage <= vMin)
      rVal = 0;
   else if (sensorVoltage >= vMax)
      rVal = 100;
   else
      rVal = (sensorVoltage - vMin) * 100 / (vMax - vMin);
   logv("vMin: %f vMax: %f rVal: %f sensorVoltage: %f", vMin, vMax, rVal, sensorVoltage);
   // --- Filtering chain ---
   float filtered = FilterMedian(rVal);
   // float filtered = FilterEMA(median);
   filtered = roundf(filtered * 10.0f);
   return filtered / 10.0f;
}

float AnalogSensor::ReadOversampledADC() {
   const int samples = 48;
   uint32_t sum = 0;
   for (int i = 0; i < samples; i++)
      sum += analogReadMilliVolts(_channel);
   return (float)sum / samples;
}

float AnalogSensor::FilterEMA(float val) {
   const float alpha = 0.12f; // tune 0.05–0.2 depending on responsiveness
   _emaFiltered = (alpha * val) + ((1.0f - alpha) * _emaFiltered);
   return _emaFiltered;
}

float AnalogSensor::FilterMedian(float val) {
   _medianBuf[_medianIndex] = val;
   _medianIndex = (_medianIndex + 1) % 3;
   float a = _medianBuf[0];
   float b = _medianBuf[1];
   float c = _medianBuf[2];
   // median of 3
   return max(min(a, b), min(max(a, b), c));
}
