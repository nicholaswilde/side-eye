# Implementation Plan - Screensaver / Clock Mode

This plan details the steps to implement a host-driven screensaver mode that transforms the SideEye into a desk clock when the PC is idle or locked.

## Phase 1: Host Protocol & Idle Detection
- [ ] Task: Update Communication Protocol
    - [ ] Write failing tests in `host/src/monitor.rs` for new JSON serialization fields (`time`, `date`, `is_idle`).
    - [ ] Update `HostMessage` and `Stats` structs in the Rust host to include these fields.
- [ ] Task: Implement System Time & Idle Logic
    - [ ] Extend the data provider in `host/src/monitor.rs` to fetch current system time/date.
    - [ ] Implement idle detection logic (e.g., checking D-Bus for screen lock state or a configurable timeout).
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Host Protocol & Idle Detection' (Protocol in workflow.md)

## Phase 2: Firmware Clock Page & State Logic
- [ ] Task: Implement Clock UI
    - [ ] Write failing tests in `firmware/test/test_all/test_all.cpp` for `drawClockPage`.
    - [ ] Implement `DisplayManager::drawClockPage` using a large font for time and a smaller font for date.
- [ ] Task: Handle Idle State Transitions
    - [ ] Update `SystemState` in `DisplayManager.h` to include time/date strings and an `is_idle` flag.
    - [ ] Update `main.cpp` logic to switch to the Clock Page when `is_idle` is true or if serial data has timed out (60s).
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Firmware Clock Page & State Logic' (Protocol in workflow.md)

## Phase 3: Power Management & Refinement
- [ ] Task: Implement Dimming Logic
    - [ ] Add logic to dim the backlight during Clock Mode to save power and prevent screen burn-in.
    - [ ] Ensure any button press or host activity restores full brightness.
- [ ] Task: Performance & Sync Polish
    - [ ] Verify that time updates are smooth and do not cause UI flickering.
    - [ ] Ensure the footer status icons remain functional and correctly positioned on the clock page.
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Power Management & Refinement' (Protocol in workflow.md)
