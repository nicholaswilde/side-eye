# Product Guidelines

## Design Principles
- **Minimalism First:** The 1.14" display is small (240x135). Information must be dense but legible. Avoid unnecessary borders or decorations that reduce text size.
- **Functionality over Form:** The primary goal is readability. Color coding should be used semantically (e.g., Green for IP, Cyan for Hostname) rather than just for decoration.
- **Set It and Forget It:** The host application should require zero user interaction after the initial launch. It should be invisible unless an error occurs.

## User Experience (UX)
- **Plug-and-Play:** The user should not need to configure serial ports manually in 90% of cases. Auto-detection is a critical UX requirement.
- **Easy Connectivity:** Wi-Fi setup should be seamless via a dedicated AP mode if credentials are missing, avoiding the need for hardcoded secrets.
- **Resilience:** If the device is unplugged, the host software should wait patiently rather than crashing. When plugged back in, it should resume immediately.
- **Visual Feedback:** The device should indicate a "Waiting for Host..." state on the screen if it has power but is not receiving data, distinguishing "no power" from "no data."
  - **On-Screen Status:** Small visual indicators (e.g., connection status dots) provide real-time feedback without cluttering the UI.

## Coding Standards
- **Rust (Host):** Follow idiomatic Rust patterns (using `cargo clippy`). Error handling should be robust using `anyhow` or `thiserror`.
- **C++ (Firmware):** Maintain clean separation between hardware drivers (display/serial) and business logic. Use descriptive variable names and standard Arduino/PlatformIO directory structures.
