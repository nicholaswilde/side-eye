# Product Definition

## Vision
SideEye is a minimalist, USB-powered hardware monitor designed to provide Linux users with immediate, "at-a-glance" system identity and network status. It eliminates the need to run terminal commands for basic connectivity checks by offloading this display to a dedicated peripheral.

## Target Audience
- **Linux Desktop Users:** For aesthetic desk setups and quick status checks.
- **System Administrators:** For headless server racks where identifying a machine's IP address or MAC address physically is valuable.
- **Embedded Hobbyists:** As a reference for Rust-to-Arduino USB serial communication.

## Core Features
- **Plug-and-Play Hardware:** Utilizes the Waveshare ESP32-C6-GEEK with a built-in 1.14" LCD.
- **Wi-Fi Configuration:** Integrated `WiFiManager` for user-friendly network setup via a unique captive portal SSID (`SideEye-XXXXXX`).
- **SD Card Synchronization:** Synchronize local host directories with the device's integrated SD card over USB serial.
- **Linux System Integration:** Native systemd service for automatic startup and lifecycle management.
- **Permission Management:** Bundled udev rules ensure non-root access and stable `/dev/side-eye` symlinks.
- **Multi-Distro Packaging:** Officially supported `.deb` (Debian/Ubuntu) and `.rpm` (Fedora/RHEL) packages.
- **Cross-Platform Host Agent:** A lightweight Rust binary (`side-eye-host`) that runs efficiently in the background.
- **Multi-Device Support:** Manage connections to multiple SideEye devices simultaneously.
- **Proactive Visual Alerts:** Configurable thresholds for CPU and RAM that trigger color-coded banner alerts (Yellow/Red) and auto-switch to the relevant dashboard page.
- **Real-Time Telemetry:** Syncs rich system stats (CPU %, RAM bar, Disk usage, Uptime, Thermal info, GPU Load) every 1 second in addition to static identity info.
- **Historical Trends:** Dedicated network page with real-time sparkline graphs for tracking download and upload throughput over time.
- **Integrated Release Workflow:** Unified GitHub Releases containing host binaries and zipped firmware artifacts for streamlined updates.
- **Intelligent UI State:** Clean "Waiting for Host" mode when idle, automatically transitioning to a detailed dashboard upon connection.
- **Orientation Support:** Physical button toggles screen rotation by 180 degrees to support any USB port orientation.
- **USB CDC Communication:** Uses native USB serial for power and data, requiring no extra cables or HDMI ports.

## Success Metrics
- **Reliability:** The device auto-reconnects if the host restarts or the USB is toggled.
- **Efficiency:** The host agent consumes negligible CPU (< 0.1%) and RAM.
- **Legibility:** The display provides clear, color-coded text readable from a normal sitting distance.
