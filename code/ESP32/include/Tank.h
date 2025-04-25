#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include <AsyncTCP.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "Enumerations.h"
#include "html.h"
#include "OTA.h"
#include "Oled.h"
#include "Log.h"
#include "Defines.h"
#include "AnalogSensor.h"


namespace HydroFloat
{
	class Tank
	{
	public:
		Tank() {};
		void setup();
		void Process();
		uint16_t stopLevel = stopLevel_default;
		uint16_t startLeadLevel = startLeadLevel_default;
		uint16_t startLagLevel = startLagLevel_default;
		uint16_t overflowLevel = overflowLevel_default;

	private:
		String _SSID = TAG;
		String _password = DEFAULT_AP_PASSWORD;
		void doBlink();
		NetworkStatus _networkStatus = NotConnected;
		unsigned long _lastBlinkTime = 0;
		bool _blinkStateOn = false;
		void beginWeb();
		void endWeb();
		void saveToEEPROM(const String &jsonString);
		String loadFromEEPROM();
		OTA _OTA = OTA();

		AnalogSensor _Sensor = AnalogSensor(SensorPin);
		float _lastWaterLevel = 0;
		#ifndef EdgeBox
		Oled _oled = Oled();
		#endif
		uint32_t _lastHeap = 0;
	};
}

