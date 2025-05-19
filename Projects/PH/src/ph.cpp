// Codigo para el sensor de PH de DFRobot
#include <Arduino.h>
#include "esp_sleep.h"
#include <DFRobot_PH.h>

#define wakePin GPIO_NUM_2
#define adcPin 32 // Pin para el ph
#define Vref 5

DFRobot_PH ph;


void setup() {
  Serial.begin(9600);

  // Configuracion sleep
  pinMode(wakePin, INPUT_PULLDOWN);
  esp_sleep_enable_gpio_wakeup();
  gpio_wakeup_enable(wakePin, GPIO_INTR_HIGH_LEVEL);

  ph.begin(); // Inicia el sensor de ph
  pinMode(adcPin, INPUT);

}

void loop() {
  esp_light_sleep_start();
  delay(100);

  // Mide el PH
  float analog = analogRead(adcPin);
  float tension = analog * Vref/4095.0;
  float ph_value = 3.5+tension;

  // Envia tanto el PH como la temperatura
  Serial.print("xA;PH;");
  Serial.flush();
  Serial.print(ph_value, 2);
  Serial.flush();
  Serial.println(";xZ");
  Serial.flush();

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
