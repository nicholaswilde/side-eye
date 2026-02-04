# Implementation Plan: Firmware - Hardware Button Integration & Backlight Control

## Phases

### Phase 1: Hardware Verification
- [x] Write a small test sketch to verify GPIO 9 reads correctly. (Confirmed in main.cpp)
- [x] Verify Backlight control pin (PWM vs Digital Write). (Digital in main.cpp)
- [x] Determine active-low vs active-high logic for button. (Active-low in main.cpp)

### Phase 2: Input Logic (C++)
- [x] Implement `Button` class (Debounce, Click, Double-Click, LongPress).
- [x] Integrate into `loop()`.

### Phase 3: Feature Implementation
- [x] **Manual Cycle:** Connect "Click" event to cycle pages.
- [x] **Backlight Control:** Connect "Hold" event to toggle backlight.
- [x] **Rotation Control:** Connect "Double-Click" event to rotate screen (migrated).
- [x] **Timeout:** Add logic to check `last_interaction_time` timestamp.

### Phase 4: Testing
- [x] Verify Short Press cycles pages.
- [x] Verify Long Press turns screen black.
- [x] Verify pulling the USB data lines (simulating host off) triggers timeout.
- [x] Verify Double-Click rotates screen.
