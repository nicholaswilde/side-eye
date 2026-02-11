# Hardware Research: ESP32-C6-Geek SD Card

## Official Resources
- **Demo Package:** https://files.waveshare.com/wiki/ESP32-C6-GEEK/ESP32-C6-GEEK-Demo.zip
- **Wiki:** https://www.waveshare.com/wiki/ESP32-C6-GEEK

## Technical Specifications
The Waveshare ESP32-C6-Geek features an integrated microSD card slot shared on the same SPI bus as the LCD display.

### Pin Mapping
| Signal | GPIO | Notes |
|--------|------|-------|
| SCK    | 1    | Shared with LCD |
| MOSI   | 2    | Shared with LCD |
| MISO   | 0    | Strapping pin (Boot). Needs internal or external pull-up. |
| SD CS  | 10   | Dedicated Chip Select for SD card. |
| LCD CS | 5    | Dedicated Chip Select for LCD. |

## Known Issues & Solutions

### 1. Mount Failed on Boot
**Problem:** The SD card fails to mount (`SD: Mount Failed`) during the `setup()` sequence.
**Solutions:**
- **Initialization Order:** The SD card **must** be initialized before the LCD display. Initializing the display first can lock the SPI bus or set a clock frequency the SD card cannot negotiate.
- **Lower Frequency:** Initialize the SD card at a lower SPI frequency (e.g., 1MHz or 400kHz) and increase it later if needed.
- **Strapping Pin Conflict:** GPIO 0 is used for MISO. Ensure `pinMode(0, INPUT_PULLUP)` is called before `SD.begin()`.

### 2. Formatting Requirements
**Problem:** Card is detected but files cannot be read/written.
**Solution:** The standard Arduino `SD.h` library for ESP32-C6 only supports **FAT16** and **FAT32**. **exFAT** is not supported. Use the official SD Association Formatter to ensure a clean FAT32 partition.

### 3. Persistent Error after Crash
**Problem:** After a firmware crash or soft reset, the SD card remains in an error state.
**Solution:** The SD card does not always reset with the ESP32. A full **hardware power cycle** (unplugging the USB dongle) is often required to reset the SD card's internal controller.

## Recommended Initialization Sequence
```cpp
void setup() {
    // 1. Set all CS pins HIGH to disable SPI devices
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    pinMode(LCD_CS, OUTPUT);
    digitalWrite(LCD_CS, HIGH);

    // 2. Enable pull-up on MISO (GPIO 0)
    pinMode(0, INPUT_PULLUP);
    delay(100);

    // 3. Initialize SD first at low speed
    SPI.begin(SCK, MISO, MOSI, SD_CS);
    if (!SD.begin(SD_CS, SPI, 1000000)) {
        Serial.println("SD: Mount Failed");
    }

    // 4. Initialize Display second
    display.begin();
}
```
