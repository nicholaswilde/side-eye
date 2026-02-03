# :eyes: SideEye: The USB Stat Monitor for Linux :terminal:

[![Rust](https://img.shields.io/badge/Host-Rust-orange?style=for-the-badge&logo=rust)](https://www.rust-lang.org/)
[![PlatformIO](https://img.shields.io/badge/Firmware-PlatformIO-blue?style=for-the-badge&logo=platformio)](https://platformio.org/)
[![Hardware](https://img.shields.io/badge/Hardware-ESP32--C6--GEEK-white?style=for-the-badge&logo=espressif)](https://www.waveshare.com/esp32-c6-geek.htm)

SideEye is a tiny USB-powered monitor that gives your Linux rig the "SideEye" by displaying real-time system stats on a dedicated LCD. It consists of a lightweight Rust host binary and custom ESP32-C6 firmware.

---

## :gear: System Architecture

1.  **Host (The Sender):** A Rust-based daemon that gathers system information (Hostname, LAN IP, and MAC Address) and streams it over USB Serial.
2.  **Firmware (The Receiver):** Arduino-based firmware running on the ESP32-C6-GEEK that parses the data and renders a polished UI on the built-in 1.14" LCD.

---

## :stopwatch: Quick Start

### 1. Flash the Firmware
Connect your **Waveshare ESP32-C6-GEEK** and flash the receiver code using PlatformIO.

```bash
cd firmware
pio run -t upload
```

### 2. Run the Host Binary
Ensure you have the Rust toolchain installed. The host will automatically attempt to detect the ESP32 on your serial ports.

```bash
cd host
cargo run --release
```

---

## :package: Components

### Part A: Rust Host
- **Auto-detection:** Automatically finds the ESP32-C6 by VID/PID (0x303A).
- **Efficient Gathering:** Uses `sysinfo` and `local-ip-address` to minimize resource usage.
- **Payload:** Sends a pipe-delimited string `HOSTNAME|IP|MAC\n` every 5 seconds.

### Part B: ESP32-C6 Firmware
- **Display:** Utilizes `Arduino_GFX` for high-performance rendering.
- **USB CDC:** Works directly over the USB-C/USB-A port without needing an external FTDI adapter.
- **Parsing:** Robust string parsing to handle real-time serial updates.

---

## :balance_scale: License

This project is licensed under the [Apache License 2.0](./LICENSE).

---

## :pencil: Author

This project was started in 2026 by [Nicholas Wilde](https://github.com/nicholaswilde/).