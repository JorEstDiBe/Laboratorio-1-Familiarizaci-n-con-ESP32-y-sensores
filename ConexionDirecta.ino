#define ADC_PIN 34

void setup() {
  Serial.begin(115200);
  delay(200);
  analogSetPinAttenuation(ADC_PIN, ADC_11db); // mejor rango ~0 a 3.3V
}

void loop() {
  int raw = analogRead(ADC_PIN);
  float v = (raw / 4095.0) * 3.3;

  Serial.print(raw);
  Serial.print(" ");
  Serial.println(v);

  delay(20);
}