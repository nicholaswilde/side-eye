# Implementation Plan: Display Host Network, Identity & System Stats

## Phase 1: Host-Side Data Gathering [checkpoint: c668fc7]
- [x] **Dependencies:** Add `sysinfo` to `host/Cargo.toml`.
- [x] **Implementation:** Create a module `src/monitor.rs` to encapsulate `sysinfo::System`. febc0ea
    - [x] Implement `get_static_info()` -> Returns Hostname, IP, MAC, OS, User.
    - [x] Implement `update_and_get_stats()` -> Refreshes system state and returns CPU, RAM, Disk, Net Speed, Uptime.
- [x] **Test:** Write a small runner to print these values to the console every second to verify accuracy. febc0ea

## Phase 2: Protocol Definition [checkpoint: 17d0d3c]
- [x] **Schema:** Define two message structures (JSON or Protobuf): 4c4fa67
    -   `HostIdentity`: { hostname, ip, mac, os, user }
    -   `HostStats`: { cpu_percent, ram_used, ram_total, disk_used, disk_total, net_up, net_down, uptime }
- [x] **Host Send:** Update host main loop: 6585b5f
    - [x] Send `HostIdentity` immediately upon connection.
    - [x] Send `HostStats` in a loop (e.g., every 1s).

## Phase 3: Firmware Display [checkpoint: 7dff7a2]
- [x] **Receive:** Update firmware packet parser to handle the new message types. 17d0d3c
- [x] **State:** Create a data store/struct in C++ to hold the latest values. 17d0d3c
- [x] **UI Refinement:** Update display layout to use a centered title on a banner (similar to frame-fi), with smaller font sizes and a slight left shift for data elements. 17d0d3c
- [x] **Orientation Support:** 17d0d3c
    -   [x] Implement button interrupt or polling.
    -   [x] On press: Toggle display rotation (0 -> 180 -> 0).
    -   [x] (Optional) Save state to NVS preferences.
- [x] **Test:** Verify the info appears and updates smoothly on the screen. Verify button toggles rotation. 17d0d3c
