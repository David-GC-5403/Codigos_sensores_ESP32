//Incluir las bibliotecas OneWire y DallasTemperature
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>
#include <esp_sleep.h>

//Pin del sensor de temperatura
#define oneWireBus 4
#define wakePin GPIO_NUM_2

//Objeto oneWire de la clase OneWire
OneWire oneWire(oneWireBus);

//El objeto anterior lo volvemos un objeto (sensors) de la clase DallasTemperature
DallasTemperature sensors(&oneWire);

// Array con la dirección del sensor. Para usar 1 solo no es muy
// importante, pero necesario al usar varios
//DeviceAddress dirSensor = {0x28, 0xFF, 0x64, 0x1F, 0x7D, 0x94, 0xC4, 0x48};
DeviceAddress dirSensor;

float printTemperature(DeviceAddress deviceAdress);
void findDevices(DeviceAddress direccion);

void setup() {
  Serial.begin(9600);

  pinMode(wakePin, INPUT_PULLDOWN);
  esp_sleep_enable_gpio_wakeup();
  gpio_wakeup_enable(wakePin, GPIO_INTR_HIGH_LEVEL);

  // Inicializa la comunicación
  sensors.begin();

  // Busca los sensores conectados
  findDevices(dirSensor);

  // Resolucion del ADC
  sensors.setResolution(dirSensor, 9); // No funciona, la resolucion siempre es 12 bit por algun motivo

}

void loop() {
  esp_light_sleep_start();
  delay(100);

  //Mide la temperatura 
  float TempC;
  sensors.requestTemperatures(); // Comando para actualizar la medida de temperatura
  TempC = printTemperature(dirSensor);

  Serial.print("xA;Temperatura;");
  Serial.print(TempC, 2);
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

float printTemperature(DeviceAddress deviceAddress)
{
  // Funcion para obtener la medicion de temperatura. 
  // Viene del ejemplo "single" de DallasTemperature.h
  float tempC = sensors.getTempC(deviceAddress);

  if (tempC == DEVICE_DISCONNECTED_C)
  {
    //Serial.println("Error: El sensor se ha desconectado");
    return 1;
  }
  /*
  // Medida en Celsius
  Serial.print("Temp C: ");
  Serial.print(tempC);

  // Medida en Farenheit
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC));
  */

  return tempC;
}


void findDevices(DeviceAddress direccion){
  // Busca si hay dispositivos conectados y su direccion
  /*  Serial.println("Buscando sensores...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  delay(1000);
  */

  // Comprueba si se ha encontrado el sensor y guarda su direccion
  if (!sensors.getAddress(direccion, 0)){   // En teoria getAddress guarda la direccion en el vector que le pases
    Serial.println("No se pudo conseguir la diereccion");
  }

  /*
  Serial.print("Device 0 Address: ");
  printAddress(direccion);
  Serial.println();*/
}
