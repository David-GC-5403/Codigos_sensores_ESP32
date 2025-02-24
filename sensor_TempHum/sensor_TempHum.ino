#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <NTPClient.h>

// Configuración del sensor
#define DHTPIN 4      // Pin donde está conectado el sensor
#define DHTTYPE DHT11 // Tipo de sensor

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
char buffer2[255];

// Creación de objetos
WiFiClientSecure espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

/*----------FUNCIONES INICIO------------*/

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

/*----------FUNCIONES FIN------------*/

void setup() {
  Serial.begin(9600);
  pinMode(DHTPIN,INPUT_PULLUP); // Se asegura de que el pin esta en modo entrada
  WiFi.begin(ssid,pass); // Inicia el wifi
  dht.begin();

  esp_sleep_enable_timer_wakeup(3000000);

  Serial.print("\n Connecting to WiFi...");
  while(WiFi.status () != WL_CONNECTED){  // Animación mientras no se conecta
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n Connected!");
  Serial.println("IP address: "); // Cuando se conecta muestra la IP
  Serial.println(WiFi.localIP());

  espClient.setCACert(root_ca);

  timeClient.begin();
  timeClient.update();

  client.setServer(mqtt_server, 8883); // Se configura el broker al que conectarse
  Serial.println("Configuracion finalizada");
}

void loop() {

  timeClient.update();

  if (!client.connected()){
    reconnect();
  }

  client.loop(); // Mantiene la conexion con el servidor
  float temperatura = dht.readTemperature(); // Celsius
  float humedad = dht.readHumidity();
  String tiempo = timeClient.getFormattedTime();

  if (isnan(temperatura) || isnan(humedad)) {
      Serial.println("Error al leer el sensor DHT11");
  } else {
      Serial.print("Temperatura: ");
      Serial.print(temperatura);
      Serial.println(" °C");

      Serial.print("Humedad: ");
      Serial.print(humedad);
      Serial.println(" %");
    }

  // Esto no imprime el valor, almacena en "buffer" la cadena de texto formateada. Es decir, en vez de imprimir una cadena de texto o lo que sea, la mete en buffer
  snprintf(buffer, sizeof(buffer), "%.2f %s", temperatura, tiempo.c_str());
  snprintf(buffer2, sizeof(buffer2), "%f %s", humedad, tiempo.c_str());

  Serial.println("Intentando enviar el mensaje...");
  delay(500);

  client.subscribe("Medidas/Temperatura");
  if (client.publish("Medidas/Temperatura", buffer)){
    Serial.print("Temperatura enviada correctamente: ");
    Serial.println(buffer);
  } else{
    Serial.println("Temperatura no enviada");
  }

  client.subscribe("Medidas/Humedad");
  if (client.publish("Medidas/Humedad", buffer2)){
    Serial.print("Humedad enviada correctamente: ");
    Serial.println(buffer2);
  } else{ delay(3000);
    Serial.println("Humedad no enviada");
  }

  esp_deep_sleep_start() ;
  // delay(3000);
}
