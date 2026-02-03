# Product Definition

## Vision
SideEye is a minimalist, USB-powered hardware monitor designed to provide Linux users with immediate, "at-a-glance" system identity and network status. It eliminates the need to run terminal commands for basic connectivity checks by offloading this display to a dedicated peripheral.

## Target Audience
- **Linux Desktop Users:** For aesthetic desk setups and quick status checks.
- **System Administrators:** For headless server racks where identifying a machine's IP address or MAC address physically is valuable.
- **Embedded Hobbyists:** As a reference for Rust-to-Arduino USB serial communication.

## Core Features
- **Plug-and-Play Hardware:** Utilizes the Waveshare ESP32-C6-GEEK with a built-in 1.14" LCD.
- **Wi-Fi Configuration:** Integrated `WiFiManager` for user-friendly network setup via a captive portal ("SideEye-Setup").
- **Cross-Platform Host Agent:** A lightweight Rust binary (`side-eye-host`) that runs efficiently in the background.
- **Multi-Device Support:** Manage connections to multiple SideEye devices simultaneously.
- **Real-Time Updates:** Syncs Hostname, LAN IP, and MAC address every 5 seconds.
- **Zero-Config Discovery:** Automatic detection and hot-plug support for all compatible devices on any USB port.
- **USB CDC Communication:** Uses native USB serial for power and data, requiring no extra cables or HDMI ports.

## Success Metrics
- **Reliability:** The device auto-reconnects if the host restarts or the USB is toggled.
- **Efficiency:** The host agent consumes negligible CPU (< 0.1%) and RAM.
- **Legibility:** The display provides clear, color-coded text readable from a normal sitting distance.
