# Implementation Plan: Host - Enhanced USB Port Monitoring & Configuration

## Phase 1: Configuration & Argument Parsing [checkpoint: 13ffe59]
- [x] Task: Host - Add `serde` and `toml` (or `json`) dependencies. f80c553
    - [x] Update `Cargo.toml`.
- [x] Task: Host - Define Configuration Structure. 9ced7b7
    - [x] Create a `Config` struct with `ports`, `monitor_all`, and `target_vids`.
- [x] Task: Host - Implement Config Loading. 2f0a34b
    - [x] Logic to load from default path (e.g., `~/.config/side-eye/config.toml`) or path specified by `--config`.
    - [x] Logic to merge CLI args with config file (CLI takes precedence).

## Phase 2: Refactoring for Multi-Device Support
- [x] Task: Host - Refactor `main.rs` loop. bf3c04d
    - [x] Move the "connect and send" logic into a separate function/struct (e.g., `DeviceConnection`).
    - [x] Use `std::sync::mpsc` or `tokio` (if moving to async, though threads are fine for low count) to manage device states.
- [x] Task: Host - Implement "System Stats" Broadcaster. bf3c04d
    - [x] Create a central loop that gathers system stats once every 5 seconds.
    - [x] Broadcast the formatted string to all active `DeviceConnection` threads.

## Phase 3: Hot-Plug & "Monitor All" Logic
- [ ] Task: Host - Implement Port Scanner.
    - [ ] Create a `PortScanner` that runs periodically.
    - [ ] In `monitor_all` mode: List all available ports, filter by VID.
    - [ ] In `explicit` mode: Check if configured ports are available.
- [ ] Task: Host - Manage Device Lifecycle.
    - [ ] If a new port is found, spawn a `DeviceConnection` thread.
    - [ ] If a thread errors (device disconnected), join/drop the thread and remove from active list so it can be re-detected.

## Phase 4: Verification
- [ ] Task: Host - Test with Config File.
    - [ ] Create a dummy config and verify ports are targeted.
- [ ] Task: Host - Test "Monitor All".
    - [ ] Plug in the device and verify it is picked up. Unplug and replug to test recovery.
- [ ] Task: Conductor - User Manual Verification 'Multi-Port Support' (Protocol in workflow.md)
