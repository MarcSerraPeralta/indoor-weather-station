#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"

#include <Wire.h>
#include <stdlib.h>

#include <Adafruit_BME280.h>
#include <SensirionI2cScd4x.h>

// =====================
// I2C PINS (ESP32)
// =====================
#define SDA_PIN 21
#define SCL_PIN 22

// =====================
// SENSORS
// =====================
Adafruit_BME280 bme;
SensirionI2cScd4x scd4x;

// =====================
// DISPLAY BUFFER
// =====================
UBYTE *BlackImage;
UWORD Imagesize;

// =====================
// TIMING
// =====================
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 30000; // 30 seconds

// =====================
// DRAW DASHBOARD
// =====================
void drawDashboard(float temp, float hum, float press, uint16_t co2,
                   int hh, int mm, int dd, int mo, int yy) {

    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);

    char val[16];
    char footer[64];

    int screenW = EPD_1IN54_V2_WIDTH;

    int y = 15;
    int rowH = 45;
    int vertShift = -3;
    int separation = 145;

    // Helper: right align string
    auto drawRight = [&](const char *text, int yPos) {
        int len = strlen(text);
        int textW = len * Font24.Width;
        int x = separation - 10 - textW;
        Paint_DrawString_EN(x, yPos, text, &Font24, WHITE, BLACK);
    };

    // Helper: center align string
    auto drawCentered = [&](const char *text, int yPos) {
        int len = strlen(text);
        int textW = len * Font8.Width;
        int x = (screenW - textW) / 2;
        Paint_DrawString_EN(x, yPos, text, &Font8, WHITE, BLACK);
    };

    // =====================
    // TEMPERATURE
    // =====================
    sprintf(val, "%.1f", temp);
    drawRight(val, y);
    Paint_DrawString_EN(separation, y - vertShift, "C", &Font16, WHITE, BLACK);
    y += rowH;

    // =====================
    // CO2
    // =====================
    sprintf(val, "%u", co2);
    drawRight(val, y);
    Paint_DrawString_EN(separation, y - vertShift, "ppm", &Font16, WHITE, BLACK);
    y += rowH;

    // =====================
    // HUMIDITY
    // =====================
    sprintf(val, "%.1f", hum);
    drawRight(val, y);
    Paint_DrawString_EN(separation, y - vertShift, "%", &Font16, WHITE, BLACK);
    y += rowH;

    // =====================
    // PRESSURE
    // =====================
    sprintf(val, "%.1f", press);
    drawRight(val, y);
    Paint_DrawString_EN(separation, y - vertShift, "hPa", &Font16, WHITE, BLACK);

    // =====================
    // FOOTER
    // =====================
    sprintf(footer,
        "Last updated: %02d:%02d %02d/%02d/%02d",
        hh, mm, dd, mo, yy);

    int footerY = EPD_1IN54_V2_HEIGHT - 10;
    drawCentered(footer, footerY);

    // Push to display
    EPD_1IN54_V2_Display(BlackImage);
}

// =====================
// SETUP
// =====================
void setup() {

    Serial.begin(115200);
    delay(1000);

    printf("Starting Indoor Weather Station...\n");

    // DISPLAY INIT
    DEV_Module_Init();
    EPD_1IN54_V2_Init();
    EPD_1IN54_V2_Clear();
    DEV_Delay_ms(500);

    // BUFFER
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

    // I2C INIT
    Wire.begin(SDA_PIN, SCL_PIN);

    // BME280 INIT
    if (!bme.begin(0x76)) {
        printf("BME280 not found\n");
        while (1);
    }

    // SCD40 INIT
    scd4x.begin(Wire, 0x62);
    scd4x.stopPeriodicMeasurement();
    delay(1000);
    scd4x.startPeriodicMeasurement();

    printf("Sensors ready\n");
}

// =====================
// LOOP
// =====================
void loop() {

    bool dataReady = false;
    scd4x.getDataReadyStatus(dataReady);

    if (millis() - lastUpdate > updateInterval && dataReady) {

        // =====================
        // READ BME280
        // =====================
        float temp = bme.readTemperature();
        float hum = bme.readHumidity();
        float press = bme.readPressure() / 100.0;

        // =====================
        // READ SCD40
        // =====================
        uint16_t co2;
        float t2, h2;

        scd4x.readMeasurement(co2, t2, h2);

        // =====================
        // FAKE TIME (replace later with RTC/NTP)
        // =====================
        unsigned long s = millis() / 1000;
        int mm = (s / 60) % 60;
        int hh = (s / 3600) % 24;

        int dd = 5;
        int mo = 5;
        int yy = 26;

        // =====================
        // DRAW SCREEN
        // =====================
        drawDashboard(temp, hum, press, co2,
                      hh, mm, dd, mo, yy);

        lastUpdate = millis();

        Serial.println("Display updated");
    }

    delay(100);
}
