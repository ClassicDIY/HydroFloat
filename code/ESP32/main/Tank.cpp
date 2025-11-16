#include <Arduino.h>
#include <ArduinoJson.h>
#include "Log.h"
#include "WebLog.h"
#include "IOT.h"
#include "CoilSet.h"
#include "Tank.h"
#include "style.html"
#include "app.html"

namespace CLASSICDIY {

static AsyncWebServer _asyncServer(ASYNC_WEBSERVER_PORT);
static AsyncWebSocket _webSocket("/ws_home");
IOT _iot = IOT();

void Tank::setup() {
   _iot.Init(this, &_asyncServer);
   _asyncServer.addHandler(&_webSocket).addMiddleware([this](AsyncWebServerRequest *request, ArMiddlewareNext next) {
      // ws.count() is the current count of WS clients: this one is trying to upgrade its HTTP connection
      if (_webSocket.count() > 1) {
         // if we have 2 clients or more, prevent the next one to connect
         request->send(503, "text/plain", "Server is busy");
      } else {
         // process next middleware and at the end the handler
         next();
      }
   });
   _asyncServer.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
      String page = home_html;
      page.replace("{style}", style);
      page.replace("{n}", _iot.getThingName().c_str());
      page.replace("{v}", APP_VERSION);
      request->send(200, "text/html", page);
   });
   _webSocket.onEvent(
       [this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
          (void)len;
          if (type == WS_EVT_CONNECT) {
             // logi("Home Page Connected!");
             _lastWaterLevel = -1; // force a broadcast
             client->setCloseClientOnQueueFull(false);
             // client->ping();
          } else if (type == WS_EVT_DISCONNECT) {
             // logi("Home Page Disconnected!");
          } else if (type == WS_EVT_ERROR) {
             loge("ws error");

             // } else if (type == WS_EVT_PONG) {
             // 	logd("ws pong");
          }
       });
}

void Tank::onSaveSetting(JsonDocument &doc) {
   doc["of"] = overflowLevel;
   doc["slag"] = startLagLevel;
   doc["slead"] = startLeadLevel;
   doc["stop"] = stopLevel;
}

void Tank::onLoadSetting(JsonDocument &doc) {
   overflowLevel = doc["of"].isNull() ? overflowLevel_default : doc["of"];
   startLagLevel = doc["slag"].isNull() ? startLagLevel_default : doc["slag"];
   startLeadLevel = doc["slead"].isNull() ? startLeadLevel_default : doc["slead"];
   stopLevel = doc["stop"].isNull() ? stopLevel_default : doc["stop"];
}

void Tank::addApplicationConfigs(String &page) {
   String appFields = app_config_fields;
   appFields.replace("{of}", String(overflowLevel));
   appFields.replace("{slag}", String(startLagLevel));
   appFields.replace("{slead}", String(startLeadLevel));
   appFields.replace("{stop}", String(stopLevel));
   page += appFields;
}

void Tank::onSubmitForm(AsyncWebServerRequest *request) {
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
   };
}

void Tank::Process() {
   _iot.Run();
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
#ifdef Has_OLED_Display
      _oled.update(waterLevel, s4 ? overflow : s3 ? slag : s2 ? slead : s1 ? stop : off);
#endif
      serializeJson(doc, s);
      _webSocket.textAll(s);
      if (_lastMessagePublished == s) // anything changed?
      {
         return;
      }
#ifdef HasMQTT
      _iot.Publish("readings", s.c_str(), false);
#endif
      _lastMessagePublished = s;

      logv("Published readings: %s", s.c_str());
   }
   return;
}
#ifdef HasModbus
void Tank::onNetworkConnect() {
   // READ_INPUT_REGISTER
   auto modbusFC04 = [this](ModbusMessage request) -> ModbusMessage {
      ModbusMessage response;
      uint16_t addr = 0;
      uint16_t words = 0;
      request.get(2, addr);
      request.get(4, words);
      logd("READ_INPUT_REGISTER %d %d[%d]", request.getFunctionCode(), addr, words);
      addr -= _iot.getMBBaseAddress(AnalogInputs);
      if ((addr + words) == 1) { // just have the level
         response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)(words * 2));
         response.add(_lastWaterLevel);
      } else {
         logw("READ_INPUT_REGISTER Address overflow: %d", (addr + words));
         response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
      }
      return response;
   };
   // READ_COIL
   auto modbusFC01 = [this](ModbusMessage request) -> ModbusMessage {
      ModbusMessage response;
      uint16_t addr = 0;
      uint16_t numCoils = 0;
      request.get(2, addr, numCoils);
      logd("READ_COIL %d %d[%d]", request.getFunctionCode(), addr, numCoils);
      // Address overflow?
      addr -= _iot.getMBBaseAddress(DigitalOutputs);
      if ((addr + numCoils) <= NUM_RELAYS) {
         CoilSet coils;
         coils.Init(NUM_RELAYS);
         coils.set(0, digitalRead(RELAY_1) == 0 ? false : true);
         coils.set(1, digitalRead(RELAY_2) == 0 ? false : true);
         coils.set(2, digitalRead(RELAY_3) == 0 ? false : true);
         coils.set(3, digitalRead(RELAY_4) == 0 ? false : true);
         vector<uint8_t> coilset = coils.slice(addr, NUM_RELAYS);
         response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)coilset.size(), coilset);
      } else {
         logw("READ_COIL Address overflow: %d", (addr + numCoils));
         response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
      }
      return response;
   };
   _iot.registerMBTCPWorkers(READ_INPUT_REGISTER, modbusFC04);
   _iot.registerMBTCPWorkers(READ_COIL, modbusFC01);
}
bool Tank::onModbusMessage(ModbusMessage &msg) { return false; }
#endif

#ifdef HasMQTT

void Tank::onMqttConnect() {
   if (!_discoveryPublished) {
      for (int i = 0; i < NUM_RELAYS; i++) {
         std::stringstream ss;
         ss << "relay" << i+1;
         if (PublishDiscoverySub(DigitalInputs, ss.str().c_str(), nullptr, "mdi:valve") == false) {
            return; // try later
         }
      }
      if (PublishDiscoverySub(AnalogInputs, "level", "%", "mdi:lightning-bolt") == false) {
         return; // try later
      }
      _discoveryPublished = true;
   }
}

boolean Tank::PublishDiscoverySub(IOTypes type, const char *entityName, const char *unit_of_meas, const char *icon) {
   String topic = HOME_ASSISTANT_PREFIX;
   switch (type) {
   case DigitalOutputs:
      topic += "/switch/";
      break;
   case AnalogOutputs:
      topic += "/number/";
      break;
   case DigitalInputs:
      topic += "/sensor/";
      break;
   case AnalogInputs:
      topic += "/sensor/";
      break;
   }
   topic += String(_iot.getUniqueId());
   topic += "/";
   topic += entityName;
   topic += "/config";

   JsonDocument payload;
   payload["platform"] = "mqtt";
   payload["name"] = entityName;
   payload["unique_id"] = String(_iot.getUniqueId()) + "_" + String(entityName);
   payload["value_template"] = ("{{ value_json." + String(entityName) + " }}").c_str();
   payload["state_topic"] = _iot.getRootTopicPrefix().c_str() + String("/stat/readings");
   if (type == DigitalOutputs) {
      payload["command_topic"] = _iot.getRootTopicPrefix().c_str() + String("/set/") + String(entityName);
      payload["state_on"] = "On";
      payload["state_off"] = "Off";
   } else if (type == AnalogOutputs) {
      payload["command_topic"] = _iot.getRootTopicPrefix().c_str() + String("/set/") + String(entityName);
      payload["min"] = 0;
      payload["max"] = 65535;
      payload["step"] = 1;
   } else if (type == DigitalInputs) {
      payload["payload_off"] = "off";
      payload["payload_on"] = "on";
   }
   payload["availability_topic"] = _iot.getRootTopicPrefix().c_str() + String("/tele/LWT");
   payload["payload_available"] = "Online";
   payload["payload_not_available"] = "Offline";
   if (unit_of_meas) {
      payload["unit_of_measurement"] = unit_of_meas;
   }
   if (icon) {
      payload["icon"] = icon;
   }

   char buffer[STR_LEN];
   JsonObject device = payload["device"].to<JsonObject>();
   device["name"] = _iot.getThingName();
   device["sw_version"] = APP_VERSION;
   device["manufacturer"] = "ClassicDIY";
   sprintf(buffer, "%s (%X)", TAG, _iot.getUniqueId());
   device["model"] = buffer;
   JsonArray identifiers = device["identifiers"].to<JsonArray>();
   sprintf(buffer, "%X", _iot.getUniqueId());
   identifiers.add(buffer);

   logd("Discovery => topic: %s", topic.c_str());
   return _iot.PublishMessage(topic.c_str(), payload, true);
}

void Tank::onMqttMessage(char *topic, char *payload) {}
#endif

} // namespace CLASSICDIY