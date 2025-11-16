#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "IOTCallbackInterface.h"
#include "Enumerations.h"
#include "Oled.h"
#include "Log.h"
#include "Defines.h"
#include "AnalogSensor.h"

namespace CLASSICDIY {
class Tank : public IOTCallbackInterface {
 public:
   Tank() {};
   void setup();
   void Process();
#ifdef HasMQTT
   void onMqttConnect();
   void onMqttMessage(char *topic, char *payload);
#endif
#ifdef HasModbus
   bool onModbusMessage(ModbusMessage &msg);
   void onNetworkConnect();
#else
   void onNetworkConnect() {}; // noop
#endif
   void onSaveSetting(JsonDocument &doc);
   void onLoadSetting(JsonDocument &doc);
   void addApplicationConfigs(String &page);
   void onSubmitForm(AsyncWebServerRequest *request);
   uint16_t stopLevel = stopLevel_default;
   uint16_t startLeadLevel = startLeadLevel_default;
   uint16_t startLagLevel = startLagLevel_default;
   uint16_t overflowLevel = overflowLevel_default;

 protected:
#ifdef HasMQTT
   boolean PublishDiscoverySub(IOTypes type, const char *entityName, const char *unit_of_meas = nullptr, const char *icon = nullptr);
#endif
 private:
   AnalogSensor _Sensor = AnalogSensor(SensorPin);
   boolean _discoveryPublished = false;
   String _lastMessagePublished;
   float _lastWaterLevel = 0;
#ifdef Has_OLED_Display
   Oled _oled = Oled();
#endif
};
} // namespace CLASSICDIY
