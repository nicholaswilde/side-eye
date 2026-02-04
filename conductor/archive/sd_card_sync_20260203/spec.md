# Specification: ESP32 SD Card Synchronization

## Overview
This feature enables the SideEye device to manage files on its integrated SD card. Users can specify a local directory on their host machine, and the host agent will synchronize that directory with the SD card on the ESP32 via the USB Serial connection.

## Functional Requirements

### Host Agent (Rust)
- **Configuration:** Add `sd_sync` section to the config file:
    - `local_path`: Path to the directory on the host to sync.
    - `sync_mode`: `one_way` (Host -> ESP32) or `bidirectional`.
    - `conflict_resolution`: `host_wins` or `newest_wins`.
- **Sync Engine:** 
    - Recursively scan the local directory.
    - Compare with files on the ESP32 SD card.
    - Transfer missing or updated files in chunks over the serial protocol.
    - Handle deletions if `sync_mode` is `one_way`.

### Firmware (C++)
- **SD Card Support:** Initialize and manage the SD card using the standard Arduino `SD` library.
- **Protocol Expansion:**
    - New JSON message types for file listing, chunk transfer, and sync status.
- **UI Display:**
    - Dedicated SD card status graphic on the LCD.
    - Progress bar or indicator for active synchronization.
    - "Storage Capacity" bar showing Used vs. Total space.
    - "Status" label (Idle, Syncing, Error).

## Non-Functional Requirements
- **Reliability:** Chunk-based transfers with simple checksums to ensure data integrity over serial.
- **Performance:** Synchronize only changed files to minimize data transfer over the relatively slow serial link.
- **Concurrency:** Telemetry updates should continue (perhaps at a lower frequency) during active synchronization.

## Acceptance Criteria
- Files added to the host directory appear on the ESP32 SD card after the next sync.
- In `bidirectional` mode, files modified on the ESP32 are synced back to the host (if the protocol allows for uploading).
- The device display correctly reflects the storage usage and sync state.
- Sync conflicts are resolved based on the user's chosen configuration.

## Out of Scope
- Support for very large files (> 50MB) due to serial bandwidth constraints.
- Real-time "Watch" mode (initial sync will be on startup or periodic).
