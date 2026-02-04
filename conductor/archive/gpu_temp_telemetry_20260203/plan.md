# Implementation Plan: GPU & Temperature Telemetry

## Phase 1: Host Gathering Logic
- [x] Task: Host - Implement thermal sensor polling using `sysinfo`.
- [x] Task: Host - Implement basic GPU usage gathering (NVIDIA prioritized).
- [x] Task: Host - Update `SystemStats` payload with new fields.
- [x] Task: Conductor - User Manual Verification 'Host Logic' (Confirmed)

## Phase 2: Firmware Page Integration
- [x] Task: Firmware - Add `PAGE_THERMAL` to state machine.
- [x] Task: Firmware - Implement `drawThermalPage` with temp and GPU bars.
- [x] Task: Firmware - Update `handleJson` to parse thermal/gpu data.
- [x] Task: Conductor - User Manual Verification 'Firmware UI' (Confirmed)

## Phase 3: Final Verification
- [x] Task: Integration - Test end-to-end telemetry with heavy load (Confirmed).
- [x] Task: Conductor - Update README.md with thermal monitoring features. (Handled in documentation sync)
