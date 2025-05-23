#include <PZEM004Tv30.h>    // Librería para comunicarse con el sensor PZEM-004T v3.0
#include <MKRWAN.h>   // Librería para gestionar la conectividad LoRaWAN en placas Arduino MKR
#include <ArduinoLowPower.h>  // Librería para gestionar el modo de bajo consumo

LoRaModem modem;    // Objeto para manejar el módem LoRa
PZEM004Tv30* pzem;    // Puntero a un objeto PZEM004Tv30
bool joined;    // Variable que indica si se ha unido exitosamente a la red LoRaWAN

// Credenciales para unirse a la red LoRaWAN con OTAA
String appEui = "APPEUI";   // App EUI
String appKey = "APPKEY";   // App Key

const float precioPorKWh = 0.187;  // Precio de energía basado en factura ejemplo: 27,24 € / 146 kWh ≈ 0,187 €/kWh

float energiaAnterior = 0.0;    // Energía acumulada en la lectura anterior (Wh)
float energiaAcumulada = 0.0;   // Energía total acumulada entre envíos (Wh)
float costeAcumulado = 0.0;   // Coste total acumulado (€)

int lecturasRealizadas = 0;   // Contador de lecturas entre envíos de resumen

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);   // Configurar el LED integrado en la placa para indicar envíos

  Serial1.begin(9600);  // Inicializar PZEM por Serial1 (pines 13 TX, 14 RX)
  pzem = new PZEM004Tv30(Serial1);    //Crear el objeto PZEM

  // Iniciar el módem en la banda EU868 y se une a la red LoRaWAN mediante OTAA con las credenciales
  modem.begin(EU868);
  joined = modem.joinOTAA(appEui, appKey);
  if (joined) {
    modem.setPort(1);   // Configurar el puerto de aplicación a 1
    modem.minPollInterval(60);  // Establecer el intervalo mínimo entre transmisiones a 60 segundos 
  }
}

void loop() {
  float energiaActual = pzem->energy();  // Obtener la energía total consumida desde el inicio (Wh)

  if (!isnan(energiaActual)) {  // Comprobar que la lectura es válida
    float energiaConsumida = energiaActual - energiaAnterior;   // Calcular la energía consumida desde la última lectura (Wh)
    float coste = energiaConsumida * precioPorKWh;    // Calcular el coste estimado acumulado según la energía y el precio unitario (€) 

    energiaAnterior = energiaActual;    // Actualizar la energía anterior para la próxima lectura
    energiaAcumulada += energiaConsumida;   // Acumular la energía consumida
    costeAcumulado += coste;    // Acumular el coste estimado

    // Preparar paquete a enviar a la red LoRaWAN 
    uint16_t energiaWh = energiaConsumida * 1000;  // Lecturas de la energía consumida en 30 min. Convertir a Wh (kWh * 1000)
    uint16_t costeCent = coste * 100;              // Lecturas del coste estimado en 30 min. Convertir a céntimos (€ * 100)

    // Paquete de 4 bytes para enviar (2 bytes para energía y 2 bytes para coste)
    byte payload[4];    
    payload[0] = highByte(energiaWh);   // Byte alto de energía
    payload[1] = lowByte(energiaWh);    // Byte bajo de energía
    payload[2] = highByte(costeCent);   // Byte alto de coste
    payload[3] = lowByte(costeCent);    // Byte bajo de coste

    // Si se ha unido a la red LoRaWAN, enviar el paquete
    if (joined) {   
      modem.beginPacket();
      modem.write(payload, 4);    
      modem.endPacket(true);
    }

    // Encender el LED integrado para indicar que se ha enviado un paquete
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);

    lecturasRealizadas++;   // Incrementar el contador de lecturas

    // Cada 2 horas (4 lecturas * 30 min = 2 h), enviar el paquete especial con el resumen
    if (lecturasRealizadas >= 4) {
      uint16_t totalWh = energiaAcumulada * 1000; // Lecturas de la energía acumulada en 2 h. Convertir a Wh (kWh * 1000)
      uint16_t totalCent = costeAcumulado * 100;    // Lecturas del coste acumulado en 2 h. Convertir a céntimos (€ * 100)

      // Paquete de 5 bytes para enviar (2 bytes para energía, 2 bytes para coste y 1 byte para identificar)
      byte resumen[5];    
      resumen[0] = 0xFF;  // Identificador del paquete resumen
      resumen[1] = highByte(totalWh);   // Byte alto de energía
      resumen[2] = lowByte(totalWh);    // Byte bajo de energía
      resumen[3] = highByte(totalCent);   // Byte alto de coste
      resumen[4] = lowByte(totalCent);    // Byte bajo de coste

      // Enviar el paquete resumen a la red LoRaWAN 
      modem.beginPacket();   
      modem.write(resumen, 5);
      modem.endPacket(true);

      // Reiniciar acumuladores
      energiaAcumulada = 0.0;
      costeAcumulado = 0.0;
      lecturasRealizadas = 0;
    }
  }

  // Modo bajo consumo: duerme 30 min en ciclos de 1 min
  for (int i = 0; i < 30; i++) {
    LowPower.sleep(60000);  // Duerme 1 minuto (=60.000 ms) 
  }
}