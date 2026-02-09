#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "IOTCallbackInterface.h"
#include "IDisplayServiceInterface.h"
#include "IOTEnumerations.h"
#include "Log.h"
#include "Enumerations.h"
#include "Thresholds.h"
#include "Device.h"
#include "AnalogSensor.h"

using namespace CLASSICDIY;

class Tank : public Device, public IOTCallbackInterface {
 public:
   Tank() {};

   void Setup();
   void Process();
#ifdef HasMQTT
   void onMqttConnect(esp_mqtt_client_handle_t &client);
   void onMqttMessage(char *topic, char *payload);
#endif
#if defined(HasModbus) && defined(HasRS485)
   bool onModbusMessage(ModbusMessage &msg);
#endif
   void onNetworkState(NetworkState state);
   void onSocketPong();
   void onSaveSetting(JsonDocument &doc);
   void onLoadSetting(JsonDocument &doc);
   String appTemplateProcessor(const String &var);
#ifdef Has_OLED
   IDisplayServiceInterface& getDisplayInterface() override {  return _oled; };
#endif
#ifdef Has_TFT
   IDisplayServiceInterface& getDisplayInterface() override { return _tft; };
#endif

 protected:
#ifdef HasMQTT
   boolean PublishDiscoverySub(IOTypes type, const char *entityName, const char *unit_of_meas = nullptr, const char *icon = nullptr);
#endif
 private:
   Mode _mode; // float or pump mode of operation
   bool _alternate = false;
   String _base_state; // all relays off label
   std::vector<Thresholds> _thresholds;
   AnalogSensor _Sensor = AnalogSensor(SensorPin);
   boolean _discoveryPublished = false;
   String _lastMessagePublished;
   float _lastWaterLevel = 0;
   String _bodyBuffer;
   

};

