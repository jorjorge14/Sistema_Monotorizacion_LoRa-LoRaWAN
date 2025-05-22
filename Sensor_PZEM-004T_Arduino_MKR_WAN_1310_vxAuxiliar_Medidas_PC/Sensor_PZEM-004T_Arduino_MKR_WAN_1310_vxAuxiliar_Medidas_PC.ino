#include <PZEM004Tv30.h>

PZEM004Tv30* pzem; //Se declara un puntero a un objeto PZEM004Tv30

void setup() {
  delay(3000); // Espera 3 segundos para asegurar que el USB esté listo y la conexión serie no se corte al iniciar
  
  Serial.begin(115200); // Inicia la comunicación por el puerto serie USB para mostrar los datos en el monitor serie
  Serial.println("Iniciando Serial1 y PZEM");

  Serial1.begin(9600); // Inicia la comunicación UART por Serial1 a 9600 baudios. En el Arduino MKR WAN 1310, Serial1 usa pines 13 (RX) y 14 (TX)
  delay(100);

  pzem = new PZEM004Tv30(Serial1); // Crea una nueva instancia del objeto PZEM004Tv30 usando la interfaz Serial1
}

void loop() {
  float current = pzem->current();
  if (isnan(current)) {
    Serial.println("Error: sin respuesta del PZEM al leer corriente");
  } else {
    Serial.print("Corriente: ");
    Serial.print(current);
    Serial.println(" A");
  }
  Serial.println("--------------------"); // Separación entre lecturas
  // Espera 4 segundos antes de hacer otra lectura
  delay(4000);
}
