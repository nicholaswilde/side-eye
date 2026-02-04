# Implementation Plan: Feature - Expanded System Telemetry

## Phases

### Phase 1: Host Logic (Rust)
- [x] Update `Host` struct to hold `sysinfo::System`.
- [x] Implement `update_stats()` method to refresh CPU/RAM/Disk.
- [x] Format `uptime` into a human-readable string (handled by firmware for now, or already in seconds).
- [x] Update `SerialSender` to construct the extended payload string (JSON used instead of Pipe).

### Phase 2: Firmware Logic (C++)
- [x] Update `handleJson()` function to handle new fields.
- [x] Implement Paged UI logic (State machine).
- [x] Create `drawIdentityPage()` function.
- [x] Create `drawResourcesPage()` function (CPU/RAM bars).
- [x] Create `drawStatusPage()` function (Disk/Uptime).

### Phase 3: Integration
- [x] Implement the Page Cycle Timer in the main loop.
- [x] Test sync between Host sending and Firmware displaying.

### Phase 4: Refinement
- [x] Tune colors and layout for readability.
- [x] Verify memory usage on ESP32 (prevent heap fragmentation).
