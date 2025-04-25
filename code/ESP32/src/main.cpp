#include <Arduino.h>
#include <esp_task_wdt.h>
#include "Log.h"
#include "Tank.h"

using namespace HydroFloat;

#ifdef EdgeBox
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads; /* Use this for the 16-bit version */
#endif

Tank *_tank = new Tank();

void setup() {
  Serial.begin(115200);
  while (!Serial)  {  }

  esp_task_wdt_init(WATCHDOG_TIMEOUT, true); 
  // Add the current task to the watchdog timer
  esp_task_wdt_add(NULL);
  #ifdef EdgeBox
  Wire.begin(SDA, SCL);
	if (!ads.begin(0x48, &Wire))
	{
		loge("Failed to initialize ADS.");
	}
  pinMode(LTE_PWR_EN, OUTPUT); // turn off modem
  digitalWrite(LTE_PWR_EN, LOW);
  pinMode(LTE_PWR_KEY, OUTPUT);
  digitalWrite(LTE_PWR_KEY, LOW);
  #endif
  _tank->setup();
  logd("HTTP server started");
}

void loop(){
  _tank->Process();
  esp_task_wdt_reset(); // feed watchdog
}