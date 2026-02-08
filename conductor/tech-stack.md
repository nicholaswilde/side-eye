# Tech Stack

## Host (The Sender)
- **Language:** Rust
- **Key Libraries:**
  - `serialport`: Cross-platform USB CDC communication.
  - `sysinfo`: System-level information gathering (Hostname, CPU, RAM, Disk, Thermal Sensors).
  - `nvml-wrapper`: NVIDIA Management Library wrapper for GPU telemetry.
  - `local-ip-address`: Reliable LAN IP discovery.
  - `mac_address`: MAC address retrieval.
  - `anyhow`: Context-aware error handling.
  - `clap`: CLI argument parsing and auto-documentation.
  - `serde`, `toml`, `yaml`, `json`: Data serialization and multi-format support.
  - `figment`: Flexible, multi-layered configuration management.
  - `dotenvy`: Environment variable loading from `.env` files.
  - `home`: Cross-platform directory discovery.
  - `std::thread` & `mpsc`: Concurrent multi-device management.

## Firmware (The Receiver)
- **Framework:** Arduino / ESP-IDF via PlatformIO.
- **Hardware Platform:** ESP32-C6 (Waveshare ESP32-C6-GEEK).
- **Storage:** Integrated Micro SD card support via `SD` and `SPI` libraries.
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
  - **Stats:** `{"type": "Stats", "data": {"cpu_percent": 12.5, "ram_used": 1024, ..., "alert_level": 0}}`
  - **Version Request:** `{"type": "GetVersion"}`
- **Versioning:** Automated synchronization between Host (`Cargo.toml`) and Firmware (via PlatformIO `extra_scripts`).

## Build & Task Automation
- **Task Runner:** [go-task](https://taskfile.dev/) (`Taskfile.yml`)
- **Usage:** Used for all common project operations including building, testing, linting, and flashing.
- **Cross-Compilation:** [`cross`](https://github.com/cross-rs/cross) for multi-architecture builds.
  - **Supported Architectures:**
    - `x86_64-unknown-linux-gnu` (AMD64)
    - `aarch64-unknown-linux-gnu` (ARM64)
    - `armv7-unknown-linux-gnueabihf` (ARMv7)
    - `arm-unknown-linux-gnueabihf` (ARMv6/v5te)
- **Packaging:** `cargo-deb` and `cargo-generate-rpm` for Linux distribution.
- **CI/CD:** Unified GitHub Actions workflow for automated build, test, and synchronized release of both host binaries and firmware artifacts.


## Deployment
- **Docker:** Multi-stage `Dockerfile` using `cargo-chef` for build optimization and `gcr.io/distroless/cc-debian12` for a minimal runtime environment.
- **Automated Builds:** GitHub Actions workflow automatically builds multi-arch images (`amd64`, `arm64`, `arm/v7`) and publishes them to Docker Hub (`nicholaswilde/side-eye-host`) and GHCR (`ghcr.io/nicholaswilde/side-eye-host`) on every release tag.
- **Hardware Access:** Requires `--privileged` mode or direct device mapping for USB serial access.
