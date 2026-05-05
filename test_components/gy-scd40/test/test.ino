#include <Arduino.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>

SensirionI2cScd4x scd4x;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);  // ESP32 I2C pins

  scd4x.begin(Wire, 0x62);

  scd4x.stopPeriodicMeasurement();
  delay(500);

  scd4x.startPeriodicMeasurement();

  Serial.println("SCD40 started");
}

void loop() {
  bool dataReady = false;

  scd4x.getDataReadyStatus(dataReady);

  if (!dataReady) {
    delay(100);
    return;
  }

  uint16_t co2;
  float temperature;
  float humidity;

  scd4x.readMeasurement(co2, temperature, humidity);

  Serial.print("CO2: ");
  Serial.print(co2);
  Serial.print(" ppm | Temp: ");
  Serial.print(temperature);
  Serial.print(" °C | Hum: ");
  Serial.print(humidity);
  Serial.println(" %");

  delay(2000);
}