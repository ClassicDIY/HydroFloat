#include <Arduino.h>
#include "Log.h"
#include "Defines.h"
#include "AnalogSensor.h"

#ifndef EdgeBox
namespace HydroFloat{
	AnalogSensor::AnalogSensor(int channel) {
		_channel = channel;
		_count = 0;
		_numberOfSummations = 0;
		_rollingSum = 0;
		pinMode(channel, ANALOG);
	}

	AnalogSensor::~AnalogSensor()	{
	}

	float AnalogSensor::Level()	{
		double adcReading_mv = analogReadMilliVolts(_channel);
		float percent = (adcReading_mv - SensorVoltageMin) * 100 / (SensorVoltageMax - SensorVoltageMin);
		float averagePercent = AddReading(percent);
		averagePercent = roundf(averagePercent); // round to 0 decimal place
		#ifdef LOG_SENSOR_VOLTAGE
		if (_count++ > 100)	{
			logd("Sensor Reading: %d percent: %f, averagePercent:%f", adcReading, percent , averagePercent);
			_count = 0;
		}
		#endif
		return averagePercent;
	}

	float AnalogSensor::AddReading(float val)	{
		float currentAvg = 0.0;
		if (_numberOfSummations > 0)		{
			currentAvg = _rollingSum / _numberOfSummations;
		}
		if (_numberOfSummations < SAMPLESIZE)		{
			_numberOfSummations++;
		}
		else		{
			_rollingSum -= currentAvg;
		}
		_rollingSum += val;
		return _rollingSum / _numberOfSummations;
	}
} // namespace namespace ESP_PLC
#else
#include <Adafruit_ADS1X15.h>
extern Adafruit_ADS1115 ads;

namespace HydroFloat
{
	AnalogSensor::AnalogSensor(int channel)
	{
		_channel = channel;
	}

	float AnalogSensor::Level()
	{
		uint16_t val = ads.readADC_SingleEnded(_channel);
		val = val > 13175 ? 13175 : val < 2635 ? 2635 : val; // limit to 1V - 5V for 4-20mA sensor
		return roundf((((val - 2635) * 100.0) / 10540) * 10.0) / 10.0;
	}
} // namespace namespace EDGEBOX

#endif
