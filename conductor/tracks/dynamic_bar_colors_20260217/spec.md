# Specification - Dynamic Bar Graph Colors

## Overview
Unify the visual language of the SideEye dashboard by making all resource usage bars (CPU, RAM, Disk, and SD Card) responsive to their current percentage. This provides immediate visual feedback on system health and storage capacity.

## Functional Requirements
- **SD Card Bar Logic:**
  - **Syncing Mode:** If `sd_sync_status` is "Syncing...", the bar color shall be `active_theme.blue`.
  - **Capacity Mode (Idle):**
    - **Green:** < 50% full.
    - **Yellow:** 50% - 80% full.
    - **Red:** > 80% full.
- **Unified Thresholds:**
  - Ensure CPU, RAM, and Disk bars consistently use the 50% (Warning) and 80% (Critical) thresholds for color transitions.
- **Thematic Consistency:**
  - Colors must be pulled from the `active_theme` struct (dynamic) rather than hardcoded constants.

## Acceptance Criteria
- [ ] The SD Card usage bar turns blue when a folder sync is in progress.
- [ ] When idle, the SD Card bar correctly transitions from Green to Yellow to Red as it fills up.
- [ ] The Disk usage bar color matches the logic used for CPU and RAM.
- [ ] All page transitions and dynamic updates remain performant.

## Out of Scope
- Customizing thresholds per resource (e.g., 90% for disk vs 70% for CPU).
- Adding numerical "Warning/Critical" labels next to the bars.
