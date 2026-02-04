# Specification: Visual Alerts & Threshold Management

## Context
SideEye currently displays information but does not actively alert the user to potential issues. Adding threshold-based visual alerts (e.g., changing the banner to Red when CPU usage is high) transforms the device from a passive display into a proactive monitoring tool.

## Goals
1.  **Configurable Thresholds:** Allow users to define warning and critical levels for CPU and RAM in the host configuration.
2.  **Visual Feedback:** Update the firmware to reflect alert states through UI color changes and optional backlight flashing.
3.  **Proactive Monitoring:** Ensure alerts are triggered immediately when a threshold is crossed.

## Detailed Requirements

### Host Agent (Rust)
- **Configuration:** Add `thresholds` section to `Config`:
    - `cpu_warning` (default 70), `cpu_critical` (default 90).
    - `ram_warning` (default 80), `ram_critical` (default 95).
- **Logic:** Compare real-time stats against these values.
- **Protocol:** Add an `alert_level` field to the `Stats` message:
    - `0`: Normal (Mocha/Mauve)
    - `1`: Warning (Yellow)
    - `2`: Critical (Red)

### Firmware (C++)
- **State Management:** Track the current `alert_level` for the system.
- **UI Updates:** 
    - Change `drawBanner` background color based on level.
    - If `alert_level == 2` (Critical), optionally flash the banner or backlight.
    - Prioritize displaying the page with the alerting stat (e.g., jump to Resources page on alert).

## Acceptance Criteria
- Setting `cpu_critical = 10` in config causes the device banner to turn Red immediately.
- The UI returns to normal colors once usage drops below warning levels.
- Configuration correctly supports overrides via environment variables.
