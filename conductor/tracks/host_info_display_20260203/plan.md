# Implementation Plan: Display Host Network, Identity & System Stats

## Phase 1: Host-Side Data Gathering
- [ ] **Dependencies:** Add `sysinfo` to `host/Cargo.toml`.
- [ ] **Implementation:** Create a module `src/monitor.rs` to encapsulate `sysinfo::System`.
    -   Implement `get_static_info()` -> Returns Hostname, IP, MAC, OS, User.
    -   Implement `update_and_get_stats()` -> Refreshes system state and returns CPU, RAM, Disk, Net Speed, Uptime.
- [ ] **Test:** Write a small runner to print these values to the console every second to verify accuracy.

## Phase 2: Protocol Definition
- [ ] **Schema:** Define two message structures (JSON or Protobuf):
    -   `HostIdentity`: { hostname, ip, mac, os, user }
    -   `HostStats`: { cpu_percent, ram_used, ram_total, disk_used, disk_total, net_up, net_down, uptime }
- [ ] **Host Send:** Update host main loop:
    -   Send `HostIdentity` immediately upon connection.
    -   Send `HostStats` in a loop (e.g., every 1s).

## Phase 3: Firmware Display
- [ ] **Receive:** Update firmware packet parser to handle the new message types.
- [ ] **State:** Create a data store/struct in C++ to hold the latest values.
- [ ] **UI:** Update the LVGL interface.
    -   Create a robust layout (Grid/Flex).
    -   Add Labels for static info.
    -   Add Bar/Arc widgets for CPU/RAM/Disk.
    -   Add Labels for dynamic text (Net speed, Uptime).
- [ ] **Orientation Support:**
    -   Implement button interrupt or polling.
    -   On press: Toggle display rotation (0 -> 180 -> 0).
    -   (Optional) Save state to NVS preferences.
- [ ] **Test:** Verify the info appears and updates smoothly on the screen. Verify button toggles rotation.
