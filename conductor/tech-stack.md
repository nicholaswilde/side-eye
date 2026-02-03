# Tech Stack

## Host (The Sender)
- **Language:** Rust
- **Key Libraries:**
  - `serialport`: Cross-platform USB CDC communication.
  - `sysinfo`: System-level information gathering (Hostname).
  - `local-ip-address`: Reliable LAN IP discovery.
  - `mac_address`: MAC address retrieval.
  - `anyhow`: Context-aware error handling.
  - `clap`: CLI argument parsing and auto-documentation.

## Firmware (The Receiver)
- **Framework:** Arduino / ESP-IDF via PlatformIO.
- **Hardware Platform:** ESP32-C6 (Waveshare ESP32-C6-GEEK).
- **Display Driver:** `Moon On Our Nation / GFX Library for Arduino` (ST7789).
- **Communication:** Native ESP32-C6 USB CDC (Serial over USB-C).

## Communication Protocol
- **Physical Layer:** USB-C (USB 2.0 Serial Emulation).
- **Framing:** Newline-terminated strings (\n).
- **Data Format:** Pipe-delimited payload: `HOSTNAME|IP|MAC`.
