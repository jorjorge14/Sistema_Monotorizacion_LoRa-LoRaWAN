function Decoder(bytes, port) {
  var mA = (bytes[0] << 8) | bytes[1];
  return {
    corriente_mA: mA + " mA"
  };
}