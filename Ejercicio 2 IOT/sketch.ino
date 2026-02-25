#include <Wire.h>

#define MPU_ADDR 0x68
#define SDA_PIN 21
#define SCL_PIN 22

// Registros
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B

int16_t ax, ay, az, gx, gy, gz;

void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission(true);
}

void readBytes(uint8_t reg, uint8_t count, uint8_t* data) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, count, true);
  for (uint8_t i = 0; i < count; i++) data[i] = Wire.read();
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(SDA_PIN, SCL_PIN);

  // “Despertar” el MPU6050
  writeReg(PWR_MGMT_1, 0x00);

  Serial.println("MPU6050 listo (sin librerias).");
  Serial.println("ax_g ay_g az_g gx_dps gy_dps gz_dps");
}

void loop() {
  uint8_t data[14];
  readBytes(ACCEL_XOUT_H, 14, data);

  ax = (data[0] << 8) | data[1];
  ay = (data[2] << 8) | data[3];
  az = (data[4] << 8) | data[5];
  // data[6], data[7] son temperatura (si la quieres luego)
  gx = (data[8] << 8) | data[9];
  gy = (data[10] << 8) | data[11];
  gz = (data[12] << 8) | data[13];

  // Escalas por defecto:
  // Accel ±2g -> 16384 LSB/g
  // Gyro ±250 dps -> 131 LSB/(deg/s)
  float ax_g = ax / 16384.0;
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;

  float gx_dps = gx / 131.0;
  float gy_dps = gy / 131.0;
  float gz_dps = gz / 131.0;

  // Solo números, friendly para Serial Plotter
  Serial.print(ax_g, 3); Serial.print(" ");
  Serial.print(ay_g, 3); Serial.print(" ");
  Serial.print(az_g, 3); Serial.print(" ");
  Serial.print(gx_dps, 3); Serial.print(" ");
  Serial.print(gy_dps, 3); Serial.print(" ");
  Serial.println(gz_dps, 3);

  delay(100);
}
