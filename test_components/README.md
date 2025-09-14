Here we test that we are able to control all four components (ESP32 microcontroller, E-ink display, TPH sensor, and CO2ppm sensor).


# E-ink display

In this section, I am following the instructions in the _Preparation_ section from the [Waveshare wiki](https://www.waveshare.com/wiki/E-Paper_ESP32_Driver_Board).
Pictures and videos from this section can be found in the `e-ink_display` directory.
Below, I provide some extra notes for the steps in the wiki:

### Hardware preparation

- To know how to connect and disconnect the flexible display cables, search for _FFC_ (or _FPC_) cables.
- My E-ink display corresponds to version 2 because it has a sticker on the back labelled _V2_.
- For my 1.54inc e-ink display, I need to set the first switch of the ESP32 to _A_. 

### Software preparation

- Add the ESP32 link (not the ESP8266 one) in the add-on board manager URL of the Arduino IDE.
- The download button for the ESP32 board is in Tools > Board > Boards Manager...
- There are two ESP32 board libraries, install the one from _Espressif Systems_.
- The _project file folder location_ is located in File > Preferences.
- I have moved the `esp32-waveshare-epd` folder inside `~/Arduino/libraries/` (not just `~/Arduino/`) and restarted the Arduino IDE.
- Open the `epd1in54_V2-demo.ino` file. 
Strangely, the `epd1in54b_V2-dem.ino` which has a `b` for _black_ has variables named `Red...` which refers to the tricolor e-ink display.
- Selected the _ESP32 Dev Module_ board (with tty to the USB port). 
If the USB port does not show up, connect the ESP32 via USB to the PC and then to select again the board and the port.
- Change `#include "imagedata.h"` to `#include "ImageData.h"` in the `epd1in54_V2-demo` file (the Arduino language is case sensitive).
- Change `#include "EPD_5IN83_V2.h"` to `#include "EPD_5in83_V2.h"` in `~/Arduino/libraries/esp32-waveshare-epd/src/utility/EPD_5in83_V2.cpp`.
- Verifying the code, I get a warning saying _This set of Touch APIs has been deprecated_ (but it is just a warning).
- Trying to upload the code, I get an error which is specific to linux systems. 
I was not in the `dialout` group so I could not talk to the ESP32.
I solved the problem with `sudo usermod -a -G dialout $USER` and restarting the PC.

The E-ink display works (see video in the `e-ink_display` directory).
