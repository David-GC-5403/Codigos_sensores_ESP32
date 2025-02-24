#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <NTPClient.h>

#define SENSOR 35 //Pin del ADC

// Certificado
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
const char* pass = "contrasena123";
const char* mqtt_server = "da67f6632258405fb1024df09f9f2676.s1.eu.hivemq.cloud";
const char* username = "esp32_ADQUISICION";
const char* password = "s20pCe7I";

// Servidor NTP
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

/*----------FUNCIONES INICIO------------*/

float tension_adc(int numero_pin){
/* Funcion para leer la tensión en un pin GPIO con ADC.
  int numero pin -> Pin del que queremos leer la tensión
  return -> Tensión leida
  */
  bool pin_correcto = false;
  int i = 0;
  int pines_posibles[] = {12, 13, 14, 27, 26, 25, 33, 32, 35, 34, 39, 36};
  for (i;i<12;i++){
    if (numero_pin == pines_posibles[i]){ 
      pin_correcto = true;
    }
  }
  if (pin_correcto == false){
    Serial.println("El pin no es correcto, por favor elige otro");
  } else{
      uint16_t lectura = analogRead(numero_pin);
      float tension = (lectura * 3.3)/4095.0;
      return tension;
  }
}

float V2def(float V_galga, float Vcc, int G){
/* Funcion para calcular la deformacion en funcion de la tensión de salida de la galga
  float V_galga -> Tensión de salida de la galga
  float Vcc -> Tensión de alimentación
  int G -> Constante de la galga ¿?
  */
  return V_galga / (G*Vcc);
}

void reconnect(){
/* Funcion para realizar la reconexión con el broker si esta es necesaria
  */
  while (!client.connected()) {
  Serial.println("Intentando conectar al broker MQTT...");
  if (client.connect("ESP32", username, password)){
    Serial.println("Conexión exitosa al broker MQTT!");
    client.subscribe("Medidas/Deformación");  // Suscríbete al tópico después de conectar
    break;
  }else{
    Serial.print("Error al conectar: ");
    Serial.println(client.state());  // Muestra el error
    delay(5000);  // Espera anteDeformacións de reintentar
  }
  }
}

/*----------FUNCIONES FIN------------*/

void setup() {

  Serial.begin(9600);
  pinMode(SENSOR,INPUT); // Se asegura de que el pin esta en modo entrada
  WiFi.begin(ssid,pass); // Inicia el wifi

  Serial.print("\n Connecting to WiFi...");
  while(WiFi.status () != WL_CONNECTED){  // Animación mientras no se conecta
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n Connected!");
  Serial.println("IP address: "); // Cuando se conecta muestra la IP
  Serial.println(WiFi.localIP());

  espClient.setCACert(root_ca);

  client.setServer(mqtt_server, 8883); // Se configura el broker al que conectarse
  Serial.println("Configuracion finalizada");

  timeClient.begin();
  timeClient.update();
}

void loop() {

  timeClient.update();

  if (!client.connected()){
    reconnect();
  }

  client.loop(); // Mantiene la conexion con el servidor

  float tension_medida = tension_adc(SENSOR);
  float ph = V2def(tension_medida, 5, 3.16);
  String tiempo = timeClient.getFormattedTime();

  Serial.print("Deformación de: ");
  Serial.println(ph, 6);
  //delay(500);

  snprintf(buffer, sizeof(buffer), "%.6f %s", ph, tiempo.c_str());

  Serial.println("Intentando enviar el mensaje...");
  delay(500);

  if (client.publish("Medidas/PH", buffer)){
    Serial.print("Mensaje enviado correctamente: ");
    Serial.println(buffer);
  }else{
    Serial.println("Mensaje no enviado");
  }
  delay(3000); // Espera a que el mensaje sea recibido antes de medir de nuevo
}
