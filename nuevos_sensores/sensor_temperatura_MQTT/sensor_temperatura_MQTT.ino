#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <NTPClient.h>

//Pin del sensor de temperatura
#define oneWireBus 4

// HiveMQ Cloud Let's Encrypt CA certificate
static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

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

// Array con la dirección del sensor. Para usar 1 solo no es muy
// importante, pero necesario al usar varios

//DeviceAddress dirSensor = {0x28, 0xFF, 0x64, 0x1F, 0x7D, 0x94, 0xC4, 0x48};
DeviceAddress dirSensor;

//------------------------------------------------------------------------------------------------------------------------------------------------//

void setup() {
  Serial.begin(9600);
  delay(500);

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

  client.setServer(mqtt_server, 8883); // Se configura el broker al que conectarse


  // Inicializa la comunicación con el sensor
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

  Serial.println("Configuracion finalizada");
}

void loop() {

  timeClient.update();  // Actualiza la hora

  // Reconecta al broker
  if (!client.connected()){
    reconnect();
  }

  client.loop();

  // Mide la temperatura 
  sensors.requestTemperatures(); // Comando para actualizar la medida de temperatura
  float TempC = printTemperature(dirSensor);
  String tiempo = timeClient.getFormattedTime(); // Hora a la que se obtuvo la medición

  // Comprueba que se haya leido la temperatura. En caso negativo vuelve a intentar medirla
  if (TempC == -1){
    return;
  }

  // Guarda la medición en "buffer" junto con la hora a la que se tomó
  snprintf(buffer, sizeof(buffer), "%.2f %s", TempC, tiempo.c_str());

  // Envío del mensaje
  Serial.println("Intentando enviar el mensaje...");
  delay(500);

  client.subscribe("Medidas/Temperatura");
  if (client.publish("Medidas/Temperatura", buffer)){
    Serial.print("Temperatura enviada correctamente: ");
    Serial.println(buffer);
  } else{
    Serial.println("Temperatura no enviada");
  }

  delay(3000);
}

//------------------------------------------------------------------------------------------------------------------------------------------------//

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

  // Medida en Celsius
  Serial.print("Temp C: ");
  Serial.print(tempC);

  // Medida en Farenheit
  Serial.print(" Temp F: ");
  Serial.println(DallasTemperature::toFahrenheit(tempC));

  return tempC;
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