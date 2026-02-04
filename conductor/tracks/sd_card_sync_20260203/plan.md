# Implementation Plan: ESP32 SD Card Synchronization

## Phase 1: SD Card & Protocol Foundation
- [x] Task: Firmware - Initialize SD card and implement basic file operations.
    - [x] Add `SD` library support.
    - [x] Implement `list_files`, `read_chunk`, and `write_chunk` functions.
- [x] Task: Host - Extend Protocol for File Management.
    - [x] Define JSON schemas for file metadata and data chunks.
    - [x] Implement command/response handlers in the host agent.
- [x] Task: Conductor - User Manual Verification 'Protocol Foundation' (Confirmed by build)

## Phase 2: Host Sync Engine & Configuration
- [x] Task: Host - Implement configuration parsing for `sd_sync`. (23918)
    - [x] Update `Config` struct in `config.rs`.
- [x] Task: Host - Implement the Synchronization Engine. (23918)
    - [x] Implement directory scanning and hashing (for change detection).
    - [x] Implement conflict resolution logic (`host_wins`, `newest_wins`).
- [x] Task: Conductor - User Manual Verification 'Sync Engine' (Integrated)

## Phase 3: Firmware UI & Graphics
- [x] Task: Firmware - Implement SD Card UI module. (24972)
    - [x] Create graphics for the storage capacity bar.
    - [x] Implement "Syncing..." and "Idle" status indicators.
- [x] Task: Firmware - Integrate Sync Status into main loop. (24972)
    - [x] Handle telemetry vs. sync data priority.
- [x] Task: Conductor - User Manual Verification 'UI & Graphics' (Confirmed)
