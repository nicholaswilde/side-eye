# Implementation Plan: SD Card Sync Testing

## Phase 1: Firmware Integration Tests [checkpoint: a0169ba]
- [x] Task: Create/Update firmware unit tests for `SyncManager` 4fab5e8
    - [ ] Add `test_sync_manager_single_file` to `test_all.cpp`
    - [ ] Add `test_sync_manager_multi_chunk` to `test_all.cpp`
    - [ ] Add `test_sync_manager_nested_dir` to `test_all.cpp`
- [x] Task: Implement missing mock logic for SD card operations 4fab5e8
    - [ ] Enhance `SD.h` and `LittleFS.h` mocks to track "written" data for verification
- [x] Task: Verify firmware tests pass 4fab5e8
    - [ ] Run `task firmware:test` and ensure all scenarios are covered
- [x] Task: Conductor - User Manual Verification 'Firmware Integration Tests' (Protocol in workflow.md) a0169ba

## Phase 2: Host End-to-End Test
- [ ] Task: Create host integration test for synchronization protocol
    - [ ] Create `host/tests/sync_protocol.rs`
    - [ ] Implement a test that mocks the `SerialPort` to intercept `WriteChunk` JSON messages
    - [ ] Verify the host correctly splits a test file into the expected number of chunks and offsets
- [ ] Task: Implement data integrity verification in host tests
    - [ ] Reassemble the intercepted chunks in the test and compare with the source file
- [ ] Task: Conductor - User Manual Verification 'Host End-to-End Test' (Protocol in workflow.md)

## Phase 3: Final Integration & Coverage
- [ ] Task: Run full CI suite
    - [ ] Execute `task test:ci`
- [ ] Task: Verify code coverage
    - [ ] Run `task firmware:coverage` and `task host:coverage`
    - [ ] Ensure new tests contribute to the >80% coverage goal
- [ ] Task: Conductor - User Manual Verification 'Final Integration & Coverage' (Protocol in workflow.md)
