# Testing individual components

Here we test that we are able to control all four components 
(ESP32 microcontroller, E-ink display, T+P+H sensor, and CO2ppm sensor).


# E-ink display

In this section, I am following the instructions in the _Preparation_ section 
from the [Waveshare wiki](https://www.waveshare.com/wiki/E-Paper_ESP32_Driver_Board).
Pictures and videos from this section can be found in the `e-ink_display` directory.
Below, I provide some extra notes for the steps in the wiki:

### Hardware preparation

- To know how to connect and disconnect the flexible display cables of the display 
to the microcontroller, search for images of _FFC_ (or _FPC_) cables.

- My E-ink display corresponds to version 2 because it has a sticker on the back labelled _V2_.

- My ESP32 microcontrollers corresponds to revision 3 because it has _Rev 3_ printed on its top.

- For my 1.54inc e-ink display, I need to set the outer-most switch of the ESP32 to _A_.

- My inner-most switch is set to _ON_.


### Software preparation

- Add the ESP32 link (not the ESP8266 one) in the add-on board manager URL of the Arduino IDE. 
This is done by going to `File > Preferences > Settings` and adding
```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
```
to the `Additional boards manager URLs`.

- Download this board in `Tools > Board > Boards Manager`.
There are two ESP32 board libraries, install the one from _Espressif Systems_.

- The _project file folder location_ (or _Sketchbook location_) is located in 
`File > Preferences`. In my case, it is `~/Arduino/`.

- Copy the `esp32-waveshare-epd` folder from 
```
test_components/e-ink_display/E-Paper_ESP32_Driver_Board_Code/examples/
```
to `~/Arduino/libraries/` and restarted the Arduino IDE.

- Open the `epd1in54_V2-demo.ino` file. Strangely, the `epd1in54b_V2-dem.ino` file, 
which has a `b` for _black_, has variables named `Red...` which refers to the 
tricolor e-ink display. However, I just have the black and white display.

- Select the _ESP32 Dev Module_ board with tty to the USB port, min is `dev/ttyS4`. 
If the USB port does not show up, connect the ESP32 via USB to the PC and then 
select again the board and the port.

- Change `#include "imagedata.h"` to `#include "ImageData.h"` in the `epd1in54_V2-demo` file.
The Arduino language is case sensitive.

- Change `#include "EPD_5IN83_V2.h"` to `#include "EPD_5in83_V2.h"` in 
```
~/Arduino/libraries/esp32-waveshare-epd/src/utility/EPD_5in83_V2.cpp
```

- Verifying the code, I get a warning saying: _This set of Touch APIs has been deprecated_.

- Trying to upload the code, I get an error which is specific to Linux systems. 
I was not in the `dialout` group so I could not talk to the ESP32.
I solved the problem with 
```
sudo usermod -a -G dialout $USER
```
and restarting the PC.

After completing these steps, the E-ink display works (see video in the `e-ink_display` directory).

The code is easy to modify to my needs. 
For example, to write text on the display, I can use `Paint_DrawString_EN`, whose signature is:
```
void Paint_DrawString_EN(UWORD Xstart, UWORD Ystart, const char * pString,
                         sFONT* Font, UWORD Color_Foreground, UWORD Color_Background)
```
which is pretty self-explanatory.

Note that one needs to first use the functions `Paint_...` to change the bits in the _Image_ object,
and then call `EPD_1IN54_V2_Display(Image)` to display this object on the screen.



# BME280 sensor (Temperature, Pressure, and Humidity)

The datasheet for BME280 specifies that it can be controlled via I2C and SPI.

The module can only be powered with 3.3VDC. 

The I2C/SPI only works on 3.3V. See discussion below.
The [Waveshare wiki](https://www.waveshare.com/wiki/E-Paper_ESP32_Driver_Board) 
has the full pinout scheme of the ESP32 in the FAQ, 
see also the attached pictures in the `bme280_sensor` directory.

The module uses I2C by default and the default I2C address of this module is _0x76_. 
If one connects the _SDO_ pin with _Vcc_, then the I2C address becomes _0x77_.

The information about which voltage each GPIO pin uses can be found in the datasheet, in particular: 
from the [Waveshare wiki](https://www.waveshare.com/wiki/E-Paper_ESP32_Driver_Board), 
one can go to the [ESP32-WROOM-32 datasheet](https://www.espressif.com.cn/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf),
whose section 4.2.1 about the GPIO pins refers to the [ESP32 Series datasheet](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf#cd-io-mux),
which contains a table with all the voltages for the pins (see page 70). 
Table 5.1 from the ESP32 Series datasheet shows that all the pins of the microcontroller are 3.3V 
(recall that microcontroller != board).
This same information is also present in the ESP32-WROOM-32 datasheet.

The ESP32 board has 5V pins which is converted to "stable" 3.3V and then fed to the ESP32 microcontroller. 
This can be seen in the [ESP32 Driver Board datasheet](https://files.waveshare.com/wiki/E-Paper-ESP32-Driver-Board/E-Paper_ESP32_Driver_Board_V3.pdf),
where the _VDD5V_ only appears in the _Power_ section (which is converted into _VDD3V3_) and in the IO connectors 
(because one can have access to the 5V input directly).

In the [ESP32 Series datasheet](https://documentation.espressif.com/esp32_datasheet_en.pdf#cd-peri-pin-config), 
section 4.10 states that the I2C interface can be done through any of the GPIO pins.
The same section states that the SPI interface can be done through any of the GPIO pins.
However, the [ESP32-WROOM-32 datasheet](https://www.espressif.com.cn/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf)
says that some GPIO pins are only input pins, not input/output.
The important information for this specific datasheet is that:
```
For SPI, the pins are multiplexed with GPIO6 ~ GPIO11 via the IO MUX.

For regular I2C, the pins used can be chosen from any GPIOs via the GPIO Matrix.
```

Looking around, I see that the common pins in this microcontroller for I2C are GPIO21 and GPIO22.
They can be set up in software to correspond to a I2C using, e.g.:
```
#include <Wire.h>

void setup() {
  Wire.begin(21, 22);  // SDA = 21, SCL = 22
}
```


### Hardware preparation

Connect the following pins from the BME280 to the ESP32 board:
- VCC --> 3.3V = 3V3
- GND --> GND = GND
- SDA --> GPIO21 = P21
- SCL --> GPIO22 = P22
- CSB --> 3.3V = 3V3
- SDO --> GND (for address `0x76`) = GND


### Software preparation

- In Library Manager, install the `Adafruit BME280` library.

- Open the file `bme280/test/test.ino` in Arduino IDE.

- When connecting the ESP32 board to the computer, check `dmesg | tail -n 30`
and search for `tty...`.

- In Arduino IDE, go ot `Tools > Port` and select the one in the previous step.

- Compile and upload the script to the ESP32 board.

- Then, go to `Tools > Serial Monitor` and select: `New Line` and `115200 baud`.
Wait for a couple of seconds and the following text should start appearing:
```
-----------------------------
Temperature = 25.30 °C
Pressure = 1014.91 hPa
Humidity = 44.25 %
-----------------------------
```


# GY-SCD40 sensor (CO2, Humidity, and Temperature)

### Hardware preparation

Connect the following pins from the GY-SCD40 to the ESP32 board:
- VCC --> 3.3V = 3V3
- GND --> GND = GND
- SDA --> GPIO21 = P21
- SCL --> GPIO22 = P22

Note: both the BME280 and the GY-SCD40 can use the same pins because
they are using the I2C (**bus**) connection to the microcontroller.
They can both use the same bus because they have different I2C addresses,
i.e., 0x76 or 0x77 for BME280 and 0x62 for GY-SCD40.


### Software preparation

- In Library Manager, install the `Sensirion I2C SCD4x` library.

- Open the file `gy-scd40/test/test.ino` in Arduino IDE.

- When connecting the ESP32 board to the computer, check `dmesg | tail -n 30`
and search for `tty...`.

- In Arduino IDE, go ot `Tools > Port` and select the one in the previous step.

- Compile and upload the script to the ESP32 board.

- Then, go to `Tools > Serial Monitor` and select: `New Line` and `115200 baud`.
Wait for a couple of seconds and the following text should start appearing:
```
-----------------------------
Temperature = 25.30 °C
Pressure = 1014.91 hPa
Humidity = 44.25 %
-----------------------------
```

