
#include <Arduino.h>
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
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"

BLECharacteristic bmeLevelCharacteristics("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeLevelDescriptor(BLEUUID((uint16_t)0x2902));

BLECharacteristic bmeStatusCharacteristics("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeStatusDescriptor(BLEUUID((uint16_t)0x2903));

// Setup callbacks onConnect and onDisconnect
class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        pServer->getAdvertising()->stop();
    };
    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        pServer->getAdvertising()->start();
    }
};

void BLE::begin()
{
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

void BLE::update(uint16_t level, const char* state)
{
    if (deviceConnected)
    {
        static char flTemp[20];
        dtostrf(level, 6, 1, flTemp);
        // Set float level Characteristic value and notify connected client
        bmeLevelCharacteristics.setValue(flTemp);
        bmeLevelCharacteristics.notify();
        logi("Level: %s", flTemp);

        // Set relayStatus value and notify connected client
        bmeStatusCharacteristics.setValue(state);
        bmeStatusCharacteristics.notify();
        
    }
    else
    {
        logw("Device not connected, cannot update BLE characteristics");
    }
}
