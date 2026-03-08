#include <math.h>

// ===== Pines =====
#define DAC_PIN 25
#define ADC_PIN 34
#define POT_PIN 35
#define BTN_PIN 27

// ===== Constantes de escala =====
const float VREF = 3.3f;

// ===== Modos =====
enum WaveType { W_SINE, W_SQUARE };
enum PotMode  { P_FREQ, P_AMP };

WaveType wave = W_SINE;
PotMode  potMode = P_FREQ;

// ===== Parámetros =====
float freqHz = 10.0f;   // 1..200 Hz aprox
float amp01  = 1.0f;    // 0..1

// ===== Seno =====
const int SAMPLES = 256;
int idx = 0;
unsigned long lastSineUs = 0;
unsigned long samplePeriodUs = 200;

// ===== Cuadrada =====
bool sqHigh = false;
unsigned long lastToggleUs = 0;

// ===== Botón =====
bool lastBtnReading = true;
unsigned long lastChangeMs = 0;
unsigned long pressedAtMs = 0;

// ===== Utilidades =====
float clampf(float x, float lo, float hi) {
  if (x < lo) return lo;
  if (x > hi) return hi;
  return x;
}

float lowpass(float cur, float target, float alpha) {
  return cur * (1.0f - alpha) + target * alpha;
}

void updateTiming() {
  samplePeriodUs = (unsigned long)(1000000.0f / (freqHz * SAMPLES));
  if (samplePeriodUs < 20) samplePeriodUs = 20;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(BTN_PIN, INPUT_PULLUP);

  // ADC config (mejor rango aprox 0..3.3V)
  analogSetPinAttenuation(ADC_PIN, ADC_11db);
  analogSetPinAttenuation(POT_PIN, ADC_11db);

  updateTiming();
}

void handleButton() {
  bool reading = digitalRead(BTN_PIN); // true=suelt0, false=presionado

  // debounce
  if (reading != lastBtnReading) {
    lastChangeMs = millis();
    lastBtnReading = reading;
  }

  if (millis() - lastChangeMs > 30) {
    if (!reading && pressedAtMs == 0) {
      pressedAtMs = millis();
    }

    if (reading && pressedAtMs != 0) {
      unsigned long held = millis() - pressedAtMs;
      pressedAtMs = 0;

      if (held >= 1000) {
        potMode = (potMode == P_FREQ) ? P_AMP : P_FREQ;
      } else {
        wave = (wave == W_SINE) ? W_SQUARE : W_SINE;
      }
    }
  }
}

void updateFromPot() {
  int raw = analogRead(POT_PIN);       // 0..4095
  float x = raw / 4095.0f;             // 0..1

  if (potMode == P_FREQ) {
    float targetF = 1.0f + x * 199.0f; // 1..200 Hz
    freqHz = lowpass(freqHz, targetF, 0.15f);
    updateTiming();
  } else {
    float targetA = x;                 // 0..1
    amp01 = lowpass(amp01, targetA, 0.15f);
  }
}

// Genera DAC y devuelve el voltaje aproximado (clamp 0..3.3)
float generateDAC_V() {
  float dacV = 0.0f;

  if (wave == W_SINE) {
    unsigned long now = micros();
    if (now - lastSineUs >= samplePeriodUs) {
      lastSineUs += samplePeriodUs;

      float rad = (2.0f * PI * idx) / SAMPLES;
      float s01 = (sinf(rad) + 1.0f) * 0.5f;     // 0..1

      int out = (int)(s01 * (255.0f * amp01));   // 0..255*amp
      if (out < 0) out = 0;
      if (out > 255) out = 255;

      dacWrite(DAC_PIN, out);

      dacV = (out / 255.0f) * VREF;

      idx++;
      if (idx >= SAMPLES) idx = 0;
    }
  } else { // cuadrada
    unsigned long halfPeriodUs = (unsigned long)(1000000.0f / (2.0f * freqHz));
    unsigned long now = micros();
    if (now - lastToggleUs >= halfPeriodUs) {
      lastToggleUs += halfPeriodUs;
      sqHigh = !sqHigh;

      int high = (int)(255.0f * amp01);
      if (high < 0) high = 0;
      if (high > 255) high = 255;

      int out = sqHigh ? high : 0;
      dacWrite(DAC_PIN, out);

      dacV = (out / 255.0f) * VREF;
    }
  }

  return clampf(dacV, 0.0f, VREF);
}

// Lee ADC y devuelve voltaje (clamp 0..3.3)
float readADC_V() {
  int raw = analogRead(ADC_PIN);     // 0..4095
  float v = (raw / 4095.0f) * VREF;  // 0..3.3 aprox
  return clampf(v, 0.0f, VREF);
}

void loop() {
  handleButton();
  updateFromPot();

  float dacV = generateDAC_V();
  float adcV = readADC_V();

  // Columna 1 = DAC_V (salida local)
  // Columna 2 = ADC_V (entrada desde el otro equipo)
  Serial.print(dacV, 3);
  Serial.print(" ");
  Serial.println(adcV, 3);

  // "Etiquetas" sin romper el plotter: imprímelas MUY de vez en cuando
  // (Si ves que te daña el plotter, comenta este bloque y usa Serial Monitor aparte)
  static unsigned long lastInfoMs = 0;
  if (millis() - lastInfoMs > 2000) {
    lastInfoMs = millis();
    // Para no romper el plotter, esta info es mejor verla en Serial Monitor, no Plotter.
    // Si tu IDE mezcla ambos, comenta estas líneas.
    // Serial.println("INFO: Plotter => Col1=DAC_V (salida), Col2=ADC_V (entrada).");
  }

  delay(5);
}