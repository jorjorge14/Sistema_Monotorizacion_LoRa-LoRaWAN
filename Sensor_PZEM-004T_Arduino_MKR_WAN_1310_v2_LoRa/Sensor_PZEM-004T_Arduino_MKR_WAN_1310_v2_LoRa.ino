#include <PZEM004Tv30.h>
#include <MKRWAN.h>

PZEM004Tv30* pzem;    // Se declara un puntero a un objeto PZEM004Tv30
LoRaModem modem;    // Objeto para manejar el módem LoRa
bool joined;    // Variable que indica si se ha unido exitosamente a la red LoRaWAN
const String appEui = "0000000000000000", appKey = "BBD5B696A423BFB8D92E45270E58F5E9";    // App EUI y App Key

void setup() {
  delay(3000);    // Tiempo para que el IDE abra el puerto serie
  Serial.begin(115200);   // Inicia la comunicación por USB para ver mensajes de depuración
  while (!Serial);    // Espera a que el puerto serie esté listo

  Serial1.begin(9600);    // Inicia la comunicación UART por Serial1 a 9600 baudios. En el Arduino MKR WAN 1310, Serial1 usa pines 13 (RX) y 14 (TX)
  pzem = new PZEM004Tv30(Serial1);    // Crea el objeto PZEM

  Serial.println("Iniciando LoRa...");    // Mensaje de inicio LoRa
  joined = modem.begin(EU868) && modem.joinOTAA(appEui, appKey);    // Intenta iniciar el módem en la banda EU868 y unirse a la red LoRaWAN mediante OTAA
  Serial.print("Estado LoRa: ");
  Serial.println(joined ? "Conectado" : "Falló conexión");

  if (joined) {    // Si se ha unido a la red, configura el puerto y el intervalo mínimo de transmisión
    modem.setPort(1);   // Usa el puerto de aplicación 1  
    modem.minPollInterval(60);    // Establece un intervalo mínimo de 60 segundos entre envíos
  }
}

void loop() {
  float instensidad = pzem->current();    // Lee la corriente en amperios desde el sensor
  Serial.print("Corriente: "); Serial.print(isnan(instensidad) ? 0 : instensidad * 1000, 0); Serial.println(" mA");

  if (joined && !isnan(instensidad)) {    // Si el dispositivo está conectado a LoRa y la lectura fue válida
    uint16_t mA = instensidad * 1000;   // Convierte la corriente a miliamperios (entero sin decimales)
    byte pl[2] = { highByte(mA), lowByte(mA) };   // Prepara un array de 2 bytes con la corriente codificada (big endian)
    
    // Envía los datos por LoRa
    modem.beginPacket(); 
    modem.write(pl, 2); 
    modem.endPacket(true);
    
    Serial.println("Envío LoRa OK");  // Confirmación de envío
  }
  // Espera 60 segundos antes de hacer una nueva lectura y envío
  delay(60000);
}