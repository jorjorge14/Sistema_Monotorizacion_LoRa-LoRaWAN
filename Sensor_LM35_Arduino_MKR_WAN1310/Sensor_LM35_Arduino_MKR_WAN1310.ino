#include <MKRWAN.h>

LoRaModem modem;
const int lm35Pin = A1;  // Pin analógico donde conectamos la salida del LM35

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Iniciando prueba con LM35 y LoRaWAN");
  
  // Inicializa el módem en la banda EU868 
  if (!modem.begin(EU868)) {
    Serial.println("Error al iniciar el módulo LoRaWAN");
    while(1);
  }
  
  // Para unirse a la red se puede usamos OTAA 
  String appEui = "YOUR_APPEUI";   // App EUI
  String appKey = "YOUR_APPKEY";   // App Key

  Serial.println("Intentando unirse a la red vía OTAA...");
  if (!modem.joinOTAA(appEui, appKey)) {
    Serial.println("Error al unirse a la red. Revisa tu configuración o posición del nodo.");
    while(1);
  }
  Serial.println("¡Conectado a la red LoRaWAN!");

  // Enviar mensaje inicial de prueba
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
  // Lectura del LM35
  int sensorValue = analogRead(lm35Pin);
  // Convertir la lectura (0-1023) a voltaje (usando 3.3V como referencia)
  float voltage = sensorValue * (3.3 / 1023.0);
  // El LM35 entrega 10 mV/°C; con 3.3V de referencia, el valor en °C es:
  float temperatureC = voltage * 100;  // (voltage en V * 100 = temperatura en °C)
  
  // Mostrar el valor
  Serial.print("Temperatura: ");
  Serial.print(temperatureC);
  Serial.println(" °C");
  
  // Enviar la medida por LoRaWAN
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
  
  delay(10000);  // Espera 10 segundos entre mediciones
}

