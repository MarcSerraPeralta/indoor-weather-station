#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_BME280 bme;  // I2C

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("BME280 test (ESP32)");

  // Initialize I2C with chosen pins
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize BME280 (0x76 is default with SDO → GND)
  if (!bme.begin(0x76)) {
    Serial.println("Could not find BME280 sensor!");
    Serial.println("Check wiring or try address 0x77.");
    while (1);
  }

  Serial.println("BME280 initialized successfully!");
}

void loop() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" °C");

  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0);  // Pa → hPa
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println("-----------------------------");

  delay(2000);
}
