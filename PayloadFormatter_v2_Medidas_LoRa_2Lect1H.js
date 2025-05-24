function Decoder(bytes, port) {
  let result = {};
  
  if (bytes[0] === 0xFF) {
    // Mensaje resumen mensual (5 bytes)
    let energia_mWh = (bytes[1] << 8) | bytes[2];
    let coste_cents = (bytes[3] << 8) | bytes[4];
    
    result.type = "Resumen 2h";
    result.energia_Wh = energia_mWh / 1000;
    result.coste_eur = coste_cents / 100;
    
  } else if (bytes.length === 4) {
    // Mensaje lectura normal (4 bytes)
    let energia_mWh = (bytes[0] << 8) | bytes[1];
    let coste_cents = (bytes[2] << 8) | bytes[3];
    
    result.type = "Lectura 30min";
    result.energia_Wh = energia_mWh / 1000;
    result.coste_eur = coste_cents / 100;
  } else {
    // Payload inesperado
    result.error = "Payload inesperado";
  }
  
  return result;
}
