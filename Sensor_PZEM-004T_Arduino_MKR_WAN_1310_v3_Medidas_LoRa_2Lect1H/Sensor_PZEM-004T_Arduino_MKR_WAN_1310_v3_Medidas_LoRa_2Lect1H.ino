#include <PZEM004Tv30.h>
#include <MKRWAN.h>
#include <ArduinoLowPower.h>  // Para bajo consumo

PZEM004Tv30* pzem;    // Se declara un puntero a un objeto PZEM004Tv30
LoRaModem modem;  // Objeto para manejar el módem LoRa
bool joined;  // Variable que indica si se ha unido exitosamente a la red LoRaWAN

const String appEui = "YOUR_APPEUI";  // App EUI y App Key
const String appKey = "YOUR_KEY";

const float precioPorKWh = 0.187;   // Precio de energía basado en factura ejemplo: 27,24 € / 146 kWh ≈ 0,187 €/kWh

float energiaAnterior = 0.0;
float energiaAcumulada = 0.0;
float costeAcumulado = 0.0;

int lecturasRealizadas = 0;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial1.begin(9600);  // Comunicación con el PZEM (pines 13 y 14)
  pzem = new PZEM004Tv30(Serial1);

  modem.begin(EU868);
  joined = modem.joinOTAA(appEui, appKey);

  if (joined) {
    modem.setPort(1);
    modem.minPollInterval(60);  // mínimo entre transmisiones (seguridad LoRa)
  }
}

void loop() {
  float energiaActual = pzem->energy();  // Energía total desde el inicio (Wh)

  if (!isnan(energiaActual)) {
    float energiaConsumida = energiaActual - energiaAnterior;
    float coste = energiaConsumida * precioPorKWh;

    energiaAnterior = energiaActual;
    energiaAcumulada += energiaConsumida;
    costeAcumulado += coste;

    // Preparar payload (energía y coste, ambos con 2 bytes)
    uint16_t energiaWh = energiaConsumida * 1000;  // Wh * 1000 = mWh
    uint16_t costeCent = coste * 100;              // € * 100 = céntimos

    byte payload[4];
    payload[0] = highByte(energiaWh);
    payload[1] = lowByte(energiaWh);
    payload[2] = highByte(costeCent);
    payload[3] = lowByte(costeCent);

    if (joined) {
      modem.beginPacket();
      modem.write(payload, 4);
      modem.endPacket(true);
    }

    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);

    lecturasRealizadas++;

    // Si han pasado 4 lecturas (2 horas), enviar resumen acumulado
    if (lecturasRealizadas >= 4) {
      uint16_t totalWh = energiaAcumulada * 1000;
      uint16_t totalCent = costeAcumulado * 100;

      byte resumen[5];
      resumen[0] = 0xFF;  // Identificador de resumen
      resumen[1] = highByte(totalWh);
      resumen[2] = lowByte(totalWh);
      resumen[3] = highByte(totalCent);
      resumen[4] = lowByte(totalCent);

      modem.beginPacket();
      modem.write(resumen, 5);
      modem.endPacket(true);

      // Reiniciar acumuladores
      energiaAcumulada = 0.0;
      costeAcumulado = 0.0;
      lecturasRealizadas = 0;
    }
  }

  // Sleep durante 30 minutos
  for (int i = 0; i < 30; i++) {
    LowPower.sleep(60000);  // 60.000 ms = 1 minuto → 30 ciclos = 30 minutos
  }
}
