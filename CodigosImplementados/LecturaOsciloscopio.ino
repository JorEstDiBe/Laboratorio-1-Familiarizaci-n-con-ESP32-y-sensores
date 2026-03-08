#include <math.h>

#define DAC_PIN 25
#define POT_PIN 34
#define BTN_PIN 27

enum WaveType { SQUARE, SINE };
enum PotMode  { POT_FREQ, POT_AMP };

WaveType wave = SQUARE;
PotMode  potMode = POT_FREQ;

float freqHz = 5.0f;     // 1..200 Hz aprox
float amp01  = 1.0f;     // 0..1 (amplitud)

const int SINE_SAMPLES = 256;
int sineIndex = 0;

unsigned long lastSampleUs = 0;
unsigned long samplePeriodUs = 200;

bool lastBtn = true;
unsigned long btnDownMs = 0;
unsigned long lastDebounceMs = 0;

bool squareHigh = false;
unsigned long lastToggleUs = 0;

void updateTiming() {
  samplePeriodUs = (unsigned long)(1000000.0f / (freqHz * SINE_SAMPLES));
  if (samplePeriodUs < 20) samplePeriodUs = 20;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(BTN_PIN, INPUT_PULLUP);
  analogSetPinAttenuation(POT_PIN, ADC_11db);

  updateTiming();
}

void handleButton() {
  bool reading = digitalRead(BTN_PIN);

  if (reading != lastBtn) {
    lastDebounceMs = millis();
    lastBtn = reading;
  }

  if (millis() - lastDebounceMs > 30) {
    if (!reading && btnDownMs == 0) btnDownMs = millis();

    if (reading && btnDownMs != 0) {
      unsigned long held = millis() - btnDownMs;
      btnDownMs = 0;

      if (held >= 1000) potMode = (potMode == POT_FREQ) ? POT_AMP : POT_FREQ;
      else wave = (wave == SQUARE) ? SINE : SQUARE;
    }
  }
}

void updateFromPot() {
  int raw = analogRead(POT_PIN); // 0..4095
  float x = raw / 4095.0f;

  if (potMode == POT_FREQ) {
    float f = 1.0f + x * 199.0f;      // 1..200 Hz
    freqHz = 0.85f * freqHz + 0.15f * f;
    updateTiming();
  } else {
    // Amplitud 0..1 (pero le damos un mínimo pequeño para que nunca sea 0 exacto si quieres)
    float a = x; // 0..1
    amp01 = 0.85f * amp01 + 0.15f * a;
  }
}

void generateSquare() {
  unsigned long halfPeriodUs = (unsigned long)(1000000.0f / (2.0f * freqHz));
  unsigned long nowUs = micros();

  if (nowUs - lastToggleUs >= halfPeriodUs) {
    lastToggleUs += halfPeriodUs;
    squareHigh = !squareHigh;

    // Cuadrada 0..(amp) escalada
    int high = (int)(255.0f * amp01);
    if (high < 0) high = 0;
    if (high > 255) high = 255;

    dacWrite(DAC_PIN, squareHigh ? high : 0);
  }
}

void generateSine() {
  unsigned long nowUs = micros();
  if (nowUs - lastSampleUs >= samplePeriodUs) {
    lastSampleUs += samplePeriodUs;

    float rad = (2.0f * PI * sineIndex) / SINE_SAMPLES;
    float s01 = (sinf(rad) + 1.0f) * 0.5f; // 0..1

    // Aquí la amplitud es MUY evidente:
    // 0.. (255*amp01)
    int out = (int)(s01 * (255.0f * amp01));
    if (out < 0) out = 0;
    if (out > 255) out = 255;

    dacWrite(DAC_PIN, out);

    sineIndex++;
    if (sineIndex >= SINE_SAMPLES) sineIndex = 0;
  }
}

void loop() {
  handleButton();
  updateFromPot();

  if (wave == SQUARE) generateSquare();
  else generateSine();
}