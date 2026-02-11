# Implementation Plan - Backlight Dimming & SPI Optimization

This plan outlines the steps to implement hardware-aligned backlight control and optimize SPI performance for the SideEye monitor.

## Phase 1: Backlight PWM Control
- [x] Task: Switch backlight control to PWM (8d9ffcf)
    - [x] Update \`DisplayManager.h\` to use \`analogWrite\` for \`LCD_BL\`.
    - [x] Update \`setBacklight(bool on)\` to use full brightness (255) and off (0).
- [x] Task: Implement smooth transitions (8d9ffcf)
    - [x] Add \`fadeBacklight(uint8_t target, uint16_t duration_ms)\` to \`DisplayManager\`.
    - [x] Update \`InputHandler\` to use fading when waking or sleeping the screen.
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Backlight PWM Control' (Protocol in workflow.md)

## Phase 2: SPI Performance Optimization
- [ ] Task: Increase SD SPI Frequency
    - [ ] Increase frequency from 1MHz to 4MHz or higher in `SyncManager.h` after initialization.
    - [ ] Test reliability during directory listing and file synchronization.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: SPI Performance Optimization' (Protocol in workflow.md)
