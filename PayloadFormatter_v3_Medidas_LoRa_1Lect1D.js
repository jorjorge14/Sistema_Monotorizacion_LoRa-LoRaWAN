function decodeUplink(input) {
  const bytes = input.bytes;

  if (bytes.length === 4) {
    const energia_kWh = ((bytes[0] << 8) | bytes[1]) / 10000;
    const coste_euros = ((bytes[2] << 8) | bytes[3]) / 10000;

    return {
      data: {
        energia_kWh: energia_kWh,
        coste_euros: coste_euros
      }
    };
  } else if (bytes.length === 5 && bytes[0] === 0xFF) {
    const energia_kWh = ((bytes[1] << 8) | bytes[2]) / 10000;
    const coste_euros = ((bytes[3] << 8) | bytes[4]) / 10000;

    return {
      data: {
        resumen_mensual: true,
        energia_kWh: energia_kWh,
        coste_euros: coste_euros
      }
    };
  } else {
    return {
      data: {
        mensaje: "Formato desconocido"
      }
    };
  }
}