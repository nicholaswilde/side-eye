# :eyes: SideEye: The USB Stat Monitor for Linux :desktop_computer:

[![Coveralls](https://img.shields.io/coveralls/github/nicholaswilde/side-eye/main?style=for-the-badge&logo=coveralls)](https://coveralls.io/github/nicholaswilde/side-eye?branch=main)
[![task](https://img.shields.io/badge/Task-Enabled-brightgreen?style=for-the-badge&logo=task&logoColor=white)](https://taskfile.dev/#/)
[![ci](https://img.shields.io/github/actions/workflow/status/nicholaswilde/side-eye/ci.yml?label=CI&style=for-the-badge&logo=github-actions)](https://github.com/nicholaswilde/side-eye/actions/workflows/ci.yml)
[![docker](https://img.shields.io/github/actions/workflow/status/nicholaswilde/side-eye/docker.yml?label=Docker&style=for-the-badge&logo=docker)](https://github.com/nicholaswilde/side-eye/actions/workflows/docker.yml)
[![release](https://img.shields.io/github/actions/workflow/status/nicholaswilde/side-eye/release.yml?label=Release&style=for-the-badge&logo=github-actions)](https://github.com/nicholaswilde/side-eye/actions/workflows/release.yml)
[![Rust](https://img.shields.io/badge/Host-Rust-orange?style=for-the-badge&logo=rust)](https://www.rust-lang.org/)
[![PlatformIO](https://img.shields.io/badge/Firmware-PlatformIO-blue?style=for-the-badge&logo=platformio)](https://platformio.org/)
[![Hardware](https://img.shields.io/badge/Hardware-ESP32--C6--GEEK-white?style=for-the-badge&logo=espressif)](https://www.waveshare.com/esp32-c6-geek.htm)

> [!WARNING]
> This project is currently in active development (v0.X.X) and is **not production-ready**. Features may change, and breaking changes may occur without notice.

SideEye is a tiny USB-powered monitor that gives your Linux rig the "SideEye" by displaying real-time system stats on a dedicated LCD. It consists of a lightweight Rust host binary and custom ESP32-C6 firmware.

---

## :gear: System Architecture

1.  **Host (The Sender):** A Rust-based daemon that gathers rich system telemetry (CPU, RAM, Disk, Uptime, Identity) and streams it over USB Serial using structured JSON.
2.  **Firmware (The Receiver):** Arduino-based firmware running on the [ESP32-C6-GEEK](https://www.waveshare.com/esp32-c6-geek.htm) that parses the JSON payloads and renders a polished, color-coded UI on the built-in 1.14" LCD.

---

## :stopwatch: Quick Start

### 1. Prerequisites (Linux)
You will need `pkg-config` and `libudev` development headers for the serial communication library.

```bash
sudo apt install pkg-config libudev-dev
```

### 2. Flash the Firmware
Connect your **Waveshare ESP32-C6-GEEK** and flash the receiver code. You can use `task` to build and flash locally:

```bash
task firmware:flash
```

Alternatively, download the pre-compiled `side-eye-<version>-firmware.zip` from the [latest release](https://github.com/nicholaswilde/side-eye/releases) and flash the included `.bin` files using `esptool.py` or your preferred ESP32 flashing tool.

### 3. Wi-Fi & MQTT Setup
Upon first boot, the device will enter setup mode:
1.  Connect to the Wi-Fi AP named **"SideEye-XXXXXX"** (where XXXXXX is the last part of your device's MAC) using your phone or laptop.
2.  Navigate to `192.168.4.1`.
3.  Enter your Wi-Fi credentials and optional **MQTT Broker** details.
4.  The device will reboot and show **"WiFi Online!"**.

> [!TIP]
> You can also bake in default credentials at compile-time by creating `firmware/include/secrets.h` (see `secrets.h.example`).

### 4. MQTT Usage
Once connected to your broker, you can interact with the device via MQTT. For example, to manually rotate the screen:
```bash
mosquitto_pub -h <MQTT_BROKER_IP> -t "side-eye/<DEVICE_ID>/rotate" -m "1"
```

### 5. Run the Host Binary
The host will automatically attempt to detect the ESP32 on your serial ports.

```bash
task host:run
```

---

## :package: Components & Features

### Rust Host Agent
- **Multi-Format Configuration:** Supports `side-eye.{toml,yaml,yml,json}` and `.env` files.
- **Zero-Config Discovery:** Automatically finds the ESP32-C6 by VID (0x303A) and creates a `/dev/side-eye` symlink via udev.
- **Multi-Device Support:** Seamlessly handles connections to multiple SideEye devices simultaneously.
- **SD Card Synchronization:** Synchronizes a local host directory to the device's integrated SD card over serial.
- **Linux Packaging:** Officially supports `.deb` (Debian/Ubuntu) and `.rpm` (Fedora/RHEL) packages, plus systemd integration for auto-start.

### ESP32-C6 Firmware
- **Catppuccin UI:** Polished, color-coded interface using the Catppuccin Mocha color palette.
- **Paged UI:** Cycles through **Identity**, **Resources** (CPU/RAM bars), **Status** (Disk/Uptime), **SD Card**, **Thermal** (CPU Temp/GPU Load), and **Network** pages.
- **Offline Access:** The **SD Card** page remains accessible via button cycle even when disconnected from the host, showing local storage usage.
- **Intelligent UI State:** Clean "Waiting..." mode when idle, automatically transitioning to a detailed dashboard upon host connection.
- **Visual Alerts:** Banner changes color based on resource usage thresholds (Mauve -> Yellow -> Flashing Red) and automatically switches to the Resources page.
- **Network History:** Dedicated page with real-time sparklines for download and upload throughput history.
- **Home Assistant Integration:** Automatic MQTT Discovery—sensors appear instantly in your HA dashboard.
- **Power Management:** 1-minute auto-off timeout to save screen life; wakes instantly on button interaction.
- **Integrated Releases:** Unified GitHub Releases provide synchronized host binaries and a complete firmware bundle (`firmware.bin`, `bootloader.bin`, `partitions.bin`) in a single zip.
- **Automated Versioning:** Firmware version is automatically synchronized with the host's `Cargo.toml` during build.

---

## :mouse: Interactivity (Physical Button)

The onboard "Boot" button (GPIO 9) provides full control:
- **Single Click:** Advance to the next information page (works even when disconnected or wakes the screen if it's off).
- **Double Click:** Rotate the display 180 degrees (supports any USB port orientation). **Works even on the setup screen!**
- **Hold (1s):** Toggle the LCD backlight on/off manually.
- **Long Hold (10s):** Factory Reset. Clears all Wi-Fi and MQTT settings. A countdown will appear on screen.

---

## :wrench: Configuration

See [host/config.toml.example](host/config.toml.example) for a documented template.

Precedence (highest to lowest):
1.  **CLI Flags:** `--port`, `--baud-rate`, `--interval`, `--verbose`, `--config`, `--dry-run`, `--monitor-all`.
2.  **Environment Variables:** Prefixed with `SIDEEYE_` (e.g., `SIDEEYE_INTERVAL=500`).
3.  **Config Files:** 
    -   Local: `side-eye.{toml,yaml,yml,json}` in current directory.
    -   Global: `~/.config/side-eye/config.{toml,yaml,yml,json}`.
    -   **Thresholds Section:** Configure `cpu_warning`, `cpu_critical`, `ram_warning`, and `ram_critical` (0-100).
4.  **.env File:** Loaded from the current working directory.

---

## :balance_scale: License

This project is licensed under the [Apache License 2.0](./LICENSE).

---

## :pencil: Author

This project was started in 2026 by [Nicholas Wilde](https://github.com/nicholaswilde/).
