# Implementation Plan: GPU & Temperature Telemetry

## Phase 1: Host Gathering Logic
- [ ] Task: Host - Implement thermal sensor polling using `sysinfo`.
- [ ] Task: Host - Implement basic GPU usage gathering (NVIDIA prioritized).
- [ ] Task: Host - Update `SystemStats` payload with new fields.
- [ ] Task: Conductor - User Manual Verification 'Host Logic' (Protocol in workflow.md)

## Phase 2: Firmware Page Integration
- [ ] Task: Firmware - Add `PAGE_THERMAL` to state machine.
- [ ] Task: Firmware - Implement `drawThermalPage` with temp and GPU bars.
- [ ] Task: Firmware - Update `handleJson` to parse thermal/gpu data.
- [ ] Task: Conductor - User Manual Verification 'Firmware UI' (Protocol in workflow.md)

## Phase 3: Final Verification
- [ ] Task: Integration - Test end-to-end telemetry with heavy load (e.g., stress-ng or gaming).
- [ ] Task: Conductor - Update README.md with thermal monitoring features.
