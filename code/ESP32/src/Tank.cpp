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
		pinMode(PUMP_1, OUTPUT);
		pinMode(PUMP_2, OUTPUT);
		pinMode(PUMP_3, OUTPUT);
		pinMode(PUMP_4, OUTPUT);
		pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
		pinMode(WIFI_STATUS_PIN, OUTPUT);
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
				logi("[%u] Home Page Disconnected!\n", num);
				_networkStatus = APMode;
			}
			else if (type == WStype_CONNECTED) {
				logi("[%u] Home Page Connected!\n", num);
				_lastWaterLevel = -1; //force a broadcast
				_networkStatus = WSMode;
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
			String page = redirect_html;
			page.replace("{n}", TAG);
			IPAddress IP = WiFi.softAPIP();
			page.replace("{ip}", IP.toString().c_str());
			request->send(200, "text/html", page);
		});

		_asyncServer.on("/settings", HTTP_GET, [this](AsyncWebServerRequest *request) {
			logd("settings");
			String page = settings_html;
			page.replace("{n}", TAG);
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
			page.replace("{n}", TAG);
			page.replace("{v}", CONFIG_VERSION);
			page.replace("{ssid}", _SSID);
			page.replace("{appw}", _password);
			page.replace("{of}", String(overflowLevel));
			page.replace("{slag}", String(startLagLevel));
			page.replace("{slead}", String(startLeadLevel));
			page.replace("{stop}", String(stopLevel));
			page.replace("{ssid_de}", "");
			page.replace("{appw_de}", "");
			page.replace("{stop_de}", "");
			page.replace("{slead_de}", "");
			page.replace("{slag_de}", "");
			page.replace("{ssid_em}", "hide");
			page.replace("{appw_em}", "hide");
			page.replace("{stop_em}", "hide");
			page.replace("{slead_em}", "hide");
			page.replace("{slag_em}", "hide");
			request->send(200, "text/html", page);
		});
		_asyncServer.on("/submit", HTTP_POST, [this](AsyncWebServerRequest *request) {
			logd("submit");
			String SSIDParam;
			String passwordParam;
			uint16_t overflowParam = 0;
			uint16_t startLagParam = 0;
			uint16_t startLeadParam = 0;
			uint16_t stopParam = 0;

			if (request->hasParam("ssid", true)) {
				SSIDParam = request->getParam("ssid", true)->value().c_str();
			}
			if (request->hasParam("appw", true)) {
				passwordParam = request->getParam("appw", true)->value().c_str();
			}
			if (request->hasParam("overflow", true)) {
				overflowParam = request->getParam("overflow", true)->value().toInt();
			}
			if (request->hasParam("slag", true)) {
				startLagParam = request->getParam("slag", true)->value().toInt();
			}
			if (request->hasParam("slead", true)) {
				startLeadParam = request->getParam("slead", true)->value().toInt();
			}	
			if (request->hasParam("stop", true)) {
				stopParam = request->getParam("stop", true)->value().toInt();
			}	
			String page = config_html;
			bool valid = true;
			if (SSIDParam.length() == 0) {
				page.replace("{ssid_de}", "de");
				page.replace("{ssid_em}", "em");
				valid = false;
			}
			else {
				page.replace("{ssid_de}", "");
				page.replace("{ssid_em}", "hide");
			}
			if (passwordParam.length() < 8) {
				page.replace("{appw_de}", "de");
				page.replace("{appw_em}", "em");
				valid = false;
			}
			else {
				page.replace("{appw_de}", "");
				page.replace("{appw_em}", "hide");
			}
			if (startLeadParam <= stopParam) {
				page.replace("{stop_de}", "de");
				page.replace("{stop_em}", "em");
				valid = false;
			}
			else {
				page.replace("{stop_de}", "");
				page.replace("{stop_em}", "hide");
			}
			if (startLagParam <= startLeadParam ) {
				page.replace("{slead_de}", "de");
				page.replace("{slead_em}", "em");
				valid = false;
			}
			else {
				page.replace("{slead_de}", "");
				page.replace("{slead_em}", "hide");
			}
			if (overflowParam <= startLagParam ) {
				page.replace("{slag_de}", "de");
				page.replace("{slag_em}", "em");
				valid = false;
			}
			else {
				page.replace("{slag_de}", "");
				page.replace("{slag_em}", "hide");
			}
			if (valid) {
				_SSID = SSIDParam;
				_password = passwordParam;
				overflowLevel = overflowParam;
				startLagLevel = startLagParam;
				startLeadLevel = startLeadParam;
				stopLevel = stopParam;
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
				page = settings_html;
			}
			page.replace("{n}", TAG);
			page.replace("{v}", CONFIG_VERSION);
			page.replace("{ssid}", SSIDParam);
			page.replace("{appw}", passwordParam);
			page.replace("{of}", String(overflowParam));
			page.replace("{slag}", String(startLagParam));
			page.replace("{slead}", String(startLeadParam));
			page.replace("{stop}", String(stopParam));
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