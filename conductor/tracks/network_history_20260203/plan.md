# Implementation Plan: Network Activity History & Sparklines

## Phase 1: Firmware Data Management
- [x] Task: Firmware - Implement `HistoryBuffer` class or struct for circular data storage. 4b5f27e
- [x] Task: Firmware - Integrate `net_up` and `net_down` history into `SystemState`. 0de081d
- [x] Task: Firmware - Update `handleJson` to push new data into buffers. 569e3e4

## Phase 2: UI Implementation
- [x] Task: Firmware - Add `PAGE_NETWORK` to the state machine and `DisplayManager`. c4980fb
- [x] Task: Firmware - Implement `drawSparkline` helper function in `DisplayManager`. eb9a295
- [x] Task: Firmware - Implement `drawNetworkPage` using sparklines and current text values. ea75e88

## Phase 3: Verification & Polish
- [x] Task: Integration - Verify graph scaling with different traffic loads. 30d4888
- [x] Task: Refinement - Add "Up/Down" icons or labels to the sparklines. a4be706
- [x] Task: Conductor - Update README.md with the new Network history feature. a4be706
