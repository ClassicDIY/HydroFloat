
#include <Arduino.h>
#ifdef Has_BT
#include "Log.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>

#include "BLE.h"

// BLE server name
#define bleServerName "BME280_ESP32"

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

BLEServer *pServer;
bool deviceConnected = false;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "bab4685d-8523-437b-b683-bf010c9de021"

BLECharacteristic bmeLevelCharacteristics("1867b1f0-5ba0-4c3c-8989-418c56698a6a", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeLevelDescriptor(BLEUUID((uint16_t)0x2902));

BLECharacteristic bmeStatusCharacteristics("ed8d106c-35e5-427c-9c34-924f214dc3c5", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeStatusDescriptor(BLEUUID((uint16_t)0x2903));

// Setup callbacks onConnect and onDisconnect
class MyServerCallbacks : public BLEServerCallbacks {
   void onConnect(BLEServer *pServer) {
      deviceConnected = true;
      pServer->getAdvertising()->stop();
   };
   void onDisconnect(BLEServer *pServer) {
      deviceConnected = false;
      pServer->getAdvertising()->start();
   }
};

void BLE::begin() {
   BLEDevice::init(bleServerName);
   pServer = BLEDevice::createServer();
   pServer->setCallbacks(new MyServerCallbacks());
   BLEService *bmeService = pServer->createService(SERVICE_UUID);
   bmeService->addCharacteristic(&bmeLevelCharacteristics);
   bmeLevelDescriptor.setValue("BME Float Level");
   bmeLevelCharacteristics.addDescriptor(&bmeLevelDescriptor);
   bmeService->addCharacteristic(&bmeStatusCharacteristics);
   bmeStatusDescriptor.setValue("BME Float State");
   bmeStatusCharacteristics.addDescriptor(&bmeStatusDescriptor);
   bmeService->start();

   // Start advertising
   BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
   pAdvertising->addServiceUUID(SERVICE_UUID);
   pServer->getAdvertising()->start();
   logi("Waiting a client connection to notify...");
}

void BLE::update(uint16_t level, const char *state) {
   if (deviceConnected) {
      char buffer[16];
      sprintf(buffer, "%d%%", level);
      // Set float level Characteristic value and notify connected client
      bmeLevelCharacteristics.setValue(buffer);
      bmeLevelCharacteristics.notify();
      // Set relayStatus value and notify connected client
      bmeStatusCharacteristics.setValue(state);
      bmeStatusCharacteristics.notify();

   } else {
      logw("Device not connected, cannot update BLE characteristics");
   }
}
#endif