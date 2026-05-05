#include "DEV_Config.h"
#include "EPD.h"
#include "GUI_Paint.h"

#include <Wire.h>
#include <stdlib.h>

#include <Adafruit_BME280.h>
#include <SensirionI2cScd4x.h>

#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>

// =====================
// I2C PINS
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
unsigned long lastMeasurement = 0;
const unsigned long measurementInterval = 60000; // 1 min

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 300000; // 5 min

int FULL_REFRESH_EVERY = 11;
int updateCount = 0;

// =====================
// WIFI
// =====================
const char* ssid = "TP-Link_9952";
const char* password = "";

// =====================
// CLEAR ROW (FIXES ARTIFACTS)
// =====================
void clearRow(int yPos) {
    int yStart = max(0, yPos - 20);
    int yEnd   = min((int)EPD_1IN54_V2_HEIGHT,
                     yPos + Font24.Height + 20);

    Paint_ClearWindows(
        0,
        yStart,
        EPD_1IN54_V2_WIDTH,
        yEnd,
        WHITE
    );
}

// =====================
// DRAW DASHBOARD
// =====================
void drawDashboard(float temp, float hum, float press, uint16_t co2,
                   int hh, int mm, int dd, int mo, int yy) {

    Paint_SelectImage(BlackImage);

    char val[16];
    char footer[64];

    int screenW = EPD_1IN54_V2_WIDTH;

    int y = 15;
    int rowH = 45;
    int vertShift = -3;
    int separation = 145;

    // Right align
    auto drawRight = [&](const char *text, int yPos) {
        int len = strlen(text);
        int textW = len * Font24.Width;
        int x = separation - 10 - textW;
        Paint_DrawString_EN(x, yPos, text, &Font24, WHITE, BLACK);
    };

    // Center align
    auto drawCentered = [&](const char *text, int yPos) {
        int len = strlen(text);
        int textW = len * Font8.Width;
        int x = (screenW - textW) / 2;
        Paint_DrawString_EN(x, yPos, text, &Font8, WHITE, BLACK);
    };

    // =====================
    // TEMPERATURE
    // =====================
    clearRow(y);
    sprintf(val, "%.1f", temp);
    drawRight(val, y);
    Paint_DrawString_EN(separation, y - vertShift, "C", &Font16, WHITE, BLACK);
    y += rowH;

    // =====================
    // CO2
    // =====================
    clearRow(y);
    sprintf(val, "%u", co2);
    drawRight(val, y);
    Paint_DrawString_EN(separation, y - vertShift, "ppm", &Font16, WHITE, BLACK);
    y += rowH;

    // =====================
    // HUMIDITY
    // =====================
    clearRow(y);
    sprintf(val, "%.1f", hum);
    drawRight(val, y);
    Paint_DrawString_EN(separation, y - vertShift, "%", &Font16, WHITE, BLACK);
    y += rowH;

    // =====================
    // PRESSURE
    // =====================
    clearRow(y);
    sprintf(val, "%.1f", press);
    drawRight(val, y);
    Paint_DrawString_EN(separation, y - vertShift, "hPa", &Font16, WHITE, BLACK);

    // =====================
    // FOOTER
    // =====================
    int footerY = EPD_1IN54_V2_HEIGHT - 10;

    Paint_ClearWindows(
        0,
        footerY - Font8.Height - 4,
        EPD_1IN54_V2_WIDTH,
        footerY + Font8.Height + 4,
        WHITE
    );

    sprintf(footer,
        "Last updated: %02d:%02d %02d/%02d/%04d",
        hh, mm, dd, mo, yy);

    drawCentered(footer, footerY);
}

// =====================
// SEND DATA
// =====================
void sendData(float temp, float hum, float press, uint16_t co2,
              int ss, int hh, int mm, int dd, int mo, int yy) {

    HTTPClient http;

    http.begin("http://192.168.0.50:8000");
    http.addHeader("Content-Type", "text/plain");

    char data[128];
    sprintf(data,
        "%04d-%02d-%02d,%02d:%02d:%02d,%.2f,%.2f,%.2f,%u",
        yy, mo, dd,
        hh, mm, ss,
        temp, hum, press,
        co2);

    int code = http.POST(data);

    Serial.print("HTTP Response: ");
    Serial.println(code);

    http.end();
}

// =====================
// SETUP
// =====================
void setup() {

    Serial.begin(115200);
    delay(1000);

    // DISPLAY INIT
    DEV_Module_Init();
    EPD_1IN54_V2_Init();
    EPD_1IN54_V2_Clear();

    Imagesize = ((EPD_1IN54_V2_WIDTH % 8 == 0) ?
                (EPD_1IN54_V2_WIDTH / 8) :
                (EPD_1IN54_V2_WIDTH / 8 + 1)) * EPD_1IN54_V2_HEIGHT;

    BlackImage = (UBYTE *)malloc(Imagesize);

    Paint_NewImage(BlackImage,
                   EPD_1IN54_V2_WIDTH,
                   EPD_1IN54_V2_HEIGHT,
                   270,
                   WHITE);

    // I2C
    Wire.begin(SDA_PIN, SCL_PIN);

    // Sensors
    bme.begin(0x76);

    scd4x.begin(Wire, 0x62);
    scd4x.stopPeriodicMeasurement();
    delay(1000);
    scd4x.startPeriodicMeasurement();

    // WIFI
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) delay(500);

    // TIME
    configTime(3600, 3600, "pool.ntp.org");

    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) delay(500);

    // INITIAL FULL DRAW
    drawDashboard(0,0,0,0,0,0,0,0,0);

    EPD_1IN54_V2_Display(BlackImage);

    // CRITICAL: prepare partial mode
    EPD_1IN54_V2_DisplayPartBaseImage(BlackImage);
    EPD_1IN54_V2_Init_Partial();
}

// =====================
// LOOP
// =====================
void loop() {

    bool dataReady = false;
    scd4x.getDataReadyStatus(dataReady);

    if (millis() - lastMeasurement > measurementInterval && dataReady) {

        float temp = bme.readTemperature();
        float hum = bme.readHumidity();
        float press = bme.readPressure() / 100.0;

        uint16_t co2;
        float t2, h2;
        scd4x.readMeasurement(co2, t2, h2);

        struct tm timeinfo;
        int ss, hh, mm, dd, mo, yy;

        if (getLocalTime(&timeinfo)) {
            ss = timeinfo.tm_sec;
            hh = timeinfo.tm_hour;
            mm = timeinfo.tm_min;
            dd = timeinfo.tm_mday;
            mo = timeinfo.tm_mon + 1;
            yy = timeinfo.tm_year + 1900;
        }

        sendData(temp, hum, press, co2,
                 ss, hh, mm, dd, mo, yy);

        lastMeasurement = millis();

        bool nightBlock = (hh >= 1 && hh <= 6);

        if ((millis() - lastUpdate > updateInterval) && !nightBlock) {

            updateCount++;

            bool fullRefresh = (updateCount >= FULL_REFRESH_EVERY);

            if (fullRefresh) {

                Serial.println("FULL refresh");

                EPD_1IN54_V2_Init();
                EPD_1IN54_V2_Clear();

                Paint_NewImage(BlackImage,
                    EPD_1IN54_V2_WIDTH,
                    EPD_1IN54_V2_HEIGHT,
                    270,
                    WHITE);

                drawDashboard(temp, hum, press, co2,
                              hh, mm, dd, mo, yy);

                EPD_1IN54_V2_Display(BlackImage);

                // re-enable partial
                EPD_1IN54_V2_DisplayPartBaseImage(BlackImage);
                EPD_1IN54_V2_Init_Partial();

                updateCount = 0;
            }
            else {

                Serial.println("PARTIAL refresh");

                drawDashboard(temp, hum, press, co2,
                              hh, mm, dd, mo, yy);

                EPD_1IN54_V2_DisplayPart(BlackImage);
            }

            lastUpdate = millis();
        }
    }

    delay(1000);
}