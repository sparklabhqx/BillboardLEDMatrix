# Billboard LED Matrix

ESP32-S2 mini web-controlled text display for a 4-module MAX7219 FC-16 LED matrix.

## Hardware

- ESP32-S2 mini, such as the LOLIN/WEMOS S2 mini
- 4x MAX7219 FC-16 8x8 LED matrix modules, chained as a 32x8 display
- Matrix wiring:
  - DIN: GPIO11 / MOSI
  - CLK: GPIO7 / SCK
  - CS / LOAD: GPIO12 / SS
  - VCC: 5V
  - GND: GND

## Libraries

Install these in Arduino IDE:

- MD_Parola
- MD_MAX72XX
- ESP32 board package

Select a compatible ESP32-S2 mini board in Arduino IDE, for example `LOLIN S2 Mini`.

## WiFi

The sketch contains no personal WiFi credentials. By default it starts a fallback access point:

- SSID: `HD-Billboard`
- Password: `change-me`
- URL: `http://192.168.4.1/`

To connect it to your own WiFi, edit `WIFI_SSID` and `WIFI_PASSWORD` in `BillboardLEDMatrix.ino` locally before flashing.
