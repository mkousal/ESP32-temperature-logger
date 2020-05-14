#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "settings.h"
#include "InfluxArduino.hpp"
#include "LM75A.h"
#include "credentials.h"  // Visit README.md for more info about this file
#include "U8g2lib.h"

U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(U8X8_PIN_NONE, SCL, SDA);

LM75A lm(true, true, true); // address pins in brackets (0x4F)
InfluxArduino influx;

// for WiFi and database settings look into 'credentials.h' file
// other settings can be found in 'settings.h' file

RTC_DATA_ATTR bool oledInit = false;

void initMCU() {
  #ifdef DEBUG
    Serial.begin(115200);                     // Initialize serial line with 115200 baudraute
  #endif
  WiFi.begin(WIFI_NAME, WIFI_PASS);           // Begin WiFi connection with SSID and password typed in 'credentials.h' file
  pinMode(19, OUTPUT);                        // WiFi status LED, when ON -> WiFi is connecting
  pinMode(23, INPUT_PULLUP);                  // Pin for disabling measurement
  #ifdef OLED
    if (!oledInit) {
      u8x8.begin();
      oledInit = true;
    }
    else {
      u8x8_gpio_Init(u8x8.getU8x8());
      u8x8_cad_Init(u8x8.getU8x8());
      u8x8_gpio_SetReset(u8x8.getU8x8(), 1);
    }
    #endif
}

void goSleep(long us) {
  esp_sleep_enable_timer_wakeup(us);
  WiFi.mode(WIFI_OFF);
  Serial.flush();
  Serial.end();
  esp_deep_sleep_start();
}

float getTemperature() {
  float temp = 0;
  for (int i = 0; i != TEMP_MEAS; i++) {
    temp += lm.getTemperatureInDegrees();   // Get temperature from connected LM75 sensor
    delay(100);
  }
  temp = temp / TEMP_MEAS;
  temp = ((round(temp*2))/2);
  return temp;
}

void displayTemperature(float temp) {
  char oledMSG[8];
  sprintf(oledMSG, "%0.1f°C", temp);
  u8x8.setFont(u8x8_font_inr21_2x4_f);
  uint8_t x = (u8x8.getCols() - (u8x8.getUTF8Len(oledMSG))*2) / 2;
  u8x8.drawUTF8(x, 0, oledMSG);
}

void setup() {
  initMCU();
  float temp = getTemperature();
  #ifdef OLED
    displayTemperature(temp);
  #endif
  if (digitalRead(23) == 0){
    goSleep(1000000);
  }

  int wifiAttempts = 0;                       // Prompt to store WiFi connection attempts
  digitalWrite(19, 1);                        // Turn WiFi status LED on

  while(WiFi.status() != WL_CONNECTED){       // Wait for connecting ESP32 to WiFi
    delay(500);
    Serial.print(".");
    wifiAttempts++;
    if(wifiAttempts == 60)                    // If WiFi connection fails after 60 attempts -> restart ESP
      esp_restart();
  }

  digitalWrite(19, 0);                        // Turn off WiFi status LED
  Serial.println("WiFi connected!");
  influx.configure(INFLUX_DATABASE, INFLUX_IP);  
  influx.authorize(INFLUX_USER, INFLUX_PASS);
  Serial.print("Using HTTPS: ");
  Serial.println(influx.isSecure());

  char tags[32];
  char fields[32];
  sprintf(tags, "new_tag=Yes");
  sprintf(fields, "temp=%0.1f",temp);
  bool writeSuccessful = influx.write(INFLUX_MEASUREMENT,tags,fields);

  if(!writeSuccessful){
    Serial.print("error: ");
    Serial.println(influx.getResponse());
  }

  goSleep(DELAY_TIME_US);
}

void loop() { }