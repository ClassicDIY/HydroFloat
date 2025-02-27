#include <Arduino.h>
#include <esp_task_wdt.h>
#include "Log.h"
#include "Tank.h"

using namespace HydroFloat;

Tank *_tank = new Tank();

void setup() {
  Serial.begin(115200);
  while (!Serial)  {  }
  _tank->setup();
  esp_task_wdt_init(WATCHDOG_TIMEOUT, true); 
  // Add the current task to the watchdog timer
  esp_task_wdt_add(NULL);
  logd("HTTP server started");
}

void loop(){
  _tank->Process();
  esp_task_wdt_reset(); // feed watchdog
}