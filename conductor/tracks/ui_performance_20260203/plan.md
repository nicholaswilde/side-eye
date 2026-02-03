# Implementation Plan: UI Performance & Flicker Reduction

## Phase 1: Logic Refactoring
- [x] Task: Firmware - Refactor `updateDisplay` into `drawStaticUI` and `updateDynamicValues`. b3baeb9
    - [x] `drawStaticUI()`: Draws the banner, background, and labels (Host:, IP:, CPU:, RAM:, Uptime:).
    - [x] `updateDynamicValues()`: Updates only the text values and the progress bar.
- [x] Task: Firmware - Implement partial clearing for text values. b3baeb9
    - [x] Before drawing a new value, draw a filled rectangle of `CATPPUCCIN_BASE` over the old value's coordinates.

## Phase 2: Optimization
- [x] Task: Firmware - Implement "Last Known State" comparisons. 7dff7a2
    - [x] Only call `updateDynamicValues` if the received data differs from the current state.
- [x] Task: Firmware - Handle orientation changes correctly. 7dff7a2
    - [x] Ensure `drawStaticUI()` is called after a rotation toggle.

## Phase 3: Verification
- [ ] Task: Firmware - Compile and visual check.
    - [ ] Verify that the flicker is gone and values update smoothly.
- [ ] Task: Conductor - User Manual Verification 'Flicker Reduction' (Protocol in workflow.md)
