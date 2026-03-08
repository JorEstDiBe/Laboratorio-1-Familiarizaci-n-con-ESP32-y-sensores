#include <math.h>

#define ADC_PIN 34
#define DAC_PIN 25   // DAC real del ESP32: 25 o 26

void setup() {
  Serial.begin(115200);
  delay(200);
}

void loop() {
  int raw = analogRead(ADC_PIN);          // 0 a 4095
  float v = (raw / 4095.0) * 3.3;         // aproximación a voltaje

  Serial.print(raw);
  Serial.print(" ");
  Serial.println(v);

  // ===== Generador de señal en DAC =====

  static int i = 0;
  const int SAMPLES = 64;                // resolución de la onda
  float rad = (2.0 * PI * i) / SAMPLES;  // fase 0..2pi

  int dacVal = (int)((sin(rad) + 1.0) * 127.5); // 0..255
  dacWrite(DAC_PIN, dacVal);

  i++;
  if (i >= SAMPLES) i = 0;

  delay(20); // (igual que tu código)
}
