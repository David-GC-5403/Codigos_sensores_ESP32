#include <Arduino.h>
#include "esp_sleep.h"

#define ADCpin 32
#define Vref 5.0
#define wakePin GPIO_NUM_2

void setup() {
  Serial.begin(9600);
  pinMode(ADCpin, INPUT);
  
  // Configure wake-up pin
  pinMode(wakePin, INPUT_PULLDOWN);
  esp_sleep_enable_gpio_wakeup();
  gpio_wakeup_enable(wakePin, GPIO_INTR_HIGH_LEVEL);
  
}

void loop() {
  // Enter light sleep
  esp_light_sleep_start();
  delay(100);

  // Take measurement
  delay(100); // ADC stabilization
  float an_value = analogRead(ADCpin);
  float V_value = (an_value / 4095.0) * Vref;
  float turb = 100.00 - (V_value / Vref) * 100.00;
  
  // Send data once
  Serial.print("xA;Turbidez;");
  Serial.print(turb, 2);
  Serial.println(";xZ");
  
  // Wait for "off" command with timeout
  unsigned long start = millis();
  while(millis() - start < 10000) {
    if(Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();
      if(cmd == "off") {
        break;
      }
    }
    delay(10);
  }

  Serial.flush(); // Ensure all data is sent
  delay(100); // Give time for transmission
  
 
}