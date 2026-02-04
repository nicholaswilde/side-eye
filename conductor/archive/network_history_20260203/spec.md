# Specification: Network Activity History & Sparklines

## Context
SideEye currently displays instantaneous network upload and download speeds. However, users cannot see trends or recent activity spikes without a historical view. Adding a dedicated Network page with sparkline graphs will provide better insight into network usage over time.

## Goals
1.  **Historical Data:** Maintain a short history of network throughput on the device.
2.  **Visual Trends:** Render sparkline graphs for both Upload and Download speeds.
3.  **Dedicated View:** Add a new "Network" page to the paged UI.

## Detailed Requirements

### Host Agent (Rust)
- No changes required to the protocol; the host already sends `net_up` and `net_down` in the `Stats` message.

### Firmware (C++)
- **Data Management:**
    - Implement a circular buffer (size ~40-60 points) to store recent `net_up` and `net_down` values.
    - Update history every time a `Stats` message is received.
- **New Page:**
    - Add `PAGE_NETWORK` to the `Page` enum.
    - Display current Up/Down speeds in text.
- **UI Component:**
    - Create a `drawSparkline` function in `DisplayManager`.
    - The sparkline should auto-scale its Y-axis based on the maximum value in the current buffer.
    - Render two sparklines: one for Download (Green) and one for Upload (Mauve/Blue).

## Acceptance Criteria
- A new "Network" page appears in the rotation.
- Sparklines update in real-time as data arrives.
- Graphs correctly scale when a high-bandwidth transfer starts or stops.
