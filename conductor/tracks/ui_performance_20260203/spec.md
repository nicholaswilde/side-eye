# Specification: UI Performance & Flicker Reduction

## Context
The current firmware implementation redraws the entire screen (including static elements like the banner and labels) every time a new data packet is received from the host (every 1 second). This full-screen clear and redraw causes a noticeable flickering effect on the LCD.

## Goals
1.  **Eliminate Flicker:** Transition from full-screen updates to partial-screen updates for dynamic values.
2.  **Optimize Rendering:** Redraw static elements only when necessary (e.g., orientation change or initial boot).
3.  **Smooth Progress Bars:** Update the RAM usage bar smoothly without clearing its entire area.

## Requirements

### Firmware (C++)
-   **Static Rendering:** Separate the UI into "static" (banner, labels) and "dynamic" (values, bars) components.
-   **Partial Updates:**
    -   When dynamic values (CPU %, Uptime) change, only the specific area occupied by the text should be cleared (by filling a small rectangle with the background color) and then redrawn.
    -   The RAM bar should be redrawn by drawing over the previous state or clearing its bounding box locally.
-   **State Comparison:** Maintain a local "previous state" to only trigger redraws when the values have actually changed.
-   **Initial Setup:** Ensure the background and static labels are drawn once during `setup()` or upon receiving the first `Identity` packet.

## Non-Functional Requirements
-   **Smoothness:** Display updates should appear instantaneous and flicker-free.
-   **Resource Efficiency:** Minimize the number of SPI transactions to the display.
