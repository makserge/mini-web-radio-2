// Includes
#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include "driver/rtc_io.h"
#include "radio.h"
#include "config.h"
#include "led.h"
#include "scd.h"
#include "simple_timer.h"

// I2S
#define I2S_DOUT 16
#define I2S_BCLK 15
#define I2S_LRC 18

// LED
#define LED 17

// SCD41
#define I2C_SDA 11
#define I2C_SCL 10
#define SCD_INTERVAL 10000 //10s
#define SCD_THRESHOLD 1000 //1000ppm

#define BTN 21

#define BLINK_INTERVAL 250

// Globals
int longPress = 2000;
int buttonState;
unsigned long btnPressed = 0;

uint16_t co2, lastCo2;
float temp, hum, lastTemp, lastHum;
bool co2Threshold = false;
int blinkTimerId = 0;
int scdTimerId = 0;

// Init Libs
Config config;
Radio radio;
SimpleTimer timer;
//Led led(LED);
Scd scd;

/**
 * @brief Set the volume of the radio.
 * 
 * @param level Volume level to set (1-64).
 */
void setVolume(int level) {
    Serial.print("Volume:");
    Serial.println(level);
    radio.setVolume(level);
}

/**
 * @brief Read the state of the button and handle press events.
 */
void readButton() {
  buttonState = digitalRead(BTN);

  // Long press - shutdown radio
  if (buttonState == LOW && btnPressed == 0) {
      btnPressed = millis(); 
  } else if (buttonState == LOW && btnPressed > 0) {
      if ((millis() - btnPressed) > longPress) { 
          btnPressed = 0;
          delay(3000);
          esp_deep_sleep_start(); 
      }
  }
  
  // Short press - change station
  if (buttonState == HIGH && btnPressed > 0) { 
      if((millis() - btnPressed) > 250) {
          btnPressed = 0;
          radio.next();
      }
  }
}

/**
 * @brief Handle WiFi disconnect events and attempt reconnection.
 * 
 * @param event WiFi event type.
 * @param info WiFi event information.
 */
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  //led.setColour("000055");
  Serial.println("Disconnected from WiFi access point");
  Serial.println("Trying to Reconnect...");
  WiFi.reconnect();
}

void startBlinking() {
  timer.enable(blinkTimerId);
}

void stopBlinking() {
  timer.disable(blinkTimerId);
}

void showCO2(uint16_t co2) {
  if (co2 > 9999) {
    co2 = 9999;
  }
  if (co2 > SCD_THRESHOLD) {
    startBlinking();
  } else {
    stopBlinking();
    //led.setState("FF0000", 1, 1);
    digitalWrite(LED, HIGH);
  }
  Serial.print("CO2:");
  Serial.println(co2);
}

void showTemperature(float temp) {
  Serial.print("Temp:");
  Serial.println(temp);
}

void showHumidity(float hum) {
  Serial.print("Hum:");
  Serial.println(hum);
}

void blinkCallback() {
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
}

void scdCallback() {
  scd.readSensor(co2, temp, hum);
  if (lastCo2 != co2) {
    showCO2(co2);
    lastCo2 = co2;
  }  
  if (lastTemp != temp) {
    lastTemp = temp;
    showTemperature(temp);
  }
  if (lastHum != hum) {
    lastHum = hum;
    showHumidity(hum);
  }
}  

void setupTimers() {
  blinkTimerId = timer.setInterval(BLINK_INTERVAL, blinkCallback);
  timer.disable(blinkTimerId);
  scdTimerId = timer.setInterval(SCD_INTERVAL, scdCallback);
}

void setup() {
  Serial.begin(115200);
  
  // Setup RTC Wake
  rtc_gpio_pullup_en(GPIO_NUM_21);
  rtc_gpio_pulldown_dis(GPIO_NUM_21);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_21, 0);

  // Setup GPIO
  pinMode(LED, OUTPUT);
  pinMode(BTN, INPUT_PULLUP);
  
  // Initialise audio
  radio.init(I2S_DOUT, I2S_BCLK, I2S_LRC);

  scd.init(I2C_SDA, I2C_SCL);

  setupTimers();

  // Handle wifi disconnect
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  config.stMode();
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      //led.setState("FF0000", 1, 1);
      digitalWrite(LED, HIGH);
      String url = radio.readLine(0);
      radio.setVolume(32);
      radio.setStation(url);
      int treble = config.getTreble();
      int mid = config.getMid();
      int bass = config.getBass();
      radio.setTone(bass, mid, treble);
      radio.next();
  } else {
      config.apMode();
  }
  delay(1000);
}

void loop() {
  timer.run();
  readButton();
  if (WiFi.status() == WL_CONNECTED) {
    radio.play();
  } else{
    Serial.println("Offline");
    delay(500);
  }
}