# Specification: Display Host Network, Identity & System Stats

## Context
The user wants the SideEye device to serve as a dashboard for the host computer it is connected to. The device should display network identity, system health, and activity metrics. Additionally, the device needs to support variable USB port orientations.

## Requirements

### Host Application (`host/`)
1.  **Data Collection:** The Rust host application must identify and monitor:
    *   **Network & Identity (Static/Slow Changing):**
        *   **Hostname:** The network name of the computer.
        *   **IP Address:** The primary local IP address (IPv4).
        *   **MAC Address:** The hardware address of the primary network interface.
        *   **OS Name & Version:** e.g., "Ubuntu 22.04 LTS".
        *   **Current User:** The currently logged-in user.
    *   **System Health (Dynamic):**
        *   **CPU Usage:** Overall percentage and potentially per-core or load average.
        *   **RAM Usage:** Used vs. Total (absolute and percentage).
        *   **Disk Usage:** Used vs. Total for the root partition (or configurable mount points).
    *   **Activity (Dynamic):**
        *   **Network Throughput:** Current Upload and Download speeds (e.g., KB/s).
        *   **System Uptime:** Duration the system has been running.

2.  **Transmission:**
    *   Data should be serialized (e.g., Protobuf/JSON).
    *   **Optimization:** Split into two packet types to save bandwidth:
        *   **Identity Packet:** Sent on connection and very rarely (e.g., once an hour).
        *   **Stats Packet:** Sent frequently (e.g., every 1-2 seconds) for real-time updates.

3.  **Frequency:**
    *   Identity: On Connect.
    *   Stats: Every 1-2 seconds.

### Firmware (`firmware/`)
1.  **Data Parsing:** Handle `IdentityPacket` and `StatsPacket`.
2.  **Display:** Render the data on the screen.
    *   **Layout:**
        *   Header: Hostname / IP.
        *   Body: Grid or list of metrics (CPU bar, RAM bar, Net graph/text).
        *   Footer: Uptime / OS / User.
    *   Use graphical elements (bars/gauges) for percentages where appropriate.
3.  **Hardware Interaction:**
    *   **Orientation Toggle:** Pressing the physical button on the ESP32 shall toggle the screen rotation by 180 degrees. This allows proper viewing regardless of the USB port's orientation.
    *   **Persistence:** The rotation state should ideally be saved to NVS (Non-Volatile Storage) so it persists across reboots (optional but recommended).

## Technical Considerations
-   **Crates:**
    -   `sysinfo`: This is the primary recommendation as it covers CPU, RAM, Disk, Network, OS, User, and Uptime cross-platform.
    -   `local-ip-address`: May still be needed if `sysinfo` network interface parsing is too verbose.
-   **TFT_eSPI / LVGL:** Ensure the display driver or UI library supports software or hardware rotation commands.
