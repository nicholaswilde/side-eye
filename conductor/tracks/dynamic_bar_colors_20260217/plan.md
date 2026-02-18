# Implementation Plan - Dynamic Bar Graph Colors

This plan outlines the steps to make the SD Card storage bar responsive to usage percentage and synchronization state, while ensuring all other resource bars follow a consistent color logic.

## Phase 1: SD Card UI Logic
- [ ] Task: Implement SD Color Selection
    - [ ] Update `DisplayManager::drawSDPage` to calculate the bar color based on `sd_p` (percentage) and `sd_sync_status`.
    - [ ] Use `active_theme.blue` during sync, else standard Green/Yellow/Red tiers.
- [ ] Task: Verification via Native Tests
    - [ ] Update `test_display_draw_smoke` or create a new test in `test_all.cpp` to verify SD bar color states.
- [ ] Task: Conductor - User Manual Verification 'Phase 1: SD Card UI Logic' (Protocol in workflow.md)

## Phase 2: Unification & Refinement
- [ ] Task: Audit & Unify All Resource Bars
    - [ ] Verify `drawResourcesPage` (CPU/RAM) and `drawStatusPage` (Disk) use the exact same logic and thresholds (50/80).
    - [ ] Update `drawThermalPage` (GPU) to also follow the unified bar logic.
- [ ] Task: Cleanup Theme Color Access
    - [ ] Ensure no legacy `CATPPUCCIN_*` constants remain in the bar drawing logic.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Unification & Refinement' (Protocol in workflow.md)
