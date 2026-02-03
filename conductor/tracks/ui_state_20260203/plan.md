# Implementation Plan: UI State & Status Display

## Phase 1: Logic & UI Implementation
- [x] Task: Firmware - Update `SystemState` to include connection status. b3baeb9
    - [x] Add `bool connected = false;` to `SystemState`.
- [x] Task: Firmware - Refactor `drawStaticUI` for conditional rendering. b3baeb9
    - [x] Add "Status:" label.
    - [x] Wrap Host/IP/CPU/RAM labels in an `if (state.connected)` block.
- [x] Task: Firmware - Update `updateDynamicValues` for status display. b3baeb9
    - [x] Draw "Waiting for Host" vs "Connected" based on state.
    - [x] Trigger `needs_static_draw = true` on the first successful data reception to reveal all labels.

## Phase 2: Verification
- [x] Task: Firmware - Compile and visual check. 8a738fe
    - [x] Verify that labels are hidden initially.
    - [x] Verify that "Status: Waiting for Host" is shown.
    - [x] Verify that all labels appear and "Status: Connected" is shown once the host connects.
- [x] Task: Conductor - User Manual Verification 'UI State Management' (Protocol in workflow.md) 8a738fe
