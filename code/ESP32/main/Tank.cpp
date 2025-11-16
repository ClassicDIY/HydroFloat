#include <Arduino.h>
#include <ArduinoJson.h>
#include "Log.h"
#include "WebLog.h"
#include "IOT.h"
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
      serializeJson(doc, s);
      _webSocket.textAll(s);
#ifdef Has_OLED_Display
      _oled.update(waterLevel, s4 ? overflow : s3 ? slag : s2 ? slead : s1 ? stop : off);
#endif
      logd("broadcast JSON: %s", s.c_str());
   }
   return;
}
#ifdef HasModbus
void Tank::onNetworkConnect() {}

bool Tank::onModbusMessage(ModbusMessage &msg) { bool rval = false; }

#endif

#ifdef HasMQTT

void Tank::onMqttConnect() {}
void Tank::onMqttMessage(char *topic, char *payload) {}
#endif

} // namespace CLASSICDIY