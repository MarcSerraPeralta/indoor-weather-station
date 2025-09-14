# Indoor weather station

Indoor weather station that shows temperature, pressure, humidity, and CO2 concentration in a e-ink display.

_Missing final weather station picture_

## Design choices

### **E-ink display**: Waveshare 1.54-inch E-Ink E-Paper Display Panel - Black and White 

The parameter values (T, P, H, CO2ppm) need to be constantly displayed and they are updated every ~1-5min.
The best type of display for these conditions is an e-ink display. 
One of the limitations of these displays is their low refresh rate.
I have chosen this specific display because:
- it supports partial refresh, which takes ~0.3s [ref](https://www.waveshare.com/wiki/1.54inch_e-Paper_Module_Manual#Resources)
- the full refresh takes ~2s [ref](https://www.waveshare.com/wiki/1.54inch_e-Paper_Module_Manual#Resources)
- it is small enough to display T, P, H, and CO2ppm, so it is cheaper
- it is just the display panel (without the module) which has thinner edges, for a smaller station enclosure

SKU: 12561 from Waveshare

### **ESP-32 microcontroller**: Waveshare Universal e-Paper Raw Panel Driver Board - ESP32

ESP32 microcontrollers can be as cheat as an Arduino Nano but supports Wifi and Bluetooth connection.
I have chosen this specific microcontroller because:
- it already has the connector for the e-ink display panel, so I don't have to spend money on a connector
- it has GPIO pins for I2C which is the most common communication protocol for sensors [ref](documentation/gpio_pins_esp32_waveshare_answer.pdf) [ref](documentation/gpio_pins_esp32_tinytronics_answer.pdf)

SKU: 15823 from Waveshare

### **Sensor for temperature, pressure, and humidity**: BME280

It is the cheapest 3-in-1 sensor that I could find that can be connected through I2C or SPI.

_Note: do not get confused with the BMP280 sensor (it only measures temperature and pressure)_

SKU: 001511 from TinyTronics

### **Sensor for CO2 concentration**: GY-SCD40

_Note: this sensor is quite expensive and it is not required for a simple indoors weather station. As I was just going to build it once, I though "might as well add this!"_

This is "true" CO2 concentration sensor (there is a difference between a "CO2 sensor" and a "eCO2 sensor").
It is the cheapest I could find that has good accuracy (+/- 50ppm) in the "normal" CO2 range (0 - 2000ppm) and that can be connected through I2C.

SKU: 005207 from TinyTronics

## Price

All the parts were bought from TinyTronics (cheapest delivery fee, as they are based on Eindhoven).

| Part          | Price (€) |
| ------------- | -------------: |
| E-ink display | 7.50 |
| ESP32 microcontroller | 13.00 |
| T, P, H sensor | 5.50 |
| CO2ppm sensor | 18.25 |
| **Total** | **44.25** |

## Testing the components individually

Each component has been tested 
(1) to know which software is required to control it, and 
(2) to check that it works correctly. 
The scripts and setups to test each component individually can be found in `test_componennts/`.

