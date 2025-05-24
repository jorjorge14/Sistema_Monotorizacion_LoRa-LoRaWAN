#include <MKRWAN.h>

LoRaModem modem;

void setup() {
  Serial.begin(115200);
  while (!Serial); // Espera a que el puerto serie esté listo (importante en placas SAMD como el MKR WAN 1310)
  
  if (!modem.begin(EU868)) {
    Serial.println("Failed to start module");
    while (1);
  }
  
  Serial.print("Firmware version: "); // Si el módem inicia correctamente, muestra la versión del firmware del módem
  Serial.println(modem.version());
  Serial.print("Your device EUI is: "); // Muestra el EUI del dispositivo
  Serial.println(modem.deviceEUI());
}
void loop() {}
