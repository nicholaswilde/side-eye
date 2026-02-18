# Implementation Plan - Global Status Indicators

This plan outlines the steps to implement a global status bar in the UI footer, providing real-time visual feedback for all major subsystems.

## Phase 1: Core Assets & Drawing Logic [checkpoint: 580be9b]
- [x] Task: Define Icon Bitmaps (b08a82d)
    - [x] Define minimalist monochrome bitmaps (e.g., 8x8 or 10x10) for WiFi, MQTT, Host, SD, and BLE symbols.
- [x] Task: Implement Icon Rendering Utility (db66d1f, 6223b65)
    - [x] Write failing tests in `test_all.cpp` for a new `drawStatusIcon` method.
    - [x] Implement `DisplayManager::drawStatusIcon(int x, int y, const uint8_t* bitmap, uint16_t color)` using `gfx.drawBitmap`.
- [x] Task: Conductor - User Manual Verification 'Phase 1: Core Assets & Drawing Logic' (Protocol in workflow.md)

## Phase 2: State Integration & Global UI [checkpoint: 9cdeb6b]
- [x] Task: Implement Status Bar Mapping (f0dd14d)
    - [x] Write failing tests verifying that different `SystemState` values (e.g., `mqtt_connected`, `sd_sync_status`) map to correct icon colors.
    - [x] Implement `DisplayManager::drawStatusBar(const SystemState& state)` to render the full row of indicators.
- [x] Task: Integrate into Dashboard Cycle (4a33040)
    - [x] Update `DisplayManager::drawStaticUI` to reserve footer space and draw the initial status bar.
    - [x] Update `DisplayManager::updateDynamicValues` to refresh icons during every update cycle.
- [x] Task: Conductor - User Manual Verification 'Phase 2: State Integration & Global UI' (Protocol in workflow.md)

## Phase 3: Layout Polish & Verification
- [ ] Task: Layout Refinement
    - [ ] Fine-tune the positioning of the status bar to ensure perfect alignment opposite the version string.
    - [ ] Verify visibility and layout integrity across all 6 dashboard pages.
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Layout Polish & Verification' (Protocol in workflow.md)
