# Implementation Plan: Feature - Expanded System Telemetry

## Phases

### Phase 1: Host Logic (Rust)
- [ ] Update `Host` struct to hold `sysinfo::System`.
- [ ] Implement `update_stats()` method to refresh CPU/RAM/Disk.
- [ ] Format `uptime` into a human-readable string (e.g., "2d 4h").
- [ ] Update `SerialSender` to construct the extended payload string.

### Phase 2: Firmware Logic (C++)
- [ ] Update `parseData()` function to handle new fields.
    - [ ] Handle variable length payloads (backward compatibility check).
- [ ] Create `drawResourcesPage()` function (CPU/RAM bars).
- [ ] Create `drawStatusPage()` function (Disk/Uptime).

### Phase 3: Integration
- [ ] Implement the Page Cycle Timer in the main loop.
- [ ] Test sync between Host sending and Firmware displaying.

### Phase 4: Refinement
- [ ] Tune colors and layout for readability.
- [ ] Verify memory usage on ESP32 (prevent heap fragmentation).
