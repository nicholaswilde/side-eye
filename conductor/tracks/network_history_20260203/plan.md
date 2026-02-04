# Implementation Plan: Network Activity History & Sparklines

## Phase 1: Firmware Data Management
- [x] Task: Firmware - Implement `HistoryBuffer` class or struct for circular data storage. 4b5f27e
- [x] Task: Firmware - Integrate `net_up` and `net_down` history into `SystemState`. 0de081d
- [x] Task: Firmware - Update `handleJson` to push new data into buffers. 569e3e4

## Phase 2: UI Implementation
- [ ] Task: Firmware - Add `PAGE_NETWORK` to the state machine and `DisplayManager`.
- [ ] Task: Firmware - Implement `drawSparkline` helper function in `DisplayManager`.
- [ ] Task: Firmware - Implement `drawNetworkPage` using sparklines and current text values.

## Phase 3: Verification & Polish
- [ ] Task: Integration - Verify graph scaling with different traffic loads.
- [ ] Task: Refinement - Add "Up/Down" icons or labels to the sparklines.
- [ ] Task: Conductor - Update README.md with the new Network history feature.
