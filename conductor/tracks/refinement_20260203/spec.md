# Specification: Refinement - Core Streaming, UI Polish, and Wi-Fi Integration

## Context
The project currently has a basic scaffold for the Host (Rust) and Firmware (C++/Arduino). This track focuses on moving from a scaffold to a functional MVP that aligns with the "SideEye" product vision and "FrameFi" infrastructure standards.

## Goals
1.  **Robust Host Application:** The Rust host must reliably detect the ESP32-C6, handle disconnections gracefully, and stream data efficiently.
2.  **Polished Firmware UI:** The display must render information clearly, using color coding and following the minimalist design guidelines.
3.  **Wi-Fi Connectivity:** The firmware must implement `WiFiManager` to allow user-friendly network configuration, mirroring the user experience of `frame-fi`.
4.  **Hardware Alignment:** Ensure pin definitions match the specific Waveshare ESP32-C6-GEEK hardware.

## Detailed Requirements

### Host (Rust)
-   **Auto-detection:** Refine the `serialport` logic to reliably identify the Waveshare ESP32-C6 (VID `0x303A` is standard, but PID might vary or need fallback).
-   **Error Handling:** Implement a retry loop. If the serial port is lost, the application should pause and attempt to reconnect every 2-5 seconds without crashing.
-   **CLI Args:** Add support for `--verbose` flag for debugging and a `--dry-run` flag to print to stdout without sending to serial.

### Firmware (C++)
-   **Dependencies:** Add `tzapu/WiFiManager` to `platformio.ini`.
-   **Startup Flow:**
    1.  Initialize Serial (USB CDC).
    2.  Initialize Display (Show splash screen).
    3.  Check for saved Wi-Fi credentials.
        -   If found, connect.
        -   If not found or connection fails, launch AP mode (Captive Portal) named "SideEye-Setup".
    4.  Once connected (or skipped), enter main loop.
-   **Main Loop:**
    -   Listen for Serial data (`HOSTNAME|IP|MAC`).
    -   Update display with received data.
    -   *Extension:* If Wi-Fi is connected, the device could optionally display its *own* IP as well (useful for OTA updates later), but primary focus is displaying the Host's info.
-   **Display Layout:**
    -   Header: "SideEye" (Cyan)
    -   Divider Line
    -   Label: "Host:" -> Value: <Hostname> (White)
    -   Label: "IP:" -> Value: <IP> (Green)
    -   Label: "MAC:" -> Value: <MAC> (Gray/Small)
    -   Footer: WiFi Status Icon (connected/disconnected)

## Non-Functional Requirements
-   **Latency:** Display updates should feel instant (< 100ms processing).
-   **Stability:** No memory leaks in the firmware (Arduino `String` usage should be minimized or managed carefully).
