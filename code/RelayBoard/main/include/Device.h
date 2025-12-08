#pragma once
#include <Arduino.h>
#include "GPIO_pins.h"
#include "Enumerations.h"
#include "Oled.h"

namespace CLASSICDIY {
class Device {
 protected:
   void Init();
   void InitCommon();
   void Run();
   uint16_t NumberOfRelays() { return NUM_RELAYS; }
   void SetRelay(const uint8_t index, const uint8_t value);
   boolean GetRelay(const uint8_t index);
#ifdef Has_OLED
   Oled _oled = Oled();
#endif
   NetworkState _networkState = Boot;
   unsigned long _lastBlinkTime = 0;
   bool _blinkStateOn = false;
   bool _running = false;
};

} // namespace CLASSICDIY