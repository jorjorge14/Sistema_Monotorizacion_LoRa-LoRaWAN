#include <MKRWAN.h>

LoRaModem modem;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  if (!modem.begin(EU868)) {
    Serial.println("Failed to start module");
    while (1);
  }
  
  Serial.print("Firmware version: ");
  Serial.println(modem.version());
}

void loop() {
}
