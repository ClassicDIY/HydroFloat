#include <Arduino.h>
#include <ArduinoJson.h>
#include "Log.h"
#include "WebLog.h"
#include "IOT.h"
#include "CoilSet.h"
#include "Tank.h"
#include "app.htm"
#include "BLE.h"
#include "app_script.js"

namespace CLASSICDIY {

static AsyncWebServer _asyncServer(ASYNC_WEBSERVER_PORT);
static AsyncWebSocket _webSocket("/ws_home");
IOT _iot = IOT();
#ifdef Has_BT
BLE _ble = BLE();
#endif

void Tank::Setup() {
   Init();
   _iot.Init(this, &_asyncServer);
#ifdef Has_BT
   _ble.begin();
#endif
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
      logd("HTTP_GET /");
      request->send(200, "text/html", home_html, [this](const String &var) { return appTemplateProcessor(var); });
   });
   _asyncServer.on("/appsettings", HTTP_GET, [this](AsyncWebServerRequest *request) {
      JsonDocument app;
      onSaveSetting(app);
      String s;
      serializeJson(app, s);
      request->send(200, "text/html", s);
   });
   _asyncServer.on(
       "/app_fields", HTTP_POST,
       [this](AsyncWebServerRequest *request) {
          // Called after all chunks are received
          logv("Full body received: %s", _bodyBuffer.c_str());
          // Parse JSON safely
          JsonDocument doc; // adjust size to expected payload
          DeserializationError err = deserializeJson(doc, _bodyBuffer);
          if (err) {
             logd("JSON parse failed: %s", err.c_str());
          } else {
             logd("app_fields: %s", formattedJson(doc).c_str());
             _relayThresholds.clear();
             onLoadSetting(doc);
          }
          request->send(200, "application/json", "{\"status\":\"ok\"}");
          _bodyBuffer = ""; // clear for next request
       },
       NULL, // file upload handler (not used here)
       [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
          logv("Chunk received: len=%d, index=%d, total=%d", len, index, total);
          // Append chunk to buffer
          _bodyBuffer.reserve(total); // reserve once for efficiency
          for (size_t i = 0; i < len; i++) {
             _bodyBuffer += (char)data[i];
          }
          if (index + len == total) {
             logd("Upload complete!");
          }
       });
   _webSocket.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
      (void)len;
      if (type == WS_EVT_CONNECT) {
         logd("ws_home socket Connected!");
         _lastWaterLevel = -1; // force a broadcast
         client->setCloseClientOnQueueFull(false);
         client->ping();
      } else if (type == WS_EVT_DISCONNECT) {
         logi("ws_home socket Disconnected!");
      } else if (type == WS_EVT_ERROR) {
         loge("ws error");
      } else if (type == WS_EVT_PONG) {
         logd("ws pong");
      }
   });
}

void Tank::onSaveSetting(JsonDocument &doc) {
   if (_relayThresholds.size() == 0) {
      logd("set default threshhold labels");
#define DEFAULT_LABELS 4
      String defaultLabels[DEFAULT_LABELS] = {"Run", "Start lead", "Start lag", "Overflow"};
      int inc = 100 / (NumberOfRelays() + 1);
      int threshold = inc;
      for (int i = 0; i < NumberOfRelays(); i++) {
         Thresholds newRule;
         newRule.threshold = threshold;
         if (i < DEFAULT_LABELS) {
            newRule.label = defaultLabels[i]; // set default label
         }
         newRule.active = i < DEFAULT_LABELS;
         _relayThresholds.push_back(newRule);
         threshold += inc;
      }
   }
   JsonArray rth = doc["relayThresholds"].to<JsonArray>();
   for (const auto &rule : _relayThresholds) {
      JsonObject obj = rth.add<JsonObject>();
      obj["threshold"] = rule.threshold;
      obj["label"] = rule.label;
      obj["active"] = rule.active;
   }
}

void Tank::onLoadSetting(JsonDocument &doc) {
   JsonArray rth = doc["relayThresholds"].as<JsonArray>();
   for (JsonObject obj : rth) {
      Thresholds rule;
      rule.threshold = obj["threshold"];
      rule.label = obj["label"].as<String>();
      rule.active = obj["active"];
      _relayThresholds.push_back(rule);
   }
}

String Tank::appTemplateProcessor(const String &var) {
   if (var == "title") {
      return String(_iot.getThingName().c_str());
   }
   if (var == "version") {
      return String(APP_VERSION);
   }
   if (var == "Relays") {
      String relays;
      int i = 1;
      for (auto &rule : _relayThresholds) {
         String relay = relay_field;
         std::stringstream relayId;
         relayId << "relay" << i;
         std::stringstream relayname;
         relayname << "Relay " << i++;
         relay.replace("{RelayId}", relayId.str().c_str());
         relay.replace("{RelayN}", relayname.str().c_str());
         relays += relay;
      }
      return relays;
   }
   if (var == "relay_script") {
      String scripts;
      int i = 1;
      for (auto &rule : _relayThresholds) {
         std::stringstream relayId;
         relayId << "relay" << i++;
         String script = relay_script;
         script.replace("{RelayId}", relayId.str().c_str());
         scripts += script;
      }
      return scripts;
   }
   if (var == "app_fields") {
      return String(app_config_fields);
   }
   if (var == "acf") {
      String appFields;
      int i = 1;
      for (auto &rule : _relayThresholds) {
         String appField = app_config_field;
         appField.replace("%RelayN%", String(i++).c_str());
         appField.replace("%th_value%", String(rule.threshold).c_str());
         appField.replace("%state_value%", rule.label.c_str());
         appFields += appField;
      }
      return appFields;
   }
   if (var == "app_script_js") {
      return String(app_script_js);
   }
   if (var == "validateInputs") {
      return String("");
   }
   logd("Did not find app template for: %s", var.c_str());
   return String("");
}

void Tank::Process() {
   _iot.Run();
   Run(); // base class
   String s;
   float waterLevel = _Sensor.Level();
   if (waterLevel >= 0 && abs(_lastWaterLevel - waterLevel) > 1.0) // limit broadcast to 1% change
   {
      JsonDocument doc;
      doc.clear();
      waterLevel = waterLevel <= 1.0 ? 0 : waterLevel;
      _lastWaterLevel = waterLevel;
      doc["level"] = waterLevel;
      String state = "Stop";
      int i = 0;
      for (auto &rule : _relayThresholds) {
         SetRelay(i, waterLevel > rule.threshold ? HIGH : LOW);
         std::stringstream ss;
         ss << "relay" << i++ + 1;
         doc[ss.str()] = waterLevel > rule.threshold ? "on" : "off";
         if (waterLevel > rule.threshold) {
            state = rule.label;
            logv("waterlevel: %f level: %d Label %s", waterLevel, rule.threshold, rule.label.c_str());
         }
      }
      doc["state"] = state.c_str();
#ifdef Has_OLED
      _oled.Display(state.c_str(), waterLevel);
#endif
#ifdef Has_TFT
      _tft.Update(state.c_str(), waterLevel);
#endif
      serializeJson(doc, s);
      _webSocket.textAll(s);
      if (_lastMessagePublished == s) // anything changed?
      {
         return;
      }
#ifdef Has_BT
      _ble.update(waterLevel, state.c_str());
#endif
#ifdef HasMQTT
      _iot.Publish("readings", s.c_str(), false);
#endif
      _lastMessagePublished = s;
      logv("Published readings: %s", s.c_str());
   }
   return;
}

void Tank::onNetworkState(NetworkState state) {
   _networkState = state;
   if (state >= NoNetwork) {
      _tft.AnalogMeter(_relayThresholds); // setup analog display after APMode timeout
   }
   if (state == OnLine) {
#ifdef HasModbus
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
         if ((addr + numCoils) <= NumberOfRelays()) {
            CoilSet coils;
            coils.Init(NumberOfRelays());
            for (int i = 0; i < NumberOfRelays(); i++) {
               coils.set(i, GetRelay(i) == 0 ? false : true);
            }
            vector<uint8_t> coilset = coils.slice(addr, NumberOfRelays());
            response.add(request.getServerID(), request.getFunctionCode(), (uint8_t)coilset.size(), coilset);
         } else {
            logw("READ_COIL Address overflow: %d", (addr + numCoils));
            response.setError(request.getServerID(), request.getFunctionCode(), ILLEGAL_DATA_ADDRESS);
         }
         return response;
      };
      _iot.registerMBTCPWorkers(READ_INPUT_REGISTER, modbusFC04);
      _iot.registerMBTCPWorkers(READ_COIL, modbusFC01);
#endif
   }
}

#if defined(HasModbus) && defined(HasRS485)
bool Tank::onModbusMessage(ModbusMessage &msg) {
   return false;
   // ToDo
}
#endif

#ifdef HasMQTT

void Tank::onMqttConnect(esp_mqtt_client_handle_t &client) {
   if (!_discoveryPublished) {
      for (int i = 0; i < NumberOfRelays(); i++) {
         std::stringstream ss;
         ss << "relay" << i + 1;
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
   device["name"] = _iot.getThingName().c_str();
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