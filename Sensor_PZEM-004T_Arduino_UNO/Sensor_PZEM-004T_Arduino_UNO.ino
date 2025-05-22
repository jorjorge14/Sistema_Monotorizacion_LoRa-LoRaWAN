#include <PZEM004Tv30.h>    // Librería para comunicarse con el sensor PZEM-004T v3.0
#include <SoftwareSerial.h>   // Librería para habilitar puertos serie por software

#define COST_PER_KWH 0.187  // Precio de energía basado en factura ejemplo: 27,24 € / 146 kWh ≈ 0,187 €/kWh

// Pines para la comunicación SoftwareSerial con el PZEM
SoftwareSerial pzemSerial(2, 3);  // Pin 2 = RX, Pin 3 = TX (en el Arduino) conectados al módulo PZEM
PZEM004Tv30 pzem(pzemSerial);     // Iniciar el objeto PZEM usando la comunicación por SoftwareSerial

void setup() {
  Serial.begin(9600);   // Iniciar el puerto serie para mostrar información por el monitor serie
  Serial.println("Sistema de medicion de consumo en enchufe");
  Serial.println("----------------------------------------");
}

void loop() {

  float potencia = pzem.power();  // Lectura de la potencia instantánea (en vatios, W)
  float energia = pzem.energy();  // Lectura de la energía acumulada (en kilovatios-hora, kWh)

  // Comprobar que las lecturas son válidas
  if (isnan(potencia) || isnan(energia)) {
    Serial.println("Error: No se detecta el PZEM o hay problema en la medición.");
  } else {
    
    float coste = energia * COST_PER_KWH; // Cálcular del coste estimado acumulado según la energía y el precio unitario

    Serial.print("Potencia: ");   // Mostrar potencia leída
    Serial.print(potencia);
    Serial.println(" W");
    
    // Detectar si hay o no un dispositivo conectado
    if (potencia>5.0){    
      Serial.println("Hay un dispositivo enchufado");
    }
    else{
      Serial.println("NO hay ningún dispositivo enchufado");
    }

    Serial.print("Energia acumulada: ");  // Mostrar energía leída
    Serial.print(energia, 5); // Mostrar 5 decimales para poder ver cambios pequeños
    Serial.println(" kWh");

    Serial.print("Coste estimado: ");   // Mostrar coste estimado
    Serial.print(coste, 5);
    Serial.println(" Euros");

    Serial.println("----------------------------------------");
  }

  // Esperar 5 segundos antes de volver a realizar la lectura
  delay(5000);
}
