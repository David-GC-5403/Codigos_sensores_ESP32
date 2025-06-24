/*
Script para el sensor de TDS v1.0 de keystudio. Este mide el TDS en base
a la conductividad del medio, valor que depende de la temperatura del 
mismo. Por ello, se le añade el sensor de temperatura de az-delivery
para tener una lectura de la temperatura 

VERSION POR USB
*/
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>
#include "esp_sleep.h"

#define ADCpin 35 // Pin analogico para el sensor de TDS
#define Vref 3.3  // Tension de alimentacion del sensor de TDS
#define oneWireBus 4 // Pin del sensor de temperatura
#define wakePin GPIO_NUM_2 // Pin de activacion del ESP32

//Objeto oneWire de la clase OneWire
OneWire oneWire(oneWireBus);

//El objeto anterior lo volvemos un objeto (sensors) de la clase DallasTemperature
DallasTemperature sensors(&oneWire);
DeviceAddress dirSensor; // Direccion del sensor de temperatura


int i = 0;
float TempC = 0.0; // Variable para almacenar la temperatura


void findDevices(DeviceAddress direccion){
  // Busca si hay dispositivos conectados y su direccion

  //Serial.println("Buscando sensores...");
  //Serial.print("Found ");
  //Serial.print(sensors.getDeviceCount(), DEC);
  //Serial.println(" devices.");

  // Comprueba si se ha encontrado el sensor y guarda su direccion
  if (!sensors.getAddress(direccion, 0)){   // En teoria getAddress guarda la direccion en el vector que le pases
    //Serial.println("No se pudo conseguir la diereccion");
  }
}

float printTemperature(DeviceAddress deviceAddress)
{
  // Funcion para obtener la medicion de temperatura. 
  // Viene del ejemplo "single" de DallasTemperature.h
  float tempC = sensors.getTempC(deviceAddress);

  if (tempC == DEVICE_DISCONNECTED_C)
  {
    //Serial.println("Error: El sensor se ha desconectado");
    return -1;
  }

  /*
  // Medida en Celsius
  Serial.print("Temp C: ");
  Serial.print(tempC);
  Serial.println(" ");
  */

  // Medida en Farenheit
  //Serial.print(" Temp F: ");
  //Serial.println(DallasTemperature::toFahrenheit(tempC));

  return tempC;
}

void setup() {
  Serial.begin(9600);

  pinMode(ADCpin, INPUT);
  
  // Configure wake-up pin
  pinMode(wakePin, INPUT_PULLDOWN);
  esp_sleep_enable_gpio_wakeup();
  gpio_wakeup_enable(wakePin, GPIO_INTR_HIGH_LEVEL);

  // Comienza la comunicacion con el sensor de temperatura
  sensors.begin();
  findDevices(dirSensor); // Encuentra el sensor de temperatura y asigna su direccion


  // Resolucion del ADC del sensor de temp
  sensors.setResolution(dirSensor, 9); // No funciona, la resolucion siempre es 10 bit por algun motivo
  //Serial.print("Resolucion de: ");
  //Serial.print(sensors.getResolution(dirSensor), DEC);
  //Serial.println(" bits");

  //Serial.println("Configuracion finalizada");
}

void loop() {
  esp_light_sleep_start();
  delay(100);

  // Lectura del valor de temperatura
  sensors.requestTemperatures();
  TempC = printTemperature(dirSensor);

  // Lectura del valor de TDS
  float analogTDS = analogRead(ADCpin);
  float VolTDS = analogTDS * Vref/4096.0; // Tension asociada al valor analogico

  float K = 1.0 + 0.02 * (TempC - 25.0); // Factor de compensacion de temperatura
  float VolTDS_comp = VolTDS/K;


  float tdsValue=(133.42*VolTDS_comp*VolTDS_comp*VolTDS_comp - 255.86*VolTDS_comp*VolTDS_comp + 857.39*VolTDS_comp)*0.5; //convert voltage value to tds value

  Serial.print("xA;TDS;");
  Serial.flush();
  Serial.print(tdsValue, 2);
  Serial.flush();
  Serial.print(";Temp;");
  Serial.flush();
  Serial.print(TempC, 2);
  Serial.flush();
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