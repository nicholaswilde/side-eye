# Specification: UI State & Status Display

## Context
The current UI shows host-related labels (Host, IP, CPU, RAM) even when no host is connected. Furthermore, the "Waiting for Host..." message currently overlaps with these labels, creating a cluttered and confusing interface.

## Goals
1.  **Clear Status Indication:** Add a "Status:" line to the UI that clearly shows whether a host is connected.
2.  **Conditional Rendering:** Only show host-specific labels and values when a host is actually connected.
3.  **Clean Layout:** Prevent any text overlap between status messages and data labels.

## Requirements

### Firmware (C++)
-   **New UI Line:** Add a "Status:" label to the static UI.
-   **Status Values:**
    -   "Waiting..." (or "Waiting for Host") when `state.has_data` is false.
    -   "Connected" when `state.has_data` is true.
-   **Visibility Logic:**
    -   Modify `drawStaticUI` to only draw the "Status:" label initially.
    -   Host labels (Host, IP, CPU, RAM) should only be drawn once `state.has_data` becomes true.
    -   The "Uptime" footer should also be hidden until connected.
-   **Redraw Trigger:** When `state.has_data` transitions from `false` to `true`, perform a full `drawStaticUI()` to reveal all labels.

## Non-Functional Requirements
-   **User Feedback:** The UI state should clearly communicate the device's connectivity status at all times.
