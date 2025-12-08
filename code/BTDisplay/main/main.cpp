
/*
#################################################################################
###### DON'T FORGET TO UPDATE THE User_Setup_Select.h FILE IN THE LIBRARY  ######
###### Comment out "#include <User_Setup.h>" line 27                       ######
###### uncomment "#include <User_Setups/Setup25_TTGO_T_Display.h>" line 58 ######
#################################################################################
*/

#include <Arduino.h>
#include "Log.h"
#include "BLEDevice.h"

#include <User_Setups/Setup25_TTGO_T_Display.h>

#include <TFT_eSPI.h> // Include the graphics library

TFT_eSPI oled_display = TFT_eSPI(); // Create object "oled_display"

// BLE Server name (the other ESP32 name running the server sketch)
#define bleServerName "BME280_ESP32"

/* UUID's of the service, characteristic that we want to read*/
// BLE Service
static BLEUUID bmeServiceUUID("bab4685d-8523-437b-b683-bf010c9de021");

// BLE Characteristics

// Level Characteristic
static BLEUUID levelCharacteristicUUID("1867b1f0-5ba0-4c3c-8989-418c56698a6a");

// Status Characteristic
static BLEUUID statusCharacteristicUUID("ed8d106c-35e5-427c-9c34-924f214dc3c5");

static BLEClient *pClient = nullptr;
static boolean doConnect = false;
static boolean connected = false;
static BLEAdvertisedDevice *myDevice;

// Characteristicd that we want to read
static BLERemoteCharacteristic *levelCharacteristic;
static BLERemoteCharacteristic *statusCharacteristic;

// Activate notify
const uint8_t notificationOn[] = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

#define SCREEN_WIDTH 240  // OLED display width, in pixels
#define SCREEN_HEIGHT 135 // OLED display height, in pixels
#define STATUS_TEXT_SIZE 3
#define LEVEL_TEXT_SIZE 7
#define STATUS_TEXT_POS 20
#define LEVEL_TEXT_POS 70

uint8_t xOffset(uint8_t textSize, uint8_t numberOfCharaters)
{
  uint8_t textPixels = textSize * 6;
  uint8_t rVal = (SCREEN_WIDTH - (numberOfCharaters * textPixels)) / 2;
  // logd("Offset: %d", rVal);
  return rVal;
}

// When the BLE Server sends a new level reading with the notify property
static void levelNotifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
  logd("levelNotifyCallback, length: %d", length);
  if (length < 20)
  {
    char levelChar[20];
    strncpy(levelChar, (char *)pData, length);
    levelChar[length] = '\0';
    oled_display.setCursor(xOffset(LEVEL_TEXT_SIZE, length), LEVEL_TEXT_POS);
    oled_display.setTextColor(TFT_BLUE, TFT_BLACK);
    oled_display.setTextSize(LEVEL_TEXT_SIZE);
    oled_display.fillRect(0, STATUS_TEXT_POS, SCREEN_WIDTH, SCREEN_HEIGHT, TFT_BLACK);
    oled_display.print(levelChar);
  }
}

// When the BLE Server sends a new status reading with the notify property
static void statusNotifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
  logd("statusNotifyCallback");
  if (length < 16)
  {
    char statusChar[16];
    strncpy(statusChar, (char *)pData, length);
    statusChar[length] = '\0'; // Ensure null termination
    oled_display.setTextSize(STATUS_TEXT_SIZE);
    oled_display.setCursor(xOffset(STATUS_TEXT_SIZE, length), STATUS_TEXT_POS);
    oled_display.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
    oled_display.fillRect(0, 0, SCREEN_WIDTH, LEVEL_TEXT_POS, TFT_BLACK);
    oled_display.print(statusChar);
  }
}

// Connect to the BLE Server that has the name, Service, and Characteristics
bool connectToServer()
{
  pClient->connect(myDevice); // Connect to the Bluetooth server
  if (pClient->isConnected())
  {
    logi("Connected to server!");
  }
  else
  {
    logw("Failed to connect, retrying...");
  }

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(bmeServiceUUID);
  if (pRemoteService == nullptr)
  {
    loge("Failed to find our service UUID: %s", bmeServiceUUID.toString().c_str());
  }

  // Obtain a reference to the characteristics in the service of the remote BLE server.
  levelCharacteristic = pRemoteService->getCharacteristic(levelCharacteristicUUID);
  if (levelCharacteristic == nullptr)
  {
    loge("Failed to find our characteristic UUID");
  }
  else
  {
    logi(" - Found levelCharacteristic");

    // Assign callback functions for the Characteristics
    levelCharacteristic->registerForNotify(levelNotifyCallback);
  }

  statusCharacteristic = pRemoteService->getCharacteristic(statusCharacteristicUUID);

  if (statusCharacteristic == nullptr)
  {
    loge("Failed to find our characteristic UUID");
  }
  else
  {
    logi(" - Found statusCharacteristic");
    statusCharacteristic->registerForNotify(statusNotifyCallback);
  }
  return true;
}

// Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    logd("onResult! %s", advertisedDevice.getName().c_str());
    if (advertisedDevice.getName() == bleServerName)
    {
      advertisedDevice.getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      logd("Device found. Connecting!");
    }
  }
};

void setup()
{
  // wait for Serial to connect, give up after 5 seconds, USB may not be connected
  delay(3000);
  unsigned long start = millis();
  Serial.begin(115200);
  while (!Serial)
  {
    if (5000 < millis() - start)
    {
      break;
    }
  }
  logd("Starting BLE Client ");

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

  oled_display.init();
  oled_display.setRotation(1);
  oled_display.fillScreen(TFT_BLACK);
  oled_display.setTextColor(TFT_YELLOW, TFT_BLACK);
  oled_display.setTextSize(4);
  oled_display.setCursor(10, 10);

  oled_display.print("Scanning");

  // Init BLE device
  BLEDevice::init("");
  pClient = BLEDevice::createClient();
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void loop()
{
  if (doConnect)
  {
    if (connectToServer())
    {
      logi("We are now connected to the BLE Server.");
      // Activate the Notify property of each Characteristic
      BLERemoteDescriptor *pDesc = levelCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902));
      if (pDesc != nullptr)
      {
        logi("set levelCharacteristic.");
        pDesc->writeValue((uint8_t *)notificationOn, 2, true);
      }
      BLERemoteDescriptor *pDescsts = statusCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902));
      if (pDescsts != nullptr)
      {
        logi("set statusCharacteristic.");
        pDescsts->writeValue((uint8_t *)notificationOn, 2, true);
      }
      oled_display.fillScreen(TFT_BLACK);
    }
    else
    {
      loge("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }

  if (pClient->isConnected() == false)
  {
    logi("Trying to reconnect...");
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->start(5, false);
    delay(5000);
  }
  delay(1000); // Delay a second between loops.
}
