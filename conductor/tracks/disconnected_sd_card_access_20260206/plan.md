# Implementation Plan: Disconnected SD Card Access [checkpoint: 7a5c36c]

## Phase 1: Navigation Logic Update
- [x] Task: Update `InputHandler` unit tests to verify button navigation while disconnected
    - [x] Add test case for button click when `state.connected` is false
    - [x] Assert that `currentPage` increments even in disconnected state
- [x] Task: Implement navigation logic in `InputHandler`
    - [x] Modify `InputHandler::update` to allow page cycling regardless of connection status
    - [x] Ensure "Waiting..." indicator remains visible or accessible via cycle
- [x] Task: Conductor - User Manual Verification 'Navigation Logic Update' (Protocol in workflow.md)

## Phase 2: SD Page Telemetry & Status
- [x] Task: Update `DisplayManager` unit tests for disconnected states
    - [x] Add test for `drawSDPage` when `state.connected` is false
    - [x] Verify "Offline" or "Disconnected" status text is rendered
- [x] Task: Implement local SD space calculation
    - [x] Update `DisplayManager::drawSDPage` to use `SD.totalBytes()` and `SD.usedBytes()` directly
    - [x] Ensure calculations are efficient and do not block the UI
- [x] Task: Implement disconnected status UI
    - [x] Update status rendering to show "Disconnected" when `state.connected` is false
- [x] Task: Conductor - User Manual Verification 'SD Page Telemetry & Status' (Protocol in workflow.md)

## Phase 3: Final Integration & Coverage
- [x] Task: Verify firmware stability and coverage
    - [x] Run `task firmware:test`
    - [x] Run `task firmware:coverage` and ensure >80% coverage
- [x] Task: Update documentation
    - [x] Update `README.md` or `docs/` if button behavior changes are user-facing
- [x] Task: Conductor - User Manual Verification 'Final Integration & Coverage' (Protocol in workflow.md)