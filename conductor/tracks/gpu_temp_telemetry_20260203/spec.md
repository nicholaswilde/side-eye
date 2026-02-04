# Specification: GPU & Temperature Telemetry

## Context
Advanced users require monitoring of system temperatures and GPU utilization to ensure hardware is running within safe limits during heavy tasks.

## Goals
1.  **Thermal Monitoring:** Gather CPU and System temperature data on the host.
2.  **GPU Insight:** Monitor GPU Load and VRAM usage.
3.  **Visual Page:** Add a dedicated "Thermal" page to the firmware UI.

## Detailed Requirements

### Host Agent (Rust)
- **Data Gathering:** 
    - CPU Temperature: Use `sysinfo::Components`.
    - GPU Stats: 
        - NVIDIA: Interface with `nvml` (via `nvml-wrapper` crate).
        - AMD: Interface with `libdrm` or parse `/sys/class/drm/`.
- **Protocol:** Add `thermal` and `gpu` fields to the `Stats` message.

### Firmware (C++)
- **New Page:** Add `PAGE_THERMAL` to the `Page` enum.
- **UI:**
    - Display CPU Temp (°C) with a gauge or progress bar.
    - Display GPU Load (%) and VRAM usage.
    - Use "Fire" or "Thermometer" icons.

## Non-Functional Requirements
- **Efficiency:** Polling thermal sensors should not significantly impact host CPU.
- **Robustness:** Gracefully handle systems without dedicated GPUs.
