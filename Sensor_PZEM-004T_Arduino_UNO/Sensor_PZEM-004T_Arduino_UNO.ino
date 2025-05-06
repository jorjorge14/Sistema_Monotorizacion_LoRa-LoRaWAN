#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>

// Precio de energía basado en factura ejemplo: 27,24 € / 146 kWh ≈ 0,187 €/kWh
#define COST_PER_KWH 0.187  

// Pines para la comunicación SoftwareSerial con el PZEM
SoftwareSerial pzemSerial(2, 3);  // Pin 2 = RX, Pin 3 = TX
PZEM004Tv30 pzem(pzemSerial);

void setup() {
  Serial.begin(9600);
  Serial.println("Sistema de medicion de consumo en enchufe");
  Serial.println("----------------------------------------");
}

void loop() {

  float potencia = pzem.power();  // Potencia en W 
  float energia = pzem.energy();  // Energía acumulada en kWh

  // Se comprueba que las lecturas sean válidas
  if (isnan(potencia) || isnan(energia)) {
    Serial.println("Error: No se detecta el PZEM o hay problema en la medición.");
  } else {
    
    float coste = energia * COST_PER_KWH; // Calcula el coste acumulado según el precio definido

    Serial.print("Potencia: ");
    Serial.print(potencia);
    Serial.println(" W");
    if (potencia>5.0){
      Serial.println("Hay un dispositivo enchufado");
    }
    else{
      Serial.println("NO hay ningún dispositivo enchufado");
    }

    Serial.print("Energia acumulada: ");
    Serial.print(energia, 5); // Se muestran 5 decimales para poder ver cambios pequeños
    Serial.println(" kWh");

    Serial.print("Coste estimado: ");
    Serial.print(coste, 5);
    Serial.println(" Euros");

    Serial.println("----------------------------------------");
  }

  // Espera 5 segundos antes de volver a leer
  delay(5000);
}
