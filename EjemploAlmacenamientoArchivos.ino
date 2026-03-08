#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// ======= microSD (SPI) =======
const int CS_PIN = 5; 

// ======= MPU6050 (I2C) =======
#define MPU_ADDR     0x68
#define SDA_PIN      21
#define SCL_PIN      22
#define PWR_MGMT_1   0x6B
#define TEMP_OUT_H   0x41

static int16_t read16(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, (uint8_t)2, (uint8_t)true);
  int16_t hi = Wire.read();
  int16_t lo = Wire.read();
  return (int16_t)((hi << 8) | lo);
}

static void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission(true);
}

static float readTempC() {
  // Fórmula típica MPU6050: Temp(°C) = raw/340 + 36.53
  int16_t raw = read16(TEMP_OUT_H);
  return (raw / 340.0f) + 36.53f;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  // Despertar MPU6050
  writeReg(PWR_MGMT_1, 0x00);

  // microSD
  if (!SD.begin(CS_PIN)) {
    Serial.println("Error: no se pudo inicializar la microSD.");
    while (true) delay(1000);
  }
  Serial.println("microSD inicializada.");

  // Crear/abrir archivo y escribir encabezado si está vacío
  File f = SD.open("/temp_log.csv", FILE_WRITE);
  if (f) {
    if (f.size() == 0) {
      f.println("t_ms,Temp_C");
    }
    f.close();
  } else {
    Serial.println("Error: no se pudo crear/abrir temp_log.csv");
  }
}

void loop() {
  float tempC = readTempC();

  File f = SD.open("/temp_log.csv", FILE_WRITE);
  if (f) {
    f.print(millis());
    f.print(",");
    f.println(tempC, 2);
    f.close();
  } else {
    Serial.println("Error: no se pudo abrir temp_log.csv para escribir");
  }

  // También lo mostramos por serial
  Serial.print("Temp_C: ");
  Serial.println(tempC, 2);

  delay(500); // 2 muestras/seg aprox
}