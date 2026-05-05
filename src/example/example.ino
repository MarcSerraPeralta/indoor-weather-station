#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"
#include <stdlib.h>
#include <Wire.h>

#include <Adafruit_BME280.h>
#include <SensirionI2cScd4x.h>

#define SDA_PIN 21
#define SCL_PIN 22

// Sensors
Adafruit_BME280 bme;
SensirionI2cScd4x scd4x;

// Display buffer
UBYTE *BlackImage;
UWORD Imagesize;

// Timing
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 30000; // 30s

void drawScreen(float t, float h, float p, uint16_t co2) {

    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);

    char buf[64];

    // Title
    Paint_DrawString_EN(10, 10, "Indoor Weather", &Font16, WHITE, BLACK);

    // BME280
    sprintf(buf, "Temp: %.2f C", t);
    Paint_DrawString_EN(10, 40, buf, &Font16, WHITE, BLACK);

    sprintf(buf, "Hum: %.2f %%", h);
    Paint_DrawString_EN(10, 65, buf, &Font16, WHITE, BLACK);

    sprintf(buf, "Pres: %.1f hPa", p);
    Paint_DrawString_EN(10, 90, buf, &Font16, WHITE, BLACK);

    // CO2
    sprintf(buf, "CO2: %u ppm", co2);
    Paint_DrawString_EN(10, 120, buf, &Font16, WHITE, BLACK);

    // Warning indicator
    if (co2 > 1000) {
        Paint_DrawString_EN(10, 150, "AIR QUALITY: POOR", &Font16, WHITE, BLACK);
    } else {
        Paint_DrawString_EN(10, 150, "AIR QUALITY: OK", &Font16, WHITE, BLACK);
    }

    EPD_1IN54_V2_Display(BlackImage);
}

void setup() {

    printf("Starting system...\n");

    DEV_Module_Init();

    // Init display
    EPD_1IN54_V2_Init();
    EPD_1IN54_V2_Clear();
    DEV_Delay_ms(500);

    // Buffer allocation
    Imagesize = ((EPD_1IN54_V2_WIDTH % 8 == 0) ?
                (EPD_1IN54_V2_WIDTH / 8) :
                (EPD_1IN54_V2_WIDTH / 8 + 1)) * EPD_1IN54_V2_HEIGHT;

    BlackImage = (UBYTE *)malloc(Imagesize);
    if (BlackImage == NULL) {
        printf("Memory allocation failed!\n");
        while (1);
    }

    Paint_NewImage(BlackImage,
                   EPD_1IN54_V2_WIDTH,
                   EPD_1IN54_V2_HEIGHT,
                   270,
                   WHITE);

    // I2C init
    Wire.begin(SDA_PIN, SCL_PIN);

    // BME280
    if (!bme.begin(0x76)) {
        printf("BME280 not found\n");
        while (1);
    }

    // SCD40
    scd4x.begin(Wire, 0x62);
    scd4x.stopPeriodicMeasurement();
    delay(1000);
    scd4x.startPeriodicMeasurement();

    printf("Sensors ready\n");

    lastUpdate = millis();
}

void loop() {

    bool dataReady = false;
    scd4x.getDataReadyStatus(dataReady);

    if (millis() - lastUpdate > updateInterval && dataReady) {

        // -------- BME280 --------
        float t = bme.readTemperature();
        float h = bme.readHumidity();
        float p = bme.readPressure() / 100.0;

        // -------- SCD40 --------
        uint16_t co2;
        float t2, h2;

        scd4x.readMeasurement(co2, t2, h2);

        // -------- DISPLAY --------
        drawScreen(t, h, p, co2);

        lastUpdate = millis();

        printf("Display updated\n");
    }

    delay(100);
}
