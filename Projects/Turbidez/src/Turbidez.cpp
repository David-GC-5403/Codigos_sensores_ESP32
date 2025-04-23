/*
Codigo para realizar la medicion de la turbidez del agua
*/
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include "certificado.h" // Certificado para la conexión segura al broker
#include <Arduino.h>

#define ADCpin 32
#define Vref 5 // La tension de alimentacion debe ser 5 V

// Punteros para wifi y el broker
const char* ssid = "Redmi Note 13 Pro";
const char* pass = "contrasena1234";
const char* mqtt_server = "da67f6632258405fb1024df09f9f2676.s1.eu.hivemq.cloud";
const char* username = "esp32_ADQUISICION";
const char* password = "s20pCe7I";

// Buffer para almacenar los mensajes
char buffer[255];

// Define NTP Server address
const char* ntpServer = "es.pool.ntp.org";  //Modify as per your country
const long  gmtOffset_sec = 3600; // Offset from UTC (in seconds) 
const int   daylightOffset_sec = 3600; // Daylight offset (in seconds)

// Creación de objetos
WiFiClientSecure espClient;
PubSubClient client(espClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

void reconnect();


void setup() {
  Serial.begin(9600);
  Serial.println("Script de medicion de turbidez");
  pinMode(ADCpin, INPUT);
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

  Serial.println("------------------------");
  Serial.println("Configuracion finalizada");
}

void loop() {

  timeClient.update();  // Actualiza la hora

  // Reconecta al broker
  if (!client.connected()) reconnect();
  client.loop();


  // Lee el valor analogico
  float an_value = analogRead(ADCpin);
  /*Serial.print("Valor analogico = ");
  Serial.println(an_value);
  delay(500);*/

  float V_value = an_value * Vref/4095.0; // Tension asociada al valor analogico
  /*Serial.print("Tension = ");
  Serial.println(V_value);
  delay(500);*/

  float turb =  100.00 - (V_value / Vref) * 100.00; // Regresion lineal segun DFRobot. Mayor turbidez = menor tension
  /*Serial.print("Turbidez = ");
  Serial.println(turb);
  delay(1000);
  Serial.println("-------------------------------------------------");
  */

  snprintf(buffer, sizeof(buffer), "%.2f", turb);

  // Envío del mensaje
  Serial.println("Intentando enviar el mensaje...");
  delay(500);

  client.subscribe("Medidas/Turbidez");
  if (client.publish("Medidas/Turbidez", buffer)){
    Serial.print("Turbidez enviada correctamente: ");
    Serial.println(buffer);
  } else{
    Serial.println("Turbidez no enviada");
  }

  delay(3000);

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