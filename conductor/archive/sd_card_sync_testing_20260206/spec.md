# Specification: SD Card Sync Testing

## Overview
This track aims to validate the reliability and correctness of the file synchronization feature between the SideEye Host and the ESP32 Firmware. It involves creating a comprehensive test suite that covers both firmware-level logic and host-driven end-to-end scenarios, ensuring that files are correctly written, chunked, and stored on the device's SD card.

## Functional Requirements
1.  **Firmware Integration Tests**:
    *   Verify `SyncManager` can write a complete file to the SD card.
    *   Verify `SyncManager` handles chunked writes correctly (appending data at the correct offsets).
    *   Verify directory creation when writing to nested paths.
    *   Verify file content integrity after write operations.

2.  **Host-Driven End-to-End Tests**:
    *   Implement a new Rust integration test in `host/tests/` that simulates a real sync session.
    *   The test must send file data over the mocked or real serial connection (depending on test mode) using the established JSON protocol (`WriteChunk`).
    *   Verify that the Host correctly chunks large files and sends sequential updates.

## Non-Functional Requirements
- **Test Isolation**: Tests should clean up created files (e.g., `test_sync.txt`) after execution to leave the SD card in a clean state.
- **Mocking**: For host tests running in CI without hardware, the serial connection and SD card responses should be mocked effectively to validate the protocol logic without physical I/O.

## Acceptance Criteria
- [ ] `pio test` on Firmware passes a new test case for:
    - [ ] Single file write.
    - [ ] Multi-chunk write.
    - [ ] Nested directory write.
- [ ] `cargo test` on Host passes a new integration test for the `sync` module that simulates the file transfer protocol.
- [ ] Verified that a large file (requiring multiple chunks) is re-assembled correctly on the target (or mock target).

## Out of Scope
- Performance benchmarking of write speeds.
- Testing SD card hardware failure modes (e.g., card removal during write).
