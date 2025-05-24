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

float energiaAnterior = 0.0;   // Energía acumulada en la lectura anterior (Wh)
float energiaAcumuladaMes = 0.0;    // Energía total acumulada entre envíos (Wh)
float costeAcumuladoMes = 0.0;    // Coste total acumulado al mes(€)

int diasMedidos = 0;  // Contador de días medidos entre envíos de resumen mensual

void setup() {

  Serial1.begin(9600);    // Inicializar PZEM por Serial1 (pines 13 TX, 14 RX)
  pzem = new PZEM004Tv30(Serial1);   // Crear el objeto PZEM

  // Inicializar el módem en la banda EU868 y unirse a la red LoRaWAN mediante OTAA con las credenciales
  modem.begin(EU868);
  joined = modem.joinOTAA(appEui, appKey);
  if (joined) {
    modem.setPort(1);
    modem.minPollInterval(60);
  }

  // Tomar referencia inicial de energía
  energiaAnterior = pzem->energy();
}

void loop() {
  // 1) Medición diaria
  float energiaActual = pzem->energy();  // Leer energía total acumulada desde el inicio (Wh)
  if (!isnan(energiaActual)) {  // Comprobar que la lectura es válida
    float deltaWh = energiaActual - energiaAnterior;  // Calcular la energía consumida desde la última lectura (Wh)
    energiaAnterior = energiaActual;    // Actualizar la energía anterior para la próxima lectura

    // Acumular energía y coste para el resumen mensual
    energiaAcumuladaMes += deltaWh;
    float costeDiario = deltaWh * precioPorKWh;
    costeAcumuladoMes += costeDiario;

    uint16_t energia_10e4 = uint16_t((deltaWh / 1000.0) * precioPorKWh * 10000 / precioPorKWh);   // Energía en kWh con 4 decimales (formato 10e4)
    uint16_t coste_10e4 = uint16_t((deltaWh / 1000.0) * precioPorKWh * 10000);     // Coste en € con 4 decimales (formato 10e4)

    // Paquete de 4 bytes para enviar (2 bytes para energía y 2 bytes para coste) 
    byte payload[4] = {   
      highByte(energia_10e4), lowByte(energia_10e4),
      highByte(coste_10e4), lowByte(coste_10e4)
    };

    // Si se ha unido a la red LoRaWAN, enviar el paquete
    if (joined) {
      modem.beginPacket();
      modem.write(payload, 4);
      modem.endPacket(true);
    }
    diasMedidos++;    // Incrementar contador de lecturas diarias
  }

  // 2) Medición mensual  (30 lecturas * 1 día = 30 días), enviar el paquete especial con el resumen mensual
  if (diasMedidos >= 30) {
    uint16_t energiaTotal_10e4 = uint16_t((energiaAcumuladaMes / 1000.0) * 10000);    // Energía en kWh con 4 decimales (formato 10e4)
    uint16_t costeTotal_10e4 = uint16_t((energiaAcumuladaMes / 1000.0) * precioPorKWh * 10000);   // Coste en € con 4 decimales (formato 10e4)

    // Paquete de 5 bytes para enviar (2 bytes para energía, 2 bytes para coste y 1 byte para identificar)
    byte resumen[5] = {
      0xFF,
      highByte(energiaTotal_10e4), lowByte(energiaTotal_10e4),
      highByte(costeTotal_10e4), lowByte(costeTotal_10e4)
    };

    // Enviar el paquete resumen a la red LoRaWAN 
    modem.beginPacket();
    modem.write(resumen, 5);
    modem.endPacket(true);

    // Reiniciar acumuladores
    energiaAcumuladaMes = 0.0;
    costeAcumuladoMes   = 0.0;
    diasMedidos = 0;
  }

  // Modo bajo consumo: duerme 24 horas en ciclos de 1 hora
  for (int i = 0; i < 24; i++) {
    LowPower.sleep(3600000);  // Duerme 1 hora (=3.600.000 ms)
  }
}