# Wiring and power

**Disconnect USB and the external supply before changing wiring.** Verify supply voltage and polarity with a multimeter before energizing the matrix. Do not handle exposed mains terminals.

## microSD breakout (marked 3V3)

| microSD | ESP32 |
| --- | --- |
| 3V3 | 3V3 (NOT 5V) |
| GND | GND |
| CS | GPIO5 |
| CLK | GPIO18 |
| MOSI | GPIO23 |
| MISO | GPIO19 |

Bare jumper wires pushed through unsoldered plated holes may not make reliable contact. Use securely soldered headers or a breakout with preinstalled pins.

## 74AHCT125 DIP-14

| Pin | Connection |
| --- | --- |
| 1 (1OE, active low) | GND |
| 2 (1A) | ESP32 GPIO27 |
| 3 (1Y) | 330Ω resistor, then LED matrix DIN |
| 7 (GND) | Common GND |
| 14 (VCC) | Regulated +5V |
| 4, 10, 13 (unused OEs) | +5V |
| 5, 9, 12 (unused inputs) | GND |
| 6, 8, 11 (unused outputs) | Leave unconnected |

Install a **0.1µF ceramic capacitor** between pins 14 and 7, close to the IC.

## LED matrix and power

| Matrix | Connection |
| --- | --- |
| 5V | External regulated +5V |
| GND | External supply negative and ESP32 GND |
| DIN | From 74AHCT125 pin 3 through 330Ω resistor |
| DOUT | Not used |

Install a **1000µF electrolytic capacitor** across matrix 5V and GND near the panel, observing polarity. Route LED power directly from the supply using suitable wire and protection, **not through the ESP32, USB, or breadboard rails**. 256 WS2812B LEDs can theoretically draw approximately 15.4A at full white (60mA each); the firmware uses reduced brightness and a software power limit, but the supply and wiring must be designed for credible fault and load conditions. Connect all grounds together.

The ESP32 can be powered by USB while the matrix uses its external 5V supply. Never connect the external 5V directly to the ESP32 3V3 pin.
