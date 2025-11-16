#pragma once

namespace CLASSICDIY {
enum NetworkSelection { NotConnected, APMode, WiFiMode, EthernetMode, ModemMode };

enum NetworkState { Boot, ApState, Connecting, OnLine, OffLine };

enum IOTypes { DigitalInputs, AnalogInputs, DigitalOutputs, AnalogOutputs };

enum ModbusMode { TCP, RTU };

typedef enum {
   off,
   stop,
   slead,
   slag,
   overflow

} relayStatus;
} // namespace CLASSICDIY