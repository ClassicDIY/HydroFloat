#pragma once
#include <Arduino.h>

#ifdef ESP_32Dev

#define Relays 2 // number of relays
#define WIFI_STATUS_PIN GPIO_NUM_2 // LED Pin
#define FACTORY_RESET_PIN GPIO_NUM_4 // Clear NVRAM

// I2C
#define I2C_SDA GPIO_NUM_21
#define I2C_SCL GPIO_NUM_22

// relay
#define DO0 GPIO_NUM_36
#define DO1 GPIO_NUM_39

#elif Waveshare_Relay_6CH

#define RGB_LED_PIN GPIO_NUM_38
#define GPIO_PIN_Buzzer GPIO_NUM_21 // Buzzer Control GPIO
#define FACTORY_RESET_PIN GPIO_NUM_12 // Clear NVRAM
#define PWM_Channel 1 // PWM Channel
#define Frequency 1000 // PWM frequencyconst
#define Resolution 8
#define Dutyfactor 200

void inline RGB_Light(uint8_t red_val, uint8_t green_val, uint8_t blue_val) {
   neopixelWrite(RGB_LED_PIN, green_val, red_val, blue_val); // RGB color adjustment
}

void inline Buzzer_PWM(uint16_t Time) // ledChannel：PWM Channe    dutyfactor:dutyfactor
{
   ledcWrite(PWM_Channel, Dutyfactor);
   delay(Time);
   ledcWrite(PWM_Channel, 0);
}

#define SensorPin GPIO_NUM_3

// UARTS
#define U0_TXD GPIO_NUM_43
#define U0_RXD GPIO_NUM_44

// RS485
#define RS485_TXD GPIO_NUM_17
#define RS485_RXD GPIO_NUM_18
#define RS485_RTS -1

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


void inline GPIO_Init() {
   pinMode(RELAY_1, OUTPUT);
   pinMode(RELAY_2, OUTPUT);
   pinMode(RELAY_3, OUTPUT);
   pinMode(RELAY_4, OUTPUT);
   pinMode(RELAY_5, OUTPUT);
   pinMode(RELAY_6, OUTPUT);
   pinMode(RGB_LED_PIN, OUTPUT);     // Initialize the control GPIO of RGB
   pinMode(GPIO_PIN_Buzzer, OUTPUT); // Initialize the control GPIO of Buzzer

   ledcSetup(PWM_Channel, Frequency, Resolution); // Set PWM channel
   ledcAttachPin(GPIO_PIN_Buzzer, PWM_Channel);   // Connect the channel to the corresponding pin

}

#elif Lilygo_Relay_4CH

#define WIFI_STATUS_PIN GPIO_NUM_25  // LED Pin on the ESP32 Lilygo T-Relay board
#define FACTORY_RESET_PIN GPIO_NUM_4 // Clear NVRAM

#define SensorPin A0

#define NUM_RELAYS 4
#define RELAY_1 GPIO_NUM_21 // RELAY GPIO switches
#define RELAY_2 GPIO_NUM_19
#define RELAY_3 GPIO_NUM_18
#define RELAY_4 GPIO_NUM_5

void inline GPIO_Init() {
   pinMode(RELAY_1, OUTPUT);
   pinMode(RELAY_2, OUTPUT);
   pinMode(RELAY_3, OUTPUT);
   pinMode(RELAY_4, OUTPUT);
   pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
   pinMode(WIFI_STATUS_PIN, OUTPUT);
}

// I2C
#define I2C_SDA GPIO_NUM_15
#define I2C_SCL GPIO_NUM_14

// OLED display definitions
#define SCREEN_ADDRESS 0x3C // OLED 128X64 I2C address
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)

#elif Lilygo_Relay_6CH

#define WIFI_STATUS_PIN GPIO_NUM_25  // LED Pin on the ESP32 Lilygo T-Relay board
#define FACTORY_RESET_PIN GPIO_NUM_4 // Clear NVRAM
#define NUM_RELAYS 6

// HT74HC595
#define HT74HC595_CLOCK GPIO_NUM_5
#define HT74HC595_LATCH GPIO_NUM_6
#define HT74HC595_DATA GPIO_NUM_7
#define HT74HC595_OUT_EN GPIO_NUM_4

#define RTC_IRQ GPIO_NUM_18

#define SensorPin A0

void inline GPIO_Init() {}

// I2C
#define I2C_SDA GPIO_NUM_16
#define I2C_SCL GPIO_NUM_17

// OLED display definitions
#define SCREEN_ADDRESS 0x3C // OLED 128X64 I2C address
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)

#elif ESP32_X4_Relay_board // old blue board

#define SensorPin A0
#define SDA_PIN GPIO_NUM_21 // OLED Display SDA and SLC pins
#define SLC_PIN GPIO_NUM_22
#define NUM_RELAYS 4
#define RELAY_1 GPIO_NUM_32 // RELAY GPIO switches
#define RELAY_2 GPIO_NUM_33
#define RELAY_3 GPIO_NUM_25
#define RELAY_4 GPIO_NUM_26
#define WIFI_STATUS_PIN GPIO_NUM_23  // LED Pin on the ESP32 X4 Relay board
#define FACTORY_RESET_PIN GPIO_NUM_4 // Clear NVRAM

#endif
