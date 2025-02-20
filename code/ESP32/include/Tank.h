#pragma once
#include <Arduino.h>
#include <DNSServer.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "html.h"
#include "OTA.h"
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
		uint16_t stopLevel = 20;
		uint16_t startLeadLevel = 40;
		uint16_t startLagLevel = 60;
		uint16_t overflowLevel = 80;

	private:
		void beginWeb();
		void endWeb();
		void saveToEEPROM(const String &jsonString);
		String loadFromEEPROM();
		OTA _OTA = OTA();
		DNSServer _dnsServer = DNSServer();
		AsyncWebServer _asyncServer = AsyncWebServer(ASYNC_WEBSERVER_PORT);
		WebSocketsServer _webSocket = WebSocketsServer(WSOCKET_HOME_PORT);
		AnalogSensor _Sensor = AnalogSensor(SensorPin);
		float _lastWaterLevel = 0;
	};
}

