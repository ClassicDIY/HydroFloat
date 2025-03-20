
#pragma once
#define TAG "HydroFloat"

#define WATCHDOG_TIMEOUT 10 // time in seconds to trigger the watchdog reset
#define EEPROM_SIZE 512
#define AP_BLINK_RATE 600
#define NC_BLINK_RATE 100
#define DEFAULT_AP_PASSWORD "12345678"

#define ADC_Resolution 4095.0
#define SAMPLESIZE 20

#define SCREEN_ADDRESS 0x3C // OLED 128X64 I2C address
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

#define ASYNC_WEBSERVER_PORT 80
#define DNS_PORT 53

#define stopLevel_default 20
#define startLeadLevel_default 40
#define startLagLevel_default 60
#define overflowLevel_default 80

#define SensorPin A0 // 4-20 Sensor using 2 X 270Ω resistors in parallel to get 135Ω
#define SensorVoltageMin 540 // Mininum output voltage from Sensor in mV (135 * .004 = 540)
#define SensorVoltageMax 2700 // Maximum output voltage from Sensor in mV (135 * .02 = 2700).

#ifdef ESP32_X4_Relay_board
    #define SDA_PIN GPIO_NUM_21 //OLED Display SDA and SLC pins
    #define SLC_PIN GPIO_NUM_22
    #define RELAY_1 32 //RELAY GPIO switches
    #define RELAY_2 33
    #define RELAY_3 25
    #define RELAY_4 26
    #define WIFI_STATUS_PIN 23 //LED Pin on the ESP32 X4 Relay board
    #define FACTORY_RESET_PIN 4 // Clear NVRAM

#elif ESP32_Lilygo_TRelay_board
    #define SDA_PIN GPIO_NUM_15 //OLED Display SDA and SLC pins
    #define SLC_PIN GPIO_NUM_14
    #define RELAY_1 21 // RELAY GPIO switches
    #define RELAY_2 19
    #define RELAY_3 18
    #define RELAY_4 05
    #define WIFI_STATUS_PIN 25 //LED Pin on the ESP32 Lilygo T-Relay board
    #define FACTORY_RESET_PIN 4 // Clear NVRAM

#else // Dev board
    #define SDA_PIN GPIO_NUM_21 //OLED Display SDA and SLC pins
    #define SLC_PIN GPIO_NUM_22
    #define RELAY_1 32 // RELAY GPIO switches
    #define RELAY_2 33
    #define RELAY_3 25
    #define RELAY_4 26
    #define WIFI_STATUS_PIN 2 //LED Pin on the Dev board
    #define FACTORY_RESET_PIN 4 // Clear NVRAM

#endif