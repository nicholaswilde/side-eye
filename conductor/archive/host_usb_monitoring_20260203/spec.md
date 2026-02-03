# Specification: Host - Enhanced USB Port Monitoring & Configuration

## Context
The current Rust host application (`side-eye-host`) is designed to auto-detect a single ESP32 device based on a hardcoded VID/PID or connect to a single port specified via CLI. The user requires the ability to monitor *all* USB ports on the system or a specific list of ports defined via configuration. This enables support for multiple SideEye devices or more granular control over which devices are targeted.

## Goals
1.  **Multi-Port Support:** The host must be able to manage connections to multiple SideEye devices simultaneously.
2.  **Configurable Monitoring:** Users can specify a list of ports to monitor via a configuration file or CLI arguments.
3.  **"Monitor All" Mode:** A mode to automatically detect and connect to any relevant device on any USB port.
4.  **Hot-Plug Support:** Ideally, the system should detect when new devices are plugged in (especially in "Monitor All" mode).

## Detailed Requirements

### Configuration
-   **Config File:** Support a configuration file (e.g., `side-eye.toml` or `config.json`) to define:
    -   `ports`: A list of specific device paths (e.g., `["/dev/ttyACM0", "/dev/ttyUSB1"]`).
    -   `monitor_all`: Boolean flag. If true, ignores the `ports` list (or uses it as an allowlist) and scans for known VIDs.
    -   `target_vids`: List of VIDs to scan for in "monitor all" mode (defaulting to Espressif 0x303A).
-   **CLI Overrides:** CLI arguments should override config file settings.
    -   `--ports /dev/ttyACM0,/dev/ttyACM1`
    -   `--monitor-all`

### Concurrency Model
-   The host application needs to spawn a separate thread or async task for each active connection to ensure that one blocked device does not stop updates to others.
-   Shared state (system stats) should be calculated once per interval and broadcast to all active connections.

### Hot-Plugging (Monitor All)
-   In "Monitor All" mode, the main thread should periodically (e.g., every 5-10s) scan `serialport::available_ports()` for new devices that match the target VID and are not currently connected.
-   If a connection drops, clean up the task and allow it to be re-detected in the next scan.

## Non-Functional Requirements
-   **Resource Usage:** Monitoring multiple ports shouldn't significantly increase CPU usage.
-   **Logging:** Clear logs indicating which ports are connected, disconnected, or failed.
