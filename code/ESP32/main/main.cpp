#include <Arduino.h>
#include <esp_task_wdt.h>
#include "Log.h"
#include "Tank.h"

using namespace CLASSICDIY;

Tank *_tank = new Tank();

void setup() {
   // wait for Serial to connect, give up after 5 seconds, USB may not be connected
   delay(3000);
   unsigned long start = millis();
   Serial.begin(115200);
   while (!Serial) {
      if (5000 < millis() - start) {
         break;
      }
   }

   logd("------------ESP32 specifications ---------------");
   logd("Chip Model: %s", ESP.getChipModel());
   logd("Chip Revision: %d", ESP.getChipRevision());
   logd("Number of CPU Cores: %d", ESP.getChipCores());
   logd("CPU Frequency: %d MHz", ESP.getCpuFreqMHz());
   logd("Flash Memory Size: %d MB", ESP.getFlashChipSize() / (1024 * 1024));
   logd("Flash Frequency: %d MHz", ESP.getFlashChipSpeed() / 1000000);
   logd("Heap Size: %d KB", ESP.getHeapSize() / 1024);
   logd("Free Heap: %d KB", ESP.getFreeHeap() / 1024);
   logd("------------ESP32 specifications ---------------");
   esp_task_wdt_init(60, true); // 60-second timeout, panic on timeout
   esp_task_wdt_add(NULL);
   _tank->Setup();
   logd("Setup Done");
}

void loop() {
   _tank->Process();
   esp_task_wdt_reset(); // feed watchdog
   delay(10);
}