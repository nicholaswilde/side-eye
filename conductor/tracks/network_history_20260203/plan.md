# Implementation Plan: Network Activity History & Sparklines

## Phase 1: Firmware Data Management
- [ ] Task: Firmware - Implement `HistoryBuffer` class or struct for circular data storage.
- [ ] Task: Firmware - Integrate `net_up` and `net_down` history into `SystemState`.
- [ ] Task: Firmware - Update `handleJson` to push new data into buffers.

## Phase 2: UI Implementation
- [ ] Task: Firmware - Add `PAGE_NETWORK` to the state machine and `DisplayManager`.
- [ ] Task: Firmware - Implement `drawSparkline` helper function in `DisplayManager`.
- [ ] Task: Firmware - Implement `drawNetworkPage` using sparklines and current text values.

## Phase 3: Verification & Polish
- [ ] Task: Integration - Verify graph scaling with different traffic loads.
- [ ] Task: Refinement - Add "Up/Down" icons or labels to the sparklines.
- [ ] Task: Conductor - Update README.md with the new Network history feature.
