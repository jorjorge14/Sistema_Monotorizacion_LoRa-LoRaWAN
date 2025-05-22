#include <MKRWAN.h>   // Librería para gestionar la conectividad LoRaWAN en placas Arduino MKR

LoRaModem modem;    // Objeto para manejar el módem LoRa
const int lm35Pin = A1;  // Pin analógico donde se conecta la salida del LM35

// Credenciales para unirse a la red LoRaWAN con OTAA
String appEui = "APPEUI";   // App EUI
String appKey = "APPKEY";   // App Key

void setup() {
  Serial.begin(115200);   // Iniciar la comunicación por USB para ver mensajes de depuración
  while (!Serial);    // Esperar a que el puerto serie esté listo

  Serial.println("Iniciando prueba con LM35 y LoRaWAN");
  
  // Inicializa el módem en la banda EU868 
  if (!modem.begin(EU868)) {
    Serial.println("Error al iniciar el módulo LoRaWAN");
    while(1);   // Si hay error, detener el programa aquí
  }
  
  // Intenta unirse a la red LoRaWAN mediante OTAA con las credenciales
  if (!modem.joinOTAA(appEui, appKey)) {
    Serial.println("Error al unirse a la red. Revisa tu configuración o posición del nodo.");
    while(1);   // Si no puede unirse, se detener el programa
  }
  
  Serial.println("¡Conectado a la red LoRaWAN!");

  // Enviar mensaje inicial de prueba para confirmar funcionamiento
  modem.beginPacket();
  modem.print("Inicio LM35");
  int err = modem.endPacket(true);
  if (err > 0) {
    Serial.println("Mensaje de inicio enviado correctamente.");
  } else {
    Serial.println("Error enviando mensaje de inicio.");
  }
}

void loop() {
  
  int sensorValue = analogRead(lm35Pin);   // Lectura analógica del LM35  
  float voltage = sensorValue * (3.3 / 4095.0);   //Convertir la lectura (0-4095) a voltaje (usando 3.3V como referencia)
  float temperatureC = voltage * 100.0;   // Convertir el voltaje a temperatura en ºC (10mV = 1ºC)

  // Mostrar el valor por consola para depuración
  Serial.print("Temperatura: ");
  Serial.print(temperatureC);
  Serial.println(" °C");
  
  // Enviar la medida de temperatura a través de LoRaWAN
  modem.beginPacket();
  modem.print("Temp:");
  modem.print(temperatureC);
  modem.print("C");
  int err = modem.endPacket(true);
  if (err > 0) {
    Serial.println("Mensaje enviado correctamente.");
  } else {
    Serial.println("Error al enviar el mensaje.");
  }
  
  delay(10000);  // Espera 10 segundos entre lecturas
}


