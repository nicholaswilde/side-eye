# Implementation Plan: Host - Enhanced USB Port Monitoring & Configuration

## Phase 1: Configuration & Argument Parsing
- [ ] Task: Host - Add `serde` and `toml` (or `json`) dependencies.
    - [ ] Update `Cargo.toml`.
- [ ] Task: Host - Define Configuration Structure.
    - [ ] Create a `Config` struct with `ports`, `monitor_all`, and `target_vids`.
- [ ] Task: Host - Implement Config Loading.
    - [ ] Logic to load from default path (e.g., `~/.config/side-eye/config.toml`) or path specified by `--config`.
    - [ ] Logic to merge CLI args with config file (CLI takes precedence).

## Phase 2: Refactoring for Multi-Device Support
- [ ] Task: Host - Refactor `main.rs` loop.
    - [ ] Move the "connect and send" logic into a separate function/struct (e.g., `DeviceConnection`).
    - [ ] Use `std::sync::mpsc` or `tokio` (if moving to async, though threads are fine for low count) to manage device states.
- [ ] Task: Host - Implement "System Stats" Broadcaster.
    - [ ] Create a central loop that gathers system stats once every 5 seconds.
    - [ ] Broadcast the formatted string to all active `DeviceConnection` threads.

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
