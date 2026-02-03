# Implementation Plan: ESP32 SD Card Synchronization

## Phase 1: SD Card & Protocol Foundation
- [ ] Task: Firmware - Initialize SD card and implement basic file operations.
    - [ ] Add `SD` library support.
    - [ ] Implement `list_files`, `read_chunk`, and `write_chunk` functions.
- [ ] Task: Host - Extend Protocol for File Management.
    - [ ] Define JSON schemas for file metadata and data chunks.
    - [ ] Implement command/response handlers in the host agent.
- [ ] Task: Conductor - User Manual Verification 'Protocol Foundation' (Protocol in workflow.md)

## Phase 2: Host Sync Engine & Configuration
- [ ] Task: Host - Implement configuration parsing for `sd_sync`.
    - [ ] Update `Config` struct in `config.rs`.
- [ ] Task: Host - Implement the Synchronization Engine.
    - [ ] Implement directory scanning and hashing (for change detection).
    - [ ] Implement conflict resolution logic (`host_wins`, `newest_wins`).
- [ ] Task: Conductor - User Manual Verification 'Sync Engine' (Protocol in workflow.md)

## Phase 3: Firmware UI & Graphics
- [ ] Task: Firmware - Implement SD Card UI module.
    - [ ] Create graphics for the storage capacity bar.
    - [ ] Implement "Syncing..." and "Idle" status indicators.
- [ ] Task: Firmware - Integrate Sync Status into main loop.
    - [ ] Handle telemetry vs. sync data priority.
- [ ] Task: Conductor - User Manual Verification 'UI & Graphics' (Protocol in workflow.md)
