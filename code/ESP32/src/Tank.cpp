#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "Log.h"
#include "Tank.h"
#include "html.h"
#include <WebSocketsServer.h>
#include <EEPROM.h>

namespace HydroFloat
{

	WebSocketsServer _webSocket = WebSocketsServer(WSOCKET_HOME_PORT);

	void Tank::setup()
	{
		logd("setup");
		pinMode(PUMP_1, OUTPUT);
		pinMode(PUMP_2, OUTPUT);
		pinMode(PUMP_3, OUTPUT);
		pinMode(PUMP_4, OUTPUT);
		WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
			logd("[WiFi-event] event: %d", event);
		  });

		if (WiFi.softAP(TAG, DEFAULT_AP_PASSWORD))
		{
			logd("AP Started");
			IPAddress IP = WiFi.softAPIP();
			logd("AP IP address: %s", IP.toString().c_str());
			_dnsServer.start(DNS_PORT, "*", IP);
			beginWeb();
		}	
	}

	void Tank::beginWeb()
	{
		_asyncServer.begin();
		_webSocket.begin();
		_webSocket.onEvent([this](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
		{ 
			if (type == WStype_DISCONNECTED)
			{
				logi("[%u] Home Page Disconnected!\n", num);
			}
			else if (type == WStype_CONNECTED)
			{
				logi("[%u] Home Page Connected!\n", num);
				_lastWaterLevel = -1; //force a broadcast
			} 
		});

		_asyncServer.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
			logd("home");
			String page = home_html;
			page.replace("{n}", TAG);
			page.replace("{v}", CONFIG_VERSION);
			page.replace("{hp}", String(WSOCKET_HOME_PORT));
			request->send(200, "text/html", page);
		});

		_asyncServer.onNotFound([this](AsyncWebServerRequest *request) {
			logd("Not found: %s", request->url().c_str());
			// String page = home_html;
			// page.replace("{n}", TAG);
			// page.replace("{v}", CONFIG_VERSION);
			// page.replace("{hp}", String(WSOCKET_HOME_PORT));
			// request->send(200, "text/html", page);
		});
		_asyncServer.on("/settings", HTTP_GET, [this](AsyncWebServerRequest *request) {
			logd("settings");
			String page = settings_html;
			page.replace("{n}", TAG);
			page.replace("{v}", CONFIG_VERSION);
			page.replace("{of}", String(overflowLevel));
			page.replace("{slag}", String(startLagLevel));
			page.replace("{slead}", String(startLeadLevel));
			page.replace("{stop}", String(stopLevel));
			request->send(200, "text/html", page);
		});

		_asyncServer.on("/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
			logd("config");
			String page = config_html;
			page.replace("{n}", TAG);
			page.replace("{v}", CONFIG_VERSION);
			page.replace("{of}", String(overflowLevel));
			page.replace("{slag}", String(startLagLevel));
			page.replace("{slead}", String(startLeadLevel));
			page.replace("{stop}", String(stopLevel));
			request->send(200, "text/html", page);
		});
		_asyncServer.on("/submit", HTTP_POST, [this](AsyncWebServerRequest *request) {
			logd("submit");
			if (request->hasParam("overflow", true))
			{
				overflowLevel = request->getParam("overflow", true)->value().toInt();
				logd("Overflow Level: %d", overflowLevel);
			}
			if (request->hasParam("slag", true))
			{
				startLagLevel = request->getParam("slag", true)->value().toInt();
				logd("Start Lag Level: %d", startLagLevel);
			}
			if (request->hasParam("slead", true))
			{
				startLeadLevel = request->getParam("slead", true)->value().toInt();
				logd("Start Lead Level: %d", startLeadLevel);
			}	
			if (request->hasParam("stop", true))
			{
				stopLevel = request->getParam("stop", true)->value().toInt();
				logd("Stop Level: %d", stopLevel);
			}	
			String page = settings_html;
			page.replace("{n}", TAG);
			page.replace("{v}", CONFIG_VERSION);
			page.replace("{of}", String(overflowLevel));
			page.replace("{slag}", String(startLagLevel));
			page.replace("{slead}", String(startLeadLevel));
			page.replace("{stop}", String(stopLevel));
			request->send(200, "text/html", page);

			JsonDocument doc;
			doc["of"] = overflowLevel;
			doc["slag"] = startLagLevel;
			doc["slead"] = startLeadLevel;
			doc["stop"] = stopLevel;
			String jsonString;
			serializeJson(doc, jsonString);
			saveToEEPROM(jsonString);
		});
		_OTA.begin(&_asyncServer);
		EEPROM.begin(EEPROM_SIZE);
		String readJsonString = loadFromEEPROM();
		JsonDocument doc;
		DeserializationError error = deserializeJson(doc, readJsonString);
		if (error) {
		  loge("Failed to load data from EEPROM, using defaults: %s", error.c_str());
		} else {
			overflowLevel = doc["of"];
			startLagLevel = doc["slag"];
			startLeadLevel = doc["slead"];
			stopLevel = doc["stop"];
		}
	}

	void Tank::endWeb()
	{
		_asyncServer.end();
		_webSocket.close();
	}

	void Tank::Process()
	{
		String s;
		float waterLevel = _Sensor.Level();
		if (waterLevel >= 0 && abs(_lastWaterLevel - waterLevel) > 1.0) // limit broadcast to 1% change
		{
			waterLevel = waterLevel <= 1.0 ? 0 : waterLevel;
			_lastWaterLevel = waterLevel;
			boolean s1 = waterLevel > stopLevel;
			boolean s2 = waterLevel > startLeadLevel;
			boolean s3 = waterLevel > startLagLevel;
			boolean s4 = waterLevel > overflowLevel;

			digitalWrite(PUMP_1, s1);
			digitalWrite(PUMP_2, s2);
			digitalWrite(PUMP_3, s3);
			digitalWrite(PUMP_4, s4);

			JsonDocument doc;
			doc.clear();
			doc["level"] = waterLevel;
			doc["pump1"] = digitalRead(PUMP_1) ? "on" : "off";
			doc["pump2"] = digitalRead(PUMP_2) ? "on" : "off";
			doc["pump3"] = digitalRead(PUMP_3) ? "on" : "off";
			doc["pump4"] = digitalRead(PUMP_4) ? "on" : "off";
			serializeJson(doc, s);
			_webSocket.broadcastTXT(s);
			logd("Water Level: %f JSON: %s", waterLevel, s.c_str());
		}
		_webSocket.loop();
		_dnsServer.processNextRequest();
		return;
	}

	void Tank::saveToEEPROM(const String &jsonString) {
		for (int i = 0; i < jsonString.length(); ++i) {
		  EEPROM.write(i, jsonString[i]);
		}
		EEPROM.write(jsonString.length(), '\0'); // Null-terminate the string
		EEPROM.commit();
		logd("JSON saved to EEPROM");
	  }
	  
	  String Tank::loadFromEEPROM() {
		String jsonString;
		char ch;
		for (int i = 0; i < EEPROM_SIZE; ++i) {
		  ch = EEPROM.read(i);
		  if (ch == '\0') break; // Stop at the null terminator
		  jsonString += ch;
		}
		return jsonString;
	  }
}