# ESP32 Wi-Fi LED Video Player

A 16×16 WS2812B LED matrix video player controlled by an **ESP32**, with videos stored on a **microSD card** and selected from a local **Wi-Fi web interface**. A Python CLI wraps FFmpeg to convert MP4s into raw RGB888 frames.

## Features

- ESP32 Wi-Fi access point and browser-based video selection
- FAT32 microSD storage with `.rgb` video playback
- 16×16 RGB LED display at 15 frames per second
- Video looping and stop control
- 74AHCT125 level shifting from 3.3V ESP32 logic to 5V LED data
- MP4-to-RGB conversion tool

## Hardware

- ESP32-WROOM-32 development board
- 16×16 WS2812B LED matrix (256 LEDs)
- 74AHCT125 logic-level shifter
- 3.3V microSD SPI breakout and FAT32 microSD card
- External regulated 5V supply rated for the LED matrix load
- 330Ω data resistor, 0.1µF level-shifter decoupling capacitor, 1000µF bulk capacitor, appropriate wires/fuse

## Wiring

See [docs/wiring.md](docs/wiring.md) for complete connections and safety notes.

| ESP32 | Connected to |
| --- | --- |
| GPIO5 | microSD CS |
| GPIO18 | microSD CLK |
| GPIO19 | microSD MISO |
| GPIO23 | microSD MOSI |
| GPIO27 | 74AHCT125 pin 2 (LED data input) |
| 3V3 | microSD 3V3 |
| GND | microSD GND and common supply ground |

## Firmware setup

1. Install the ESP32 board package in Arduino IDE.
2. Install **FastLED** using Arduino Library Manager. `WiFi`, `WebServer`, `SPI`, and `SD` come with the ESP32 Arduino core.
3. Open `firmware/ESP32_LED_Video_Player.ino`.
4. Select **ESP32 Dev Module**, select the correct port, and upload.
5. Open Serial Monitor at **115200 baud** to check SD initialization.
6. With hardware safely powered, connect your phone or computer to the ESP32 AP:
   - SSID: `LED-Video-Player`
   - Password: `LEDplayer2026` (change in firmware before sharing or deploying)
   - Website: <http://192.168.4.1>
7. Choose an `.rgb` video from the web page.

This AP is local only; it does not provide internet access. The web interface has no per-user authentication; anyone connected to the AP can control playback.

## Convert MP4 to RGB

Install [FFmpeg](https://ffmpeg.org/download.html) so `ffmpeg` is on your PATH. Python 3 is required, but no Python packages are needed.

```bash
python tools/mp4_to_rgb.py my_video.mp4
```

This produces `my_video_16x16.rgb`. To specify the output filename:

```bash
python tools/mp4_to_rgb.py my_video.mp4 -o my_video.rgb
```

Copy the `.rgb` file to the **root** of a FAT32 microSD card. Each frame is 16 × 16 × 3 = **768 bytes**, in row-major RGB order. Playback is **15 FPS**, with no audio. Do not change width/height/fps without making corresponding firmware changes.

## Troubleshooting

- **SD CARD FAILED:** Confirm 3.3V supply, CS on GPIO5, CLK 18, MISO 19, MOSI 23, FAT32 card, and **secure/soldered connections** on the microSD module.
- **Website loads, no video listed:** Confirm the `.rgb` file is in the SD card root and has the `.rgb` extension.
- **Video listed, LEDs stay dark:** Run a simple FastLED color test; check GPIO27 → 74AHCT125 → resistor → **DIN**, 5V matrix power, and common ground.
- **Wrong LED order:** Adjust `SERPENTINE` and/or `pixelIndex()` for your matrix wiring layout.
- **Video plays at wrong speed:** Ensure conversion and firmware both use 15 FPS.

## Notes

- Designed for a horizontal serpentine matrix with its first pixel at the top left. Other layouts need a mapping change.
- The software power limit is not a substitute for safe supply sizing, fusing, wiring, and insulation.
- This repository intentionally does **not** include third-party video content. Convert and use media you have rights to distribute or display.
- Firmware is a cleaned-up project version based on the working configuration; test it on your specific hardware before treating it as a verified release.

## License

See [LICENSE](LICENSE) for MIT license terms.
