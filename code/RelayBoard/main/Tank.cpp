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

IOT _iot = IOT();
#ifdef Has_BT
BLE _ble = BLE();
#endif

void Tank::Setup() {
   Init();
   _iot.Init(this);
#ifdef Has_BT
   _ble.begin();
#endif
}

void Tank::onSaveSetting(JsonDocument &doc) {
   if (_thresholds.size() == 0) {
      logd("set default threshhold labels");
      _mode = Float;
      _base_state = "Stop";
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
         _thresholds.push_back(newRule);
         threshold += inc;
      }
   }
   doc["mode"] = _mode;
   doc["baseState"] = _base_state;
   JsonArray rth = doc["relayThresholds"].to<JsonArray>();
   for (const auto &rule : _thresholds) {
      JsonObject obj = rth.add<JsonObject>();
      obj["threshold"] = rule.threshold;
      obj["label"] = rule.label;
      obj["active"] = rule.active;
   }
   doc["_calibrationOffset"] = _Sensor._calibrationOffset;
   doc["_lowerLimit"] = _Sensor._lowerLimit;
   doc["_upperLimit"] = _Sensor._upperLimit;
}

void Tank::onLoadSetting(JsonDocument &doc) {
   logd("app_fields: %s", formattedJson(doc).c_str());
   _mode = doc["mode"].isNull() ? Float : doc["mode"].as<Mode>();
   _thresholds.clear();
   _base_state = doc["baseState"].as<String>();
   JsonArray rth = doc["relayThresholds"].as<JsonArray>();
   for (JsonObject obj : rth) {
      Thresholds rule;
      rule.threshold = obj["threshold"];
      rule.label = obj["label"].as<String>();
      rule.active = obj["active"];
      _thresholds.push_back(rule);
   }
   _Sensor._calibrationOffset = doc["_calibrationOffset"];
   _Sensor._lowerLimit = doc["_lowerLimit"];
   _Sensor._upperLimit = doc["_upperLimit"];
}

String Tank::appTemplateProcessor(const String &var) {
   if (var == "title") {
      return String(_iot.getThingName().c_str());
   }
   if (var == "version") {
      return String(APP_VERSION);
   }
   if (var == "home_html") {
      String home;
      home.reserve(strlen(home_html));
      home += home_html;
      return home;
   }
   if (var == "Relays") {
      String relays;
      int i = 1;
      for (auto &rule : _thresholds) {
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
      for (auto &rule : _thresholds) {
         std::stringstream relayId;
         relayId << "relay" << i++;
         String script = relay_script;
         script.replace("{RelayId}", relayId.str().c_str());
         scripts += script;
      }
      return scripts;
   }
   if (var == "app_fields") {
      return String(app_configs);
   }
   if (var == "acf") {
      String appFields;
      String base_state = base_state_config;
      base_state.replace("%state_value%", _base_state.c_str());
      appFields += base_state;
      int i = 1;
      for (auto &rule : _thresholds) {
         String appField = threshold_config;
         appField.replace("%RelayN%", String(i++).c_str());
         appField.replace("%th_value%", String(rule.threshold).c_str());
         appField.replace("%state_value%", rule.label.c_str());
         appFields += appField;
      }
      return appFields;
   }
   if (var == "appSelectValues") {
      return String(app_script_select);
   }
   if (var == "appScript") {
      return String(app_script_base);
   }
   if (var == "app_script_js") {
      return String(app_script_js);
   }
   if (var == "onload") {
      return String(onLoadScript);
   }
   if (var == "validateInputs") {
      return String(app_validateInputs);
   }
   logd("Did not find app template for: %s", var.c_str());
   return String("");
}

void Tank::Process() {
   _iot.Run();
   Run(); // base class
   String s;
   float waterLevel = _Sensor.Level();
   if (waterLevel >= 0 && abs(_lastWaterLevel - waterLevel) > 0.5) // limit broadcast to .5% change
   {
      JsonDocument doc;
      doc.clear();
      waterLevel = waterLevel <= 1.0 ? 0 : waterLevel;
      _lastWaterLevel = waterLevel;
      doc["level"] = waterLevel;
      String state = _base_state;
      int i = 0;
      for (auto &rule : _thresholds) {
         if (_mode == Pump) {
            if (i == 0) {
               if (waterLevel <= rule.threshold) { // below stop threshold? turn all relays off
                  for (int j = 0; j < NumberOfRelays(); j++) {
                     SetRelay(j, LOW);
                  }
               } else {
                  SetRelay(i, HIGH); // just set relay 0 on
               }
            }
            if (i == 1) {
               if (waterLevel > rule.threshold) {         // start lead
                  if (!GetRelay(i) && !GetRelay(i + 1)) { // both relays off?
                     SetRelay(_alternate ? i + 1 : i, HIGH);
                     _alternate = !_alternate; // toggle
                  }
               }
            }
            if (i == 2) {
               if (waterLevel > rule.threshold) { // start lag => all pumps on
                  SetRelay(1, HIGH);
                  SetRelay(2, HIGH);
               }
            }
            if (i == 3) {
               SetRelay(i, waterLevel > rule.threshold ? HIGH : LOW); // overflow alarm
            }
         } else {
            SetRelay(i, waterLevel > rule.threshold ? HIGH : LOW);
            if (waterLevel > rule.threshold) {
               state = rule.label;
            }
         }
         std::stringstream ss;
         ss << "relay" << i + 1;
         doc[ss.str()] = GetRelay(i) ? "on" : "off";

         i++;
      }
      if (_mode == Pump) {
         if (GetRelay(3)) {
            state = _thresholds[3].label;
         } else if (GetRelay(2) && GetRelay(1)) { // both relays on. ->Lag
            state = _thresholds[2].label;
         } else if (GetRelay(2) || GetRelay(1)) { // either relays on. ->Lag
            state = _thresholds[1].label;
         } else if (GetRelay(0)) { // at mark
            state = _thresholds[0].label;
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
      _iot.PostWeb(s);
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

void Tank::onSocketPong() {
   _lastWaterLevel = -1;
   _lastMessagePublished.clear(); // force a broadcast
}

void Tank::onNetworkState(NetworkState state) {
   _networkState = state;
#ifdef Has_TFT
   if (state >= NoNetwork) {
      delay(5000);                        // display the IP address for 5 seconds then display the gauge
      _tft.AnalogMeter(_relayThresholds); // setup analog display after APMode timeout
      _lastWaterLevel = 0;                // force publish to update tft
   }
#endif
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
   sprintf(buffer, "%s (%X)", APP_LOG_TAG, _iot.getUniqueId());
   device["model"] = buffer;
   JsonArray identifiers = device["identifiers"].to<JsonArray>();
   sprintf(buffer, "%X", _iot.getUniqueId());
   identifiers.add(buffer);

   logd("Discovery => topic: %s", topic.c_str());
   return _iot.PublishMessage(topic.c_str(), payload, true);
}

void Tank::onMqttMessage(char *topic, char *payload) {}
#endif
