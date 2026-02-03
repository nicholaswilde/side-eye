# Specification: Feature - Expanded System Telemetry

## Context
The current 1.14" display is underutilized, showing only static IP/MAC info. Users want real-time insight into their system's load to spot issues (e.g., runaway processes, memory leaks) at a glance without unlocking the screen or SSH-ing in.

## Goals
1.  **Comprehensive Stats:** Monitor CPU Usage (%), Memory Usage (Used/Total), Disk Usage (Root partition), and System Uptime.
2.  **Extensible Protocol:** Move from a fixed pipe-delimited string to a more flexible format (e.g., JSON or Key-Value pairs) to support future expansion.
3.  **Paged UI:** The firmware must cycle through multiple "pages" of data to fit everything legibly on the small screen.

## Detailed Requirements

### Host (Data Gathering)
-   Use `sysinfo` crate capabilities:
    -   `RefreshKind::new().with_cpu().with_memory()`
    -   Calculate CPU usage averaged across cores.
    -   Get RAM used vs. total.
    -   Get Root `/` disk usage.
    -   Get Uptime (seconds).

### Communication Protocol
-   **Old:** `HOSTNAME|IP|MAC`
-   **New Proposal (JSON):**
    ```json
    {
      "host": "my-server",
      "ip": "192.168.1.50",
      "mac": "AA:BB:CC...",
      "cpu": 12.5,
      "ram": {"u": 4096, "t": 16384},
      "dsk": 45,
      "up": 123456
    }
    ```
-   *Alternative (Compact Pipe):* `HOSTNAME|IP|MAC|CPU%|RAM_USED|RAM_TOTAL|DISK%|UPTIME`
-   *Decision:* JSON is preferred for flexibility if firmware parser overhead is acceptable. Otherwise, stick to Pipe with defined index positions. *Let's stick to Pipe for efficiency on ESP32-C6 unless specific JSON need arises.*
-   **Format:** `HOSTNAME|IP|MAC|CPU_PCT|RAM_PCT|DISK_PCT|UPTIME_STR`

### Firmware (UI)
-   **State Machine:** Implement a `Page` enum: `Identity`, `Resources`, `Status`.
-   **Auto-Cycle:** Automatically switch pages every 5-10 seconds (configurable).
-   **Visuals:**
    -   CPU/RAM/Disk: Use horizontal progress bars.
        -   Green (< 50%), Yellow (< 80%), Red (> 80%).

## Non-Functional Requirements
-   **Performance:** Host CPU usage must remain < 0.5% even with extra polling.
-   **Smoothness:** Screen updates should be double-buffered to prevent flickering.
