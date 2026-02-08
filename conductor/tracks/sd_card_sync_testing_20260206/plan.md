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

## Phase 2: Host End-to-End Test [checkpoint: ed42900]
- [x] Task: Create host integration test for synchronization protocol 4e6a280
    - [x] Create `host/tests/sync_protocol.rs` 4e6a280
    - [x] Implement a test that mocks the `SerialPort` to intercept `WriteChunk` JSON messages 4e6a280
    - [x] Verify the host correctly splits a test file into the expected number of chunks and offsets 4e6a280
- [x] Task: Implement data integrity verification in host tests 4e6a280
    - [x] Reassemble the intercepted chunks in the test and compare with the source file 4e6a280
- [x] Task: Conductor - User Manual Verification 'Host End-to-End Test' (Protocol in workflow.md) ed42900

## Phase 3: Final Integration & Coverage [checkpoint: 486ebd5]
- [x] Task: Run full CI suite 4329860
    - [x] Execute `task test:ci` 4329860
- [x] Task: Verify code coverage 4329860
    - [x] Run `task firmware:coverage` and `task host:coverage` 4329860
    - [x] Ensure new tests contribute to the >80% coverage goal 4329860
- [x] Task: Conductor - User Manual Verification 'Final Integration & Coverage' (Protocol in workflow.md) 486ebd5
