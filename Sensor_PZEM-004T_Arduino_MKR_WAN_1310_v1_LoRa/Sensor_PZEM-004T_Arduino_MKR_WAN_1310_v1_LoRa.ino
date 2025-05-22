#include <PZEM004Tv30.h>  // Librería para comunicarse con el sensor PZEM-004T v3.0
#include <MKRWAN.h>   // Librería para gestionar la conectividad LoRaWAN en placas Arduino MKR

LoRaModem modem;    // Objeto para manejar el módem LoRa
PZEM004Tv30* pzem;    // Puntero a un objeto PZEM004Tv30
bool joined;    // Variable que indica si se ha unido exitosamente a la red LoRaWAN

// Credenciales ara unirse a la red LoRaWAN con OTAA
String appEui = "APPEUI";   // App EUI
String appKey = "APPKEY";   // App Key

void setup() {
  Serial.begin(115200);   // Inicia la comunicación por USB para ver mensajes de depuración
  while (!Serial);    // Espera a que el puerto serie esté listo 

  Serial1.begin(9600);    // Inicia la comunicación UART por Serial1 a 9600 baudios. En el Arduino MKR WAN 1310, Serial1 usa pines 13 (RX) y 14 (TX)
  pzem = new PZEM004Tv30(Serial1);    // Crea el objeto PZEM

  Serial.println("Iniciando LoRa...");    // Enviar mensaje inicial de prueba para confirmar funcionamiento
  joined = modem.begin(EU868) && modem.joinOTAA(appEui, appKey);    // Intenta iniciar el módem en la banda EU868 y unirse a la red LoRaWAN mediante OTAA
  Serial.print("Estado LoRa: ");
  Serial.println(joined ? "Conectado" : "Falló conexión");

  if (joined) {    // Si se ha unido a la red, configura el puerto y el intervalo mínimo de transmisión
    modem.setPort(1);   // Usa el puerto de aplicación 1  
    modem.minPollInterval(60);    // Establece un intervalo mínimo de 60 segundos entre envío de paquetes
  }
}

void loop() {
  float instensidad = pzem->current();    // Lectura de la corriente en amperios medida

// Mostrar el valor por consola para depuración
  Serial.print("Corriente: "); 
  Serial.print(isnan(instensidad) ? 0 : instensidad * 1000, 0);   // Si la lectura es NaN, muestra 0, de lo contrario multiplica por 1000 para convertir a miliamperios
  Serial.println(" mA");

  // Se comprueba que la lectura sea válida y se envia el paquete a través de LoRaWAN
  if (joined && !isnan(instensidad)) {
    uint16_t mA = instensidad * 1000;   
    byte pl[2] = { highByte(mA), lowByte(mA) };   // Se utilizan 2 bytes por paquete para enviar la lectura de corriente (mA) a través de LoRaWAN
    
    // Envía los datos a través de LoRaWAN
    modem.beginPacket(); 
    modem.write(pl, 2); 
    modem.endPacket(true);
    
    Serial.println("Envío LoRa OK");  // Confirmación de envío
  }
  // Espera 60 segundos entre lecturas y envíos
  delay(60000);
}