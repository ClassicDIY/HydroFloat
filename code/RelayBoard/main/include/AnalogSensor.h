#pragma once
#include <Arduino.h>
#include <sstream> 
#include <string>
#include "defines.h"

namespace CLASSICDIY
{
	class AnalogSensor
	{
	public:
		
		AnalogSensor(int channel);
		~AnalogSensor();
		float Level();

	private:
		int _channel; // Defines the pin that the sensor is connected to
		float AddReading(float val);
		float _rollingSum;
		int _numberOfSummations;
		int _count;
		uint32_t adcReadingMin = 2635;
		uint32_t adcReadingMax = 13175;
	};
}
