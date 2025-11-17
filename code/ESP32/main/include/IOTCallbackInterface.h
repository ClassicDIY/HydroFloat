#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#ifdef HasModbus
#include <ModbusMessage.h>
#endif
#include "Enumerations.h"

namespace CLASSICDIY {
class IOTCallbackInterface {
 public:
   virtual void onNetworkState(NetworkState state) = 0;
   virtual void addApplicationConfigs(String &page);
   virtual void onSubmitForm(AsyncWebServerRequest *request);
   virtual void onSaveSetting(JsonDocument &doc);
   virtual void onLoadSetting(JsonDocument &doc);
#ifdef HasMQTT
   virtual void onMqttConnect() = 0;
   virtual void onMqttMessage(char *topic, char *payload) = 0;
#endif
};
} // namespace CLASSICDIY