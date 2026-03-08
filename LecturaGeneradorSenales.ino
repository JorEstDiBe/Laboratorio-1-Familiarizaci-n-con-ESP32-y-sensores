#define ADC_PIN 34
const float VREF = 3.3;

void setup() {
  Serial.begin(115200);
  delay(200);

  // Mejora el rango del ADC para señales hasta ~3.3V
  analogSetPinAttenuation(ADC_PIN, ADC_11db);
}

void loop() {
  int raw = analogRead(ADC_PIN);        // 0..4095
  float v = (raw / 4095.0) * VREF;      // voltaje aproximado

  // Para Serial Plotter: enviar solo números
  Serial.println(v, 3);

  delay(2); // muestreo rápido
}