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
  - `serde` & `toml`: Persistent configuration file support.
  - `home`: Cross-platform directory discovery.
  - `std::thread` & `mpsc`: Concurrent multi-device management.

## Firmware (The Receiver)
- **Framework:** Arduino / ESP-IDF via PlatformIO.
- **Hardware Platform:** ESP32-C6 (Waveshare ESP32-C6-GEEK).
- **Display Driver:** `Moon On Our Nation / GFX Library for Arduino` (ST7789).
- **Wi-Fi Management:** `tzapu/WiFiManager` for credential configuration.
- **JSON Parsing:** `bblanchon/ArduinoJson` for structured data updates.
- **UI Theming:** Custom `catppuccin_colors.h` (RGB565 Mocha palette).
- **Communication:** Native ESP32-C6 USB CDC (Serial over USB-C).

## Communication Protocol
- **Physical Layer:** USB-C (USB 2.0 Serial Emulation).
- **Framing:** Newline-terminated strings (\n).
- **Data Format:** Structured JSON using `ArduinoJson`.
  - **Identity:** `{"type": "Identity", "data": {"hostname": "...", "ip": "...", ...}}`
  - **Stats:** `{"type": "Stats", "data": {"cpu_percent": 12.5, "ram_used": 1024, ...}}`
  - **Version Request:** `{"type": "GetVersion"}`
- **Versioning:** Automated synchronization between Host (`Cargo.toml`) and Firmware (via PlatformIO `extra_scripts`).

## Build & Task Automation
- **Task Runner:** [go-task](https://taskfile.dev/) (`Taskfile.yml`)
- **Usage:** Used for all common project operations including building, testing, linting, and flashing.
- **Packaging:** `cargo-deb` and `cargo-generate-rpm` for Linux distribution.
- **CI/CD:** GitHub Actions for automated build, test, and artifact generation on release tags.

