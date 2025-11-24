#pragma once
#ifdef Has_BT

#include "Enumerations.h"

class BLE {
 public:
   BLE() {};
   void begin();
   void update(uint16_t level, const char *state);

 private:
};
#endif