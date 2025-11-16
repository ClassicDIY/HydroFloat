#include <Arduino.h>
#include <esp_task_wdt.h>
#include "Log.h"
#include "Tank.h"

using namespace CLASSICDIY;

Tank *_tank = new Tank();
#ifdef Has_OLED_Display
Adafruit_SSD1306 oled_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

void setup() {
#if defined(Waveshare_Relay_6CH)
   delay(5000);
#endif
   Serial.begin(115200);
   while (!Serial) {
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
   GPIO_Init();
   Wire.begin(I2C_SDA, I2C_SCL);
#ifdef Has_OLED_Display
   if (!oled_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      loge("SSD1306 allocation failed");
   } else {
      oled_display.clearDisplay();
   }
#endif
   esp_task_wdt_init(60, true); // 60-second timeout, panic on timeout
   esp_task_wdt_add(NULL);
   _tank->setup();
   logd("Setup Done");
}

void loop() {
   _tank->Process();
   esp_task_wdt_reset(); // feed watchdog
   delay(10);
}