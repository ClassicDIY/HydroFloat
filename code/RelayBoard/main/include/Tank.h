#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "IOTCallbackInterface.h"
#include "Enumerations.h"
#include "Log.h"
#include "Defines.h"
#include "Device.h"
#include "AnalogSensor.h"

namespace CLASSICDIY {

struct Thresholds {
   uint16_t threshold;
   String label;
   bool active;
};

class Tank : public Device, public IOTCallbackInterface {
 public:
   Tank() {};

   void Setup();
   void Process();
#ifdef HasMQTT
   void onMqttConnect();
   void onMqttMessage(char *topic, char *payload);
#endif
#if defined(HasModbus) && defined(HasRS485)
   bool onModbusMessage(ModbusMessage &msg);
#endif
   void onNetworkState(NetworkState state);
   void onSaveSetting(JsonDocument &doc);
   void onLoadSetting(JsonDocument &doc);
   String appTemplateProcessor(const String &var);
#ifdef Has_OLED
   void update(const char *mode, const char *detail);
   void update(const char *mode, int count);
#endif

 protected:
#ifdef HasMQTT
   boolean PublishDiscoverySub(IOTypes type, const char *entityName, const char *unit_of_meas = nullptr, const char *icon = nullptr);
#endif
 private:
   std::vector<Thresholds> _relayThresholds;
   AnalogSensor _Sensor = AnalogSensor(SensorPin);
   boolean _discoveryPublished = false;
   String _lastMessagePublished;
   float _lastWaterLevel = 0;
   String _bodyBuffer;
};
} // namespace CLASSICDIY
