#include <Arduino.h>
#include <WebServer.h>
#include <Update.h>
#include "Log.h"
#include "Tank.h"

using namespace HydroFloat;

Tank *_tank = new Tank();
hw_timer_t *timer = NULL;

// ToDo AP SSID / Password
// ToDo settings page style
// ToDo I2C Display Module

void feed_watchdog()
{
  if (timer != NULL)
  {
    timerWrite(timer, 0); // feed the watchdog
  }
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
  }
  _tank->setup();
  if (timer == NULL)
  {
    timer = timerBegin(0, 80, true); // timer 0, div 80
    timerAttachInterrupt(timer, []() { esp_restart(); }, true); // attach callback
    timerAlarmWrite(timer, WATCHDOG_TIMER * 1000, false); // set time in us
    timerAlarmEnable(timer); // enable interrupt
  }
  logd("HTTP server started");
}

void loop()
{
  feed_watchdog();
  _tank->Process();
}