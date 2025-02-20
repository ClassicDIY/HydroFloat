#include <Arduino.h>
#include <WebServer.h>
#include <Update.h>
#include "Log.h"
#include "Tank.h"

using namespace HydroFloat;

Tank* _tank = new Tank();


// ToDo watchdog
// ToDo AP SSID / Password
// ToDo settings page style
// ToDo level validation

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
  }
 
  _tank->setup();
  logd("HTTP server started");
}

void loop()
{

  _tank->Process();
}