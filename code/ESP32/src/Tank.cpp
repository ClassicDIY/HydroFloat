#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "Log.h"
#include "Tank.h"
#include "html.h"
#include <WebSocketsServer.h>
#include <EEPROM.h>

namespace HydroFloat {

	WebSocketsServer _webSocket = WebSocketsServer(WSOCKET_HOME_PORT);

	void Tank::setup() {
		logd("setup");
		pinMode(RELAY_1, OUTPUT);
		pinMode(RELAY_2, OUTPUT);
		pinMode(RELAY_3, OUTPUT);
		pinMode(RELAY_4, OUTPUT);
		pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
		pinMode(WIFI_STATUS_PIN, OUTPUT);
		_oled.begin();
		_oled.update(0, off);
		EEPROM.begin(EEPROM_SIZE);
		if (digitalRead(FACTORY_RESET_PIN) == LOW)
		{
			logi("Factory Reset");
			EEPROM.write(0, 0);
			EEPROM.commit();
		}
		else {
			logi("Loading configuration from EEPROM");
			String readJsonString = loadFromEEPROM();
			JsonDocument doc;
			DeserializationError error = deserializeJson(doc, readJsonString);
			if (error) {
				loge("Failed to load data from EEPROM, using defaults: %s", error.c_str());
				JsonDocument doc;
				doc["version"] = CONFIG_VERSION;
				doc["ssid"] = _SSID;
				doc["appw"] = _password;
				doc["of"] = overflowLevel;
				doc["slag"] = startLagLevel;
				doc["slead"] = startLeadLevel;
				doc["stop"] = stopLevel;
				String jsonString;
				serializeJson(doc, jsonString);
				saveToEEPROM(jsonString);
			} else {
				_SSID = doc["ssid"].isNull() ? TAG : doc["ssid"].as<String>();
				_password = doc["appw"].isNull() ? DEFAULT_AP_PASSWORD : doc["appw"].as<String>();
				overflowLevel = doc["of"].isNull() ? overflowLevel_default : doc["of"];
				startLagLevel = doc["slag"].isNull() ? startLagLevel_default : doc["slag"];
				startLeadLevel = doc["slead"].isNull() ? startLeadLevel_default : doc["slead"];
				stopLevel = doc["stop"].isNull() ? stopLevel_default : doc["stop"];
			}
		}
		WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
			logd("[WiFi-event] event: %d", event);
			switch (event) {
			case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
				_networkStatus = NotConnected;
			break;
			case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
				_networkStatus = APMode;
			break;
			}
		  });
		logi("WiFi AP SSID: %s PW: %s", _SSID, _password);
		if (WiFi.softAP(_SSID, _password)) {
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
			if (type == WStype_DISCONNECTED) {
				logi("[%u] Home Page Disconnected!", num);
				_networkStatus = APMode;
			}
			else if (type == WStype_CONNECTED) {
				logi("[%u] Home Page Connected!", num);
				_lastWaterLevel = -1; //force a broadcast
				_networkStatus = WSMode;
			} 
		});

		_asyncServer.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
			logd("home");
			String page = home_html;
			page.replace("{n}", _SSID);
			page.replace("{v}", CONFIG_VERSION);
			page.replace("{hp}", String(WSOCKET_HOME_PORT));
			request->send(200, "text/html", page);
		});

		_asyncServer.onNotFound([this](AsyncWebServerRequest *request) {
			if (APMode == _networkStatus) {
				logd("Redirecting from: %s", request->url().c_str());
				String page = redirect_html;
				page.replace("{n}", _SSID);
				IPAddress IP = WiFi.softAPIP();
				page.replace("{ip}", IP.toString().c_str());
				request->send(200, "text/html", page);
			}
		});

		_asyncServer.on("/settings", HTTP_GET, [this](AsyncWebServerRequest *request) {
			logd("settings");
			String page = settings_html;
			page.replace("{n}", _SSID);
			page.replace("{v}", CONFIG_VERSION);
			page.replace("{ssid}", _SSID);
			page.replace("{appw}", _password);
			page.replace("{of}", String(overflowLevel));
			page.replace("{slag}", String(startLagLevel));
			page.replace("{slead}", String(startLeadLevel));
			page.replace("{stop}", String(stopLevel));
			request->send(200, "text/html", page);
		});

		_asyncServer.on("/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
			logd("config");
			String page = config_html;
			page.replace("{n}", _SSID);
			page.replace("{v}", CONFIG_VERSION);
			page.replace("{ssid}", _SSID);
			page.replace("{appw}", _password);
			page.replace("{of}", String(overflowLevel));
			page.replace("{slag}", String(startLagLevel));
			page.replace("{slead}", String(startLeadLevel));
			page.replace("{stop}", String(stopLevel));
			request->send(200, "text/html", page);
		});
		_asyncServer.on("/submit", HTTP_POST, [this](AsyncWebServerRequest *request) {
			logd("submit");

			if (request->hasParam("ssid", true)) {
				_SSID = request->getParam("ssid", true)->value().c_str();
			}
			if (request->hasParam("appw", true)) {
				_password = request->getParam("appw", true)->value().c_str();
			}
			if (request->hasParam("overflow", true)) {
				overflowLevel = request->getParam("overflow", true)->value().toInt();
			}
			if (request->hasParam("slag", true)) {
				startLagLevel = request->getParam("slag", true)->value().toInt();
			}
			if (request->hasParam("slead", true)) {
				startLeadLevel = request->getParam("slead", true)->value().toInt();
			}	
			if (request->hasParam("stop", true)) {
				stopLevel = request->getParam("stop", true)->value().toInt();
			}	
			JsonDocument doc;
			doc["version"] = CONFIG_VERSION;
			doc["ssid"] = _SSID;
			doc["appw"] = _password;
			doc["of"] = overflowLevel;
			doc["slag"] = startLagLevel;
			doc["slead"] = startLeadLevel;
			doc["stop"] = stopLevel;
			String jsonString;
			serializeJson(doc, jsonString);
			saveToEEPROM(jsonString);
			String page = settings_html;
			page.replace("{n}", _SSID);
			page.replace("{v}", CONFIG_VERSION);
			page.replace("{ssid}", _SSID);
			page.replace("{appw}", _password);
			page.replace("{of}", String(overflowLevel));
			page.replace("{slag}", String(startLagLevel));
			page.replace("{slead}", String(startLeadLevel));
			page.replace("{stop}", String(stopLevel));
			request->send(200, "text/html", page);
		});
		_OTA.begin(&_asyncServer);
	}

	void Tank::endWeb()	{
		_asyncServer.end();
		_webSocket.close();
	}

	void Tank::Process() {
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

			digitalWrite(RELAY_1, s1);
			digitalWrite(RELAY_2, s2);
			digitalWrite(RELAY_3, s3);
			digitalWrite(RELAY_4, s4);

			JsonDocument doc;
			doc.clear();
			doc["level"] = waterLevel;
			doc["relay1"] = digitalRead(RELAY_1) ? "on" : "off";
			doc["relay2"] = digitalRead(RELAY_2) ? "on" : "off";
			doc["relay3"] = digitalRead(RELAY_3) ? "on" : "off";
			doc["relay4"] = digitalRead(RELAY_4) ? "on" : "off";
			serializeJson(doc, s);
			_webSocket.broadcastTXT(s);
			_oled.update(waterLevel, s4 ? overflow : s3 ? slag : s2 ? slead : s1 ? stop : off);
			logd("broadcast JSON: %s", s.c_str());
		}
		_webSocket.loop();
		_dnsServer.processNextRequest();
		doBlink();
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

	void Tank::doBlink()
	{
		unsigned long blinkRate = 0;
		switch (_networkStatus)
		{
		case WSMode:
			blinkRate = 0;
			break;
		case APMode:
			blinkRate = AP_BLINK_RATE;
			break;
		case NotConnected:
			blinkRate = NC_BLINK_RATE;
			break;
		}
		if (blinkRate != 0)
		{
			unsigned long now = millis();
			if (blinkRate < now - _lastBlinkTime)
			{
				_blinkStateOn = !_blinkStateOn;
				_lastBlinkTime = now;
				digitalWrite(WIFI_STATUS_PIN, _blinkStateOn ? HIGH : LOW);
			}
		}
		else
		{
			digitalWrite(WIFI_STATUS_PIN, HIGH);
		}
	}
}