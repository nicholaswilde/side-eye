# Implementation Plan: Firmware - Hardware Button Integration & Backlight Control

## Phases

### Phase 1: Hardware Verification
- [ ] Write a small test sketch to verify GPIO 9 reads correctly.
- [ ] Verify Backlight control pin (PWM vs Digital Write).
- [ ] Determine active-low vs active-high logic for button.

### Phase 2: Input Logic (C++)
- [ ] Implement `Button` class (Debounce, Click, LongPress).
- [ ] Integrate into `loop()`.

### Phase 3: Feature Implementation
- [ ] **Manual Cycle:** Connect "Click" event to `nextPage()` function.
    - [ ] Reset the "Auto-Cycle" timer when manually clicked so the page stays for a moment.
- [ ] **Backlight Control:** Connect "Hold" event to `toggleBacklight()`.
- [ ] **Timeout:** Add logic to check `last_data_received` timestamp.
    - [ ] Logic: `if (now - last_data > TIMEOUT) setBacklight(OFF)`

### Phase 4: Testing
- [ ] Verify Short Press cycles pages.
- [ ] Verify Long Press turns screen black.
- [ ] Verify pulling the USB data lines (simulating host off) triggers timeout.
