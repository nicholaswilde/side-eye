# Specification: Firmware - Hardware Button Integration & Backlight Control

## Context
Currently, the device is "passive" – it just shows what it receives. To make it a better desk accessory, users need control over its behavior, specifically the ability to turn off the bright screen at night or quickly check specific stats without waiting for the auto-cycle.

## Goals
1.  **Manual Page Control:** Short pressing the "Boot" button should immediately advance to the next information page.
2.  **Backlight Toggle:** Long pressing the button should toggle the LCD backlight (On/Off).
3.  **Auto-Dim/Timeout:** If the host stops sending data (PC shutdown) or after a configurable inactivity period, the screen should turn off.

## Detailed Requirements

### Hardware
-   **Button Pin:** GPIO 9 (Standard Boot button on C6-GEEK).
-   **Backlight Pin:** Check schematic (Usually a GPIO connected to BLK control, often needing PWM for dimming or simple High/Low).

### Logic
-   **Input Handling:**
    -   Implement a non-blocking debounce class/function.
    -   Detect "Click" (Release < 500ms).
    -   Detect "Hold" (Press > 1000ms).
-   **State Management:**
    -   `is_screen_on`: Boolean.
    -   `last_interaction_time`: Timestamp.
    -   `current_page`: Integer.
-   **Screen Saver:**
    -   If `millis() - last_serial_packet_time > 60000` (1 min), turn off backlight.
    -   Wake up immediately on next button press or serial packet (configurable - maybe only button wakes it to avoid sleep-wake loops). *Decision: Serial Packet from host should wake screen (e.g. PC boots up).*

## Non-Functional Requirements
-   **Responsiveness:** Button actions must feel immediate.
-   **Power:** Turning off the backlight should significantly reduce power draw (though USB powered, it matters for heat/hub load).
