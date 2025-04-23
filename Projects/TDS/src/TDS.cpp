/*
Script para el sensor de TDS v1.0 de keystudio. Este mide el TDS en base
a la conductividad del medio, valor que depende de la temperatura del 
mismo. Por ello, se le añade el sensor de temperatura de az-delivery
para tener una lectura de la temperatura 
*/
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include "certificado.h" // Certificado para la conexión segura al broker

#define ADCpin 35 // Pin analogico para el sensor de TDS
#define Vref 3.3  // Tension de alimentacion del sensor de TDS
#define oneWireBus 4 // Pin del sensor de temperatura

// Punteros para wifi y el broker
const char* ssid = "Redmi Note 13 Pro";
const char* pass = "contrasena1234";
const char* mqtt_server = "da67f6632258405fb1024df09f9f2676.s1.eu.hivemq.cloud";
const char* username = "esp32_ADQUISICION";
const char* password = "s20pCe7I";

// Define NTP Server address
const char* ntpServer = "es.pool.ntp.org";  //Modify as per your country
const long  gmtOffset_sec = 3600; // Offset from UTC (in seconds) 
const int   daylightOffset_sec = 3600; // Daylight offset (in seconds)

// Buffer para almacenar los mensajes
char buffer[255];


// Creación de objetos
WiFiClientSecure espClient;
PubSubClient client(espClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

//Objeto oneWire de la clase OneWire
OneWire oneWire(oneWireBus);

//El objeto anterior lo volvemos un objeto (sensors) de la clase DallasTemperature
DallasTemperature sensors(&oneWire);
DeviceAddress dirSensor; // Direccion del sensor de temperatura


int i = 0;
#define media 5
float vector_TDS_medio[media];

float average(float data[], int size);
void findDevices(DeviceAddress direccion);
float printTemperature(DeviceAddress deviceAddress);
void printAddress(DeviceAddress direccion);
void reconnect();

void setup() {
  Serial.begin(9600);
  Serial.print("Script de medición de TDS");
  delay(1000);

  // Inicia el wifi
  WiFi.begin(ssid,pass);
  Serial.print("\n Connecting to WiFi...");
  while(WiFi.status () != WL_CONNECTED){  // Animación mientras no se conecta
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n Connected!");
  Serial.println("IP address: "); // Cuando se conecta muestra la IP
  Serial.println(WiFi.localIP());


  espClient.setCACert(root_ca); // Configura el certificado


  // Inicia el RTC
  timeClient.begin();
  timeClient.update();

  // Se configura el broker al que conectarse
  client.setServer(mqtt_server, 8883);


  pinMode(ADCpin, INPUT);

  // Comienza la comunicacion con el sensor de temperatura
  sensors.begin();
  findDevices(dirSensor); // Encuentra el sensor de temperatura y asigna su direccion
  delay(500);


  // Resolucion del ADC del sensor de temp
  sensors.setResolution(dirSensor, 9); // No funciona, la resolucion siempre es 10 bit por algun motivo
  Serial.print("Resolucion de: ");
  Serial.print(sensors.getResolution(dirSensor), DEC);
  Serial.println(" bits");
  delay(500);

  Serial.println("Configuracion finalizada");
  delay(1000);
}

void loop() {

  timeClient.update();  // Actualiza la hora

  // Reconecta al broker
  if (!client.connected()) reconnect();

  client.loop();


  
  for (i = 0;i<media;i++){
    // Lectura del valor de temperatura
    sensors.requestTemperatures();
    
    float TempC = -1;
    while (TempC == -1){
      TempC = printTemperature(dirSensor);
    }

    // Lectura del valor de TDS
    float analogTDS = analogRead(ADCpin);
    float VolTDS = analogTDS * Vref/1024.0; // Tension asociada al valor analogico

    float K = 1.0 + 0.02 * (TempC - 25.0); // Factor de compensacion de temperatura
    float VolTDS_comp = VolTDS/K;
  

    float tdsValue=(133.42*VolTDS_comp*VolTDS_comp*VolTDS_comp - 255.86*VolTDS_comp*VolTDS_comp + 857.39*VolTDS_comp)*0.5; //convert voltage value to tds value
    vector_TDS_medio[i] = tdsValue;
  }

  float TDS_medio = average(vector_TDS_medio, media);
  snprintf(buffer, sizeof(buffer), "%.2f", TDS_medio);

  // Envío del mensaje
  Serial.println("Intentando enviar el mensaje...");
  delay(500);

  client.subscribe("Medidas/TDS");
  if (client.publish("Medidas/TDS", buffer)){
    Serial.print("TDS enviado correctamente: ");
    Serial.println(buffer);
  } else{
    Serial.println("TDS no enviado");
  }


  delay(3000);
}

float average(float data[], int size){

/* Funcion simple que hace la media

  data[] -> Vector con los datos a promediar
  size -> Tamaño del vector data[]

*/
  float sum = 0;
  for (int i = 0; i<media; i++){
    sum = sum + data[i];
  }
  return sum/float(size);
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
  printAddress(dirSensor);
  Serial.println();
}

float printTemperature(DeviceAddress deviceAddress)
{
  // Funcion para obtener la medicion de temperatura. 
  // Viene del ejemplo "single" de DallasTemperature.h
  float tempC = sensors.getTempC(deviceAddress);

  if (tempC == DEVICE_DISCONNECTED_C)
  {
    Serial.println("Error: El sensor se ha desconectado");
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

void reconnect(){
/* Funcion para realizar la reconexión con el broker si esta es necesaria
  */
  while (!client.connected()) {
  Serial.println("Intentando conectar al broker MQTT...");
  if (client.connect("ESP32", username, password)){
    Serial.println("Conexión exitosa al broker MQTT!");
    break;
  }else{
    Serial.print("Error al conectar: ");
    Serial.println(client.state());  // Muestra el error
    delay(5000);  // Espera antes de reintentar
  }
  }
}