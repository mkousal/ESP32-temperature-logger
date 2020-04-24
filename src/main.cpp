#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "InfluxArduino.hpp"
#include "LM75A.h"
#include "credentials.h"  // Visit README.md for more info about this file

LM75A lm(true, true, true); // address pins in brackets (0x4F)
InfluxArduino influx;

//for settings look into credentials.h file

#define DELAY_TIME_US 300 * 1000 * 1000 //5 minutes
RTC_DATA_ATTR unsigned long count = 0;  //store count variable into RTC memory for use after waking up from deep sleep
#define TEMP_MEAS 50 //number of temperature measurement to calculate average temp --> better accuracy

void setup() {
  Serial.begin(115200);                       // Initialize serial line with 115200 baudraute
  WiFi.begin(WIFI_NAME, WIFI_PASS);           // Begin WiFi connection with SSID and password typed in 'credentials.h' file
  int wifiAttempts = 0;                       // Prompt to store WiFi connection attempts
  pinMode(19, OUTPUT);                        // WiFi status LED, when ON -> WiFi is connecting
  digitalWrite(19, 1);                        // Turn WiFi status LED on
  while(WiFi.status() != WL_CONNECTED){       // Wait for connecting ESP32 to WiFi
    delay(500);
    Serial.print(".");
    wifiAttempts++;
    if(wifiAttempts == 60){                   // If WiFi connection fails after 60 attempts -> restart ESP
      esp_restart();
    }
  }
  digitalWrite(19, 0);                        // Turn off WiFi status LED
  Serial.println("WiFi connected!");
  influx.configure(INFLUX_DATABASE, INFLUX_IP);  
  influx.authorize(INFLUX_USER, INFLUX_PASS);
  Serial.print("Using HTTPS: ");
  Serial.println(influx.isSecure());

  float temp = 0;
  for (int i = 0; i != TEMP_MEAS; i++){
    temp += lm.getTemperatureInDegrees();   // Get temperature from connected LM75 sensor
    delay(100);
  }
  temp = temp/TEMP_MEAS;
  count++;
  char tags[32];
  char fields[32];
  sprintf(tags, "new_tag=Yes");
  sprintf(fields, "count=%d,temp=%0.1f",count,temp);
  bool writeSuccessful = influx.write(INFLUX_MEASUREMENT,tags,fields);
  if(!writeSuccessful){
    Serial.print("error: ");
    Serial.println(influx.getResponse());
  }

  esp_sleep_enable_timer_wakeup(DELAY_TIME_US);
  Serial.flush();
  Serial.end();
  esp_deep_sleep_start();

}

void loop() {

}