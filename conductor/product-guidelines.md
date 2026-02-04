# Product Guidelines

## Design Principles
- **Minimalism First:** The 1.14" display is small (240x135). Information must be dense but legible. Avoid unnecessary borders or decorations that reduce text size.
  - **Banner UI:** Use a centered title on a dedicated banner for clear context (e.g., "SIDEEYE MONITOR").
  - **Paged UI:** Use automatic page cycling (e.g., every 5 seconds) to present dense information legibly across multiple screens.
- **Functionality over Form:** The primary goal is readability. Color coding should be used semantically (e.g., Green for IP, Cyan for Hostname) rather than just for decoration.
- **Set It and Forget It:** The host application should require zero user interaction after the initial launch. It should be invisible unless an error occurs.

## User Experience (UX)
- **Plug-and-Play:** The user should not need to configure serial ports manually in 90% of cases. Auto-detection is a critical UX requirement.
- **Multi-Device Management:** The system should handle multiple connected monitors seamlessly, broadcasting updates to all active units.
- **Easy Connectivity:** Wi-Fi setup should be seamless via a dedicated unique AP mode (`SideEye-XXXXXX`) if credentials are missing, avoiding the need for hardcoded secrets and allowing identification of specific devices.
- **Resilience:** If the device is unplugged, the host software should wait patiently rather than crashing. When plugged back in, it should resume immediately.
- **Visual Feedback:** The device should indicate a "Waiting for Host..." state on the screen if it has power but is not receiving data, distinguishing "no power" from "no data."
  - **Contextual UI:** Only display host-specific labels and metrics when a connection is active to maintain a clean aesthetic during idle periods.
  - **On-Screen Status:** Small visual indicators (e.g., connection status dots) provide real-time feedback without cluttering the UI.
- **Proactive Alerts:** The device should actively grab the user's attention when critical thresholds are met.
  - **Visual Priority:** Alerts should override the current page and use high-contrast color changes (e.g., Red flashing) to signal urgency.
  - **Automatic Context:** Automatically switch to the page containing the alerting metric to provide immediate diagnostic information.
- **Power Management:**
  - **Auto-Off:** Automatically turn off the backlight after 1 minute of inactivity (no data or button presses) to save power and screen life.
  - **Interactive Wake:** Ensure the screen wakes immediately on any button press or when the host resumes sending data.
  - **Manual Control:** Provide a hardware toggle (e.g., long-press) for users to override and manage screen power manually.

## Coding Standards
- **Rust (Host):** Follow idiomatic Rust patterns (using `cargo clippy`). Error handling should be robust using `anyhow` or `thiserror`.
- **C++ (Firmware):** Follow a modular, class-based architecture. Use dedicated managers (e.g., `DisplayManager`, `SideEyeNetworkManager`) to encapsulate complex logic and keep `main.cpp` as a lightweight orchestrator. Use descriptive variable names and standard Arduino/PlatformIO directory structures.
