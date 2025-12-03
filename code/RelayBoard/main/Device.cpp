#include <Arduino.h>
#include <memory>
#include "Wire.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "Log.h"
#include "Device.h"

#ifdef Has_OLED
Adafruit_SSD1306 oled_display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

namespace CLASSICDIY {

#ifdef Lilygo_Relay_6CH
#include <ShiftRegister74HC595.h>
std::shared_ptr<ShiftRegister74HC595<1>> HT74HC595 = std::make_shared<ShiftRegister74HC595<1>>(HT74HC595_DATA, HT74HC595_CLOCK, HT74HC595_LATCH);

void Device::Init() {
   pinMode(HT74HC595_OUT_EN, OUTPUT);
   digitalWrite(HT74HC595_OUT_EN, HIGH);
   HT74HC595->setAllLow();
   logd("Set HT74HC595_OUT_EN to low level to enable relay output");
   digitalWrite(HT74HC595_OUT_EN, LOW);

   Wire.begin(I2C_SDA, I2C_SCL);
#ifdef Has_OLED
   if (!oled_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      loge("SSD1306 allocation failed");
   } else {
      oled_display.clearDisplay();
   }
#endif
}

void Device::Run() {
   if (_networkState != OnLine) {
      unsigned long binkRate = _networkState == ApState ? AP_BLINK_RATE : NC_BLINK_RATE;
      unsigned long now = millis();
      if (binkRate < now - _lastBlinkTime) {
         _blinkStateOn = !_blinkStateOn;
         _lastBlinkTime = now;
         HT74HC595->set(7, _blinkStateOn ? HIGH : LOW);
      }
   } else if (!_running) {
      HT74HC595->set(7, LOW);
      HT74HC595->set(6, HIGH);
      _running = true;
   }
}

void Device::SetRelay(const uint8_t index, const uint8_t value) { HT74HC595->set(index, value); }

boolean Device::GetRelay(const uint8_t index) { return HT74HC595->get(index); }

#elif ESP32_DEV_BOARD

gpio_num_t _relays[NUM_RELAYS] = {RELAY_1, RELAY_2};

void Device::Init() {
   Wire.begin(I2C_SDA, I2C_SCL);
   for (int i = 0; i < NUM_RELAYS; i++) {
      pinMode(_relays[i], OUTPUT);
   }
   pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
   pinMode(WIFI_STATUS_PIN, OUTPUT);
}

void Device::Run() {
   // handle blink led, fast : NotConnected slow: AP connected On: Station connected
   if (_networkState != OnLine) {
      unsigned long binkRate = _networkState == ApState ? AP_BLINK_RATE : NC_BLINK_RATE;
      unsigned long now = millis();
      if (binkRate < now - _lastBlinkTime) {
         _blinkStateOn = !_blinkStateOn;
         _lastBlinkTime = now;
         digitalWrite(WIFI_STATUS_PIN, _blinkStateOn ? HIGH : LOW);
      }
   } else {
      digitalWrite(WIFI_STATUS_PIN, HIGH);
   }
}

void Device::SetRelay(const uint8_t index, const uint8_t value) { digitalWrite(_relays[index], value); }

boolean Device::GetRelay(const uint8_t index) { return digitalRead(_relays[index]) == 0 ? false : true; }

#elif Lilygo_Relay_4CH

gpio_num_t _relays[NUM_RELAYS] = {RELAY_1, RELAY_2, RELAY_3, RELAY_4};

void Device::Init() {
   Wire.begin(I2C_SDA, I2C_SCL);
#ifdef Has_OLED
   if (!oled_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      loge("SSD1306 allocation failed");
   } else {
      oled_display.clearDisplay();
   }
#endif
   for (int i = 0; i < NUM_RELAYS; i++) {
      pinMode(_relays[i], OUTPUT);
   }
   pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
   pinMode(WIFI_STATUS_PIN, OUTPUT);
   if (!LittleFS.begin()) {
      loge("LittleFS mount failed");
   }
}

void Device::Run() {
   // handle blink led, fast : NotConnected slow: AP connected On: Station connected
   if (_networkState != OnLine) {
      unsigned long binkRate = _networkState == ApState ? AP_BLINK_RATE : NC_BLINK_RATE;
      unsigned long now = millis();
      if (binkRate < now - _lastBlinkTime) {
         _blinkStateOn = !_blinkStateOn;
         _lastBlinkTime = now;
         digitalWrite(WIFI_STATUS_PIN, _blinkStateOn ? HIGH : LOW);
      }
   } else {
      digitalWrite(WIFI_STATUS_PIN, HIGH);
   }
}

void Device::SetRelay(const uint8_t index, const uint8_t value) { digitalWrite(_relays[index], value); }

boolean Device::GetRelay(const uint8_t index) { return digitalRead(_relays[index]) == 0 ? false : true; }

#elif Waveshare_Relay_6CH

gpio_num_t _relays[NUM_RELAYS] = {RELAY_1, RELAY_2, RELAY_3, RELAY_4, RELAY_5, RELAY_6};

void Device::Init() {
   Wire.begin(I2C_SDA, I2C_SCL);
#ifdef Has_OLED
   if (!oled_display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      loge("SSD1306 allocation failed");
   } else {
      oled_display.clearDisplay();
   }
#endif
   for (int i = 0; i < NUM_RELAYS; i++) {
      pinMode(_relays[i], OUTPUT);
   }
   pinMode(RGB_LED_PIN, OUTPUT);     // Initialize the control GPIO of RGB
   pinMode(GPIO_PIN_Buzzer, OUTPUT); // Initialize the control GPIO of Buzzer

   ledcSetup(PWM_Channel, Frequency, Resolution); // Set PWM channel
   ledcAttachPin(GPIO_PIN_Buzzer, PWM_Channel);   // Connect the channel to the corresponding pin
}

void Device::Run() {
   if (_networkState != OnLine) {
      unsigned long binkRate = _networkState == ApState ? AP_BLINK_RATE : NC_BLINK_RATE;
      unsigned long now = millis();
      if (binkRate < now - _lastBlinkTime) {
         _blinkStateOn = !_blinkStateOn;
         _lastBlinkTime = now;
         neopixelWrite(RGB_LED_PIN, _blinkStateOn ? 60 : 0, _blinkStateOn ? 0 : 60, 0);
      }
   } else if (!_running) {
      neopixelWrite(RGB_LED_PIN, 0, 0, 60);
      _running = true;
   }
}

void Device::SetRelay(const uint8_t index, const uint8_t value) { digitalWrite(_relays[index], value); }

boolean Device::GetRelay(const uint8_t index) { return digitalRead(_relays[index]) == 0 ? false : true; }

void inline Buzzer_PWM(uint16_t Time) // ledChannel：PWM Channe    dutyfactor:dutyfactor
{
   ledcWrite(PWM_Channel, Dutyfactor);
   delay(Time);
   ledcWrite(PWM_Channel, 0);
}

#endif

} // namespace CLASSICDIY