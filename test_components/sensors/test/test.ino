#include <Wire.h>
#include <Adafruit_BME280.h>
#include <SensirionI2cScd4x.h>

#define SDA_PIN 21
#define SCL_PIN 22

// Sensors
Adafruit_BME280 bme;
SensirionI2cScd4x scd4x;

// Warm-up handling
unsigned long startTime;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Starting Indoor Weather Station...");

  // I2C init
  Wire.begin(SDA_PIN, SCL_PIN);

  // -------------------------
  // BME280 init
  // -------------------------
  if (!bme.begin(0x76)) {
    Serial.println("BME280 not found (try 0x77)");
    while (1);
  }
  Serial.println("BME280 OK");

  // -------------------------
  // SCD40 init
  // -------------------------
  scd4x.begin(Wire, 0x62);

  scd4x.stopPeriodicMeasurement();
  delay(1000);
  scd4x.startPeriodicMeasurement();

  Serial.println("SCD40 OK");
  Serial.println("Warming up sensors... (30s recommended)");

  startTime = millis();
}

void loop() {
  // =========================
  // Read BME280 (always ready)
  // =========================
  float bme_temp = bme.readTemperature();
  float bme_hum = bme.readHumidity();
  float bme_press = bme.readPressure() / 100.0;

  // =========================
  // Read SCD40
  // =========================
  bool dataReady = false;
  scd4x.getDataReadyStatus(dataReady);

  uint16_t co2 = 0;
  float scd_temp = 0;
  float scd_hum = 0;
  bool valid_scd = false;

  if (dataReady) {
    scd4x.readMeasurement(co2, scd_temp, scd_hum);

    // Filter invalid readings
    if (co2 > 0 && co2 < 10000) {
      valid_scd = true;
    }
  }

  // =========================
  // Output
  // =========================

  // Only print SCD40 if valid AND after warm-up
  if (millis() - startTime > 5000 && valid_scd) {
    Serial.println("\n============================");

    Serial.print("BME280 Temp: ");
    Serial.print(bme_temp);
    Serial.println(" °C");

    Serial.print("BME280 Hum: ");
    Serial.print(bme_hum);
    Serial.println(" %");

    Serial.print("BME280 Pressure: ");
    Serial.print(bme_press);
    Serial.println(" hPa");

    Serial.print("SCD40 CO2: ");
    Serial.print(co2);
    Serial.println(" ppm");

    Serial.print("SCD40 Temp: ");
    Serial.print(scd_temp);
    Serial.println(" °C");

    Serial.print("SCD40 Hum: ");
    Serial.print(scd_hum);
    Serial.println(" %");

    Serial.println("============================");

  } else {
    Serial.println("SCD40 warming up / no valid data yet...");
  }
  

  delay(2000);
}