//Incluir las bibliotecas OneWire y DallasTemperature
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>

//Pin del sensor de temperatura
#define oneWireBus 4

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
void printAdress(DeviceAddress direccion);

void setup() {
  Serial.begin(9600);

  delay(1000);

  // Inicializa la comunicación
  sensors.begin();

  // Busca los sensores conectados
  findDevices(dirSensor);
  delay(1000);

  // Resolucion del ADC
  sensors.setResolution(dirSensor, 9); // No funciona, la resolucion siempre es 12 bit por algun motivo
  Serial.print("Resolucion de: ");
  Serial.print(sensors.getResolution(dirSensor), DEC);
  Serial.println(" bits");
  delay(1000);


}

void loop() {
  //Mide la temperatura 
  float TempC;
  sensors.requestTemperatures(); // Comando para actualizar la medida de temperatura
  TempC = printTemperature(dirSensor);

  delay(1000);
}

float printTemperature(DeviceAddress deviceAddress)
{
  // Funcion para obtener la medicion de temperatura. 
  // Viene del ejemplo "single" de DallasTemperature.h
  float tempC = sensors.getTempC(deviceAddress);

  if (tempC == DEVICE_DISCONNECTED_C)
  {
    Serial.println("Error: El sensor se ha desconectado");
    return 1;
  }

  // Medida en Celsius
  Serial.print("Temp C: ");
  Serial.print(tempC);

  // Medida en Farenheit
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC));

  return tempC;
}

void printAddress(DeviceAddress direccion)
// Funcion para imprimir la direccion del sensor. 
// Viene del ejemplo "single" de DallasTemperature.h
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (direccion[i] < 16) Serial.print("0");
    Serial.print(direccion[i], HEX);
  }
}

void findDevices(DeviceAddress direccion){
  // Busca si hay dispositivos conectados y su direccion

  Serial.println("Buscando sensores...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  delay(1000);

  // Comprueba si se ha encontrado el sensor y guarda su direccion
  if (!sensors.getAddress(direccion, 0)){   // En teoria getAddress guarda la direccion en el vector que le pases
    Serial.println("No se pudo conseguir la diereccion");
  }

  Serial.print("Device 0 Address: ");
  printAddress(direccion);
  Serial.println();
}
