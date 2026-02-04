# Implementation Plan: Visual Alerts & Threshold Management

## Phase 1: Host Threshold Logic
- [x] Task: Host - Update `Config` struct with threshold settings. 439968b
- [ ] Task: Host - Implement alert level calculation logic in `update_and_get_stats`.
- [ ] Task: Host - Update `SystemStats` message to include `alert_level`.
- [ ] Task: Conductor - User Manual Verification 'Host Logic' (Protocol in workflow.md)

## Phase 2: Firmware Alert Rendering
- [ ] Task: Firmware - Update `drawBanner` to support dynamic background colors.
- [ ] Task: Firmware - Implement alert level handling in `handleJson`.
- [ ] Task: Firmware - Add "Alert Priority" UI logic (auto-switching to alerting page).
- [ ] Task: Conductor - User Manual Verification 'Firmware UI' (Protocol in workflow.md)

## Phase 3: Testing & Refinement
- [ ] Task: Integration - Verify end-to-end alert triggering via manual usage spikes.
- [ ] Task: Refinement - Implement smooth banner color transitions or flashing logic.
- [ ] Task: Conductor - Update README.md with threshold configuration details.
