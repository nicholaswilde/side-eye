# :eyes: SideEye: The USB Stat Monitor for Linux :terminal:

[![Rust](https://img.shields.io/badge/Host-Rust-orange?style=for-the-badge&logo=rust)](https://www.rust-lang.org/)
[![PlatformIO](https://img.shields.io/badge/Firmware-PlatformIO-blue?style=for-the-badge&logo=platformio)](https://platformio.org/)
[![Hardware](https://img.shields.io/badge/Hardware-ESP32--C6--GEEK-white?style=for-the-badge&logo=espressif)](https://www.waveshare.com/esp32-c6-geek.htm)

SideEye is a tiny USB-powered monitor that gives your Linux rig the "SideEye" by displaying real-time system stats on a dedicated LCD. It consists of a lightweight Rust host binary and custom ESP32-C6 firmware.

---

## :gear: System Architecture

1.  **Host (The Sender):** A Rust-based daemon that gathers rich system telemetry (CPU, RAM, Disk, Uptime, Identity) and streams it over USB Serial using structured JSON.
2.  **Firmware (The Receiver):** Arduino-based firmware running on the ESP32-C6-GEEK that parses the JSON payloads and renders a polished, color-coded UI on the built-in 1.14" LCD.

---

## :stopwatch: Quick Start

### 1. Prerequisites (Linux)
You will need `pkg-config` and `libudev` development headers for the serial communication library.

```bash
sudo apt install pkg-config libudev-dev
```

### 2. Flash the Firmware
Connect your **Waveshare ESP32-C6-GEEK** and flash the receiver code. We recommend using `task`.

```bash
task firmware:flash
```

### 3. Wi-Fi Setup
Upon first boot, the device will enter setup mode:
1.  Connect to the Wi-Fi AP named **"SideEye-XXXXXX"** (where XXXXXX is the last part of your device's MAC) using your phone or laptop.
2.  A captive portal should open (or navigate to `192.168.4.1`).
3.  Enter your local Wi-Fi credentials and save.
4.  The device will reboot and show **"WiFi Online!"**.

### 4. Run the Host Binary
The host will automatically attempt to detect the ESP32 on your serial ports.

```bash
task host:run
```

---

## :package: Components

### Part A: Rust Host
- **Zero-Config Discovery:** Automatically finds the ESP32-C6 by VID (0x303A).
- **Real-Time Telemetry:** Streams CPU usage, RAM bars, disk stats, and uptime every 1 second.
- **Robustness:** Handles hot-plugging and automatic reconnection seamlessly.
- **CLI Options:**
    - `--verbose`: Enable detailed logging of configuration and payloads.
    - `--dry-run`: Print stats to terminal without sending to serial.
    - `--port <PATH>`: Manually specify a serial port.

### Part B: ESP32-C6 Firmware
- **Intelligent UI State:** Clean "Waiting for Host" mode when idle, automatically transitioning to a detailed dashboard upon connection.
- **Orientation Support:** A physical button toggle rotates the screen 180 degrees to support any USB port orientation.
- **WiFiManager:** User-friendly Wi-Fi configuration via captive portal.
- **JSON Protocol:** Uses `ArduinoJson` for robust, extensible data handling.

---

## :balance_scale: License

This project is licensed under the [Apache License 2.0](./LICENSE).

---

## :pencil: Author

This project was started in 2026 by [Nicholas Wilde](https://github.com/nicholaswilde/).
