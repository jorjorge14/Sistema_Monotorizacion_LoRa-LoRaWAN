#include <PZEM004Tv30.h>
#include <MKRWAN.h>
#include <ArduinoLowPower.h>  // Para modo sleep de bajo consumo

PZEM004Tv30* pzem;
LoRaModem modem;
bool joined;

const String appEui = "APPEUI", appKey = "APPKEY";    // App EUI y App Key

const float precioPorKWh = 0.187;  // €/kWh

// Variables de estado
float energiaAnterior = 0.0;
float energiaAcumuladaMes = 0.0;
float costeAcumuladoMes = 0.0;
int diasMedidos = 0;

void setup() {

  // Inicializar PZEM por Serial1 (pines 13 TX, 14 RX)
  Serial1.begin(9600);
  pzem = new PZEM004Tv30(Serial1);

  // Inicializar LoRaWAN
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
  float energiaActual = pzem->energy();  // Wh desde el inicio
  if (!isnan(energiaActual)) {
    float deltaWh = energiaActual - energiaAnterior;
    energiaAnterior = energiaActual;

    // Acumular para resumen mensual
    energiaAcumuladaMes += deltaWh;
    float costeDiario = deltaWh * precioPorKWh;
    costeAcumuladoMes += costeDiario;

    // Convertimos Wh → kWh, y mandamos kWh * 10.000 para mantener 4 decimales
    float energiaKWh = deltaWh / 1000.0;
    float costeEuros = energiaKWh * precioPorKWh;

    uint16_t energia_10e4 = uint16_t(energiaKWh * 10000);
    uint16_t coste_10e4 = uint16_t(costeEuros * 10000);

    byte payload[4] = {
      highByte(energia_10e4), lowByte(energia_10e4),
      highByte(coste_10e4), lowByte(coste_10e4)
    };


    if (joined) {
      modem.beginPacket();
      modem.write(payload, 4);
      modem.endPacket(true);
    }

    diasMedidos++;
  }

  // 2) Si ya se han medido 30 días, enviar resumen mensual
  if (diasMedidos >= 30) {
    float totalKWh = energiaAcumuladaMes / 1000.0;
    float totalEuros = totalKWh * precioPorKWh;

    uint16_t energiaTotal_10e4 = uint16_t(totalKWh * 10000);
    uint16_t costeTotal_10e4 = uint16_t(totalEuros * 10000);


    // Payload de resumen (5 bytes: 0xFF + 2B energía + 2B coste)
    byte resumen[5] = {
      0xFF,
      highByte(energiaTotal_10e4), lowByte(energiaTotal_10e4),
      highByte(costeTotal_10e4), lowByte(costeTotal_10e4)
    };

    modem.beginPacket();
    modem.write(resumen, 5);
    modem.endPacket(true);

    // Reset acumulados
    energiaAcumuladaMes = 0.0;
    costeAcumuladoMes   = 0.0;
    diasMedidos = 0;
  }

  // 3) Dormir 24h con bajo consumo (24 x 1 hora)
  for (int i = 0; i < 24; i++) {
    LowPower.sleep(3600000);  // 1hora = 3 600 000 ms
  }
}
