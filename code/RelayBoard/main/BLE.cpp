#include <Arduino.h>
#ifdef Has_BT
#include "Log.h"
#include <NimBLEDevice.h>
#include <Wire.h>

#include "BLE.h"

// BLE server name
#define bleServerName "BME280_ESP32"

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

NimBLEServer *pServer;
bool deviceConnected = false;

// UUIDs
#define SERVICE_UUID "bab4685d-8523-437b-b683-bf010c9de021"
#define LEVEL_CHAR_UUID "1867b1f0-5ba0-4c3c-8989-418c56698a6a"
#define STATUS_CHAR_UUID "ed8d106c-35e5-427c-9c34-924f214dc3c5"

NimBLECharacteristic *bmeLevelCharacteristics;
NimBLECharacteristic *bmeStatusCharacteristics;

// Callbacks for connect/disconnect
class MyServerCallbacks : public NimBLEServerCallbacks {
   void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override {
      logd("onConnect");
      deviceConnected = true;
      pServer->getAdvertising()->stop();
   }
   void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason) override {
      logd("onDisconnect");
      deviceConnected = false;
      pServer->getAdvertising()->start();
   }
};

void BLE::begin() {
   NimBLEDevice::init(bleServerName);
   NimBLEDevice::setDeviceName("BME280_ESP32");
   pServer = NimBLEDevice::createServer();
   pServer->setCallbacks(new MyServerCallbacks());
   NimBLEDevice::setSecurityAuth(false, false, false);
   NimBLEService *bmeService = pServer->createService(SERVICE_UUID);
   bmeLevelCharacteristics = bmeService->createCharacteristic(LEVEL_CHAR_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
   bmeStatusCharacteristics = bmeService->createCharacteristic(STATUS_CHAR_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
   bmeService->start();
   NimBLEAdvertisementData advData;
   advData.setName("BME280_ESP32");                       // Device name
   advData.setCompleteServices(NimBLEUUID(SERVICE_UUID)); // Service UUID
   NimBLEAdvertisementData scanData;
   scanData.setName("BME280_ESP32");
   // Start advertising
   NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
   pAdvertising->addServiceUUID(SERVICE_UUID);
   pAdvertising->setAdvertisementData(advData);
   pAdvertising->setScanResponseData(scanData);
   pAdvertising->start();
   logd("Waiting for client connection to notify...");
}

void BLE::update(uint16_t level, const char *state) {
   if (deviceConnected) {
      char buffer[16];
      sprintf(buffer, "%d%%", level);
      bmeLevelCharacteristics->setValue(buffer);
      bmeLevelCharacteristics->notify();
      bmeStatusCharacteristics->setValue(state);
      bmeStatusCharacteristics->notify();
   } else {
      logd("Device not connected, cannot update BLE characteristics");
   }
}
#endif