#pragma once
#include <Arduino.h>
#include "Log.h"

#ifdef ESP32_DEV_BOARD

#define SensorPin GPIO_NUM_3
#define ADC_Resolution 4095.0

#define NUM_RELAYS 2                     // number of relays

// I2C
#define I2C_SDA GPIO_NUM_21
#define I2C_SCL GPIO_NUM_22

// relay
#define RELAY_1 GPIO_NUM_36
#define RELAY_2 GPIO_NUM_39

#endif
#ifdef Waveshare_Relay_6CH

#define RGB_LED_PIN GPIO_NUM_38
#define GPIO_PIN_Buzzer GPIO_NUM_21   // Buzzer Control GPIO
#define PWM_Channel 1                 // PWM Channel
#define Frequency 1000                // PWM frequencyconst
#define Resolution 8
#define Dutyfactor 200

void inline RGB_Light(uint8_t red_val, uint8_t green_val, uint8_t blue_val) {
   neopixelWrite(RGB_LED_PIN, green_val, red_val, blue_val); // RGB color adjustment
}

#define SensorPin GPIO_NUM_3
#define ADC_Resolution 4095.0

// UARTS
#define U0_TXD GPIO_NUM_43
#define U0_RXD GPIO_NUM_44

// I2C
#define I2C_SDA GPIO_NUM_47
#define I2C_SCL GPIO_NUM_48

#define NUM_RELAYS 6
#define RELAY_1 GPIO_NUM_1
#define RELAY_2 GPIO_NUM_2
#define RELAY_3 GPIO_NUM_41
#define RELAY_4 GPIO_NUM_42
#define RELAY_5 GPIO_NUM_45
#define RELAY_6 GPIO_NUM_46

#endif
#ifdef Lilygo_Relay_4CH

#define SensorPin A0
#define ADC_Resolution 4095.0

#define NUM_RELAYS 4
#define RELAY_1 GPIO_NUM_21 // RELAY GPIO switches
#define RELAY_2 GPIO_NUM_19
#define RELAY_3 GPIO_NUM_18
#define RELAY_4 GPIO_NUM_5

// I2C
#define I2C_SDA GPIO_NUM_15
#define I2C_SCL GPIO_NUM_14

// OLED display definitions
#define SCREEN_ADDRESS 0x3C // OLED 128X64 I2C address
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)

#endif
#ifdef Lilygo_Relay_6CH

#define NUM_RELAYS 6

// HT74HC595
#define HT74HC595_CLOCK GPIO_NUM_5
#define HT74HC595_LATCH GPIO_NUM_6
#define HT74HC595_DATA GPIO_NUM_7
#define HT74HC595_OUT_EN GPIO_NUM_4

#define RTC_IRQ GPIO_NUM_18

#ifdef Has_TFT
#define SensorPin GPIO_NUM_1
#else
#define SensorPin GPIO_NUM_8
#endif
#define ADC_Resolution 4095.0

// I2C
#define I2C_SDA GPIO_NUM_16
#define I2C_SCL GPIO_NUM_17

// OLED display definitions
#define SCREEN_ADDRESS 0x3C // OLED 128X64 I2C address
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)

#endif

