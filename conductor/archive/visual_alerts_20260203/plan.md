# Implementation Plan: Visual Alerts & Threshold Management

## Phase 1: Host Threshold Logic [checkpoint: 9fe1129]
- [x] Task: Host - Update `Config` struct with threshold settings. 439968b
- [x] Task: Host - Implement alert level calculation logic in `update_and_get_stats`. d28857b
- [x] Task: Host - Update `SystemStats` message to include `alert_level`. d28857b
- [x] Task: Conductor - User Manual Verification 'Host Logic' (Confirmed)

## Phase 2: Firmware Alert Rendering [checkpoint: da9c323]
- [x] Task: Firmware - Update `drawBanner` to support dynamic background colors. a0be881
- [x] Task: Firmware - Implement alert level handling in `handleJson`. a0be881
- [x] Task: Firmware - Add "Alert Priority" UI logic (auto-switching to alerting page). a0be881
- [x] Task: Conductor - User Manual Verification 'Firmware UI' (Confirmed)

## Phase 3: Testing & Refinement
- [x] Task: Integration - Verify end-to-end alert triggering via manual usage spikes. d28857b
- [x] Task: Refinement - Implement smooth banner color transitions or flashing logic. 55bdc9f
- [x] Task: Conductor - Update README.md with threshold configuration details. 55bdc9f
