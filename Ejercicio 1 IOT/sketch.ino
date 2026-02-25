#define ADC_PIN 34

void setup() {
  Serial.begin(115200);
  delay(200);
}

void loop() {
  int raw = analogRead(ADC_PIN);          // 0 a 4095
  float v = (raw / 4095.0) * 3.3;         // aproximación a voltaje

  // Para el Serial Plotter es mejor mandar SOLO números (una o varias columnas)
  Serial.print(raw);
  Serial.print(" ");
  Serial.println(v);

  delay(20);
}

