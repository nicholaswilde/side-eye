# Backlight Dimming & SPI Optimization

## Goal
Implement hardware-aligned backlight control and optimize SPI performance based on the official Waveshare ESP32-C6-GEEK demo analysis.

## Objectives
1.  **Backlight Control:** Switch from `digitalWrite` to `analogWrite` (PWM) for the LCD backlight (GPIO 6). Implement smooth fade-in/fade-out transitions for screen wake/sleep events.
2.  **SPI Speed:** Investigate increasing the SD card SPI frequency after successful initialization (currently 1MHz). Test reliability at higher speeds (e.g., 4MHz, 10MHz) to improve sync performance for large files.

## References
- Waveshare Demo: `LCD_Driver.cpp` (Backlight)
- Waveshare Demo: `DEV_Config.cpp` (SPI Clock Divider)

