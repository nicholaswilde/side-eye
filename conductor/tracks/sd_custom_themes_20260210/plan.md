# Implementation Plan - SD Card Custom Themes

This plan outlines the steps to implement SD-based custom theming, including image decoding and JSON-driven color overrides.

## Phase 1: Image Decoding & Fallback
- [ ] Task: Integrate Image Decoder
    - [ ] Add the `TJpg_Decoder` library to `platformio.ini`.
    - [ ] Implement `DisplayManager::drawJpg(const char* path, int x, int y)` to render images from SD.
- [ ] Task: Implement Fallback Logic
    - [ ] Ensure `DisplayManager` can gracefully switch between "Image Mode" and "Solid Color Mode" if the SD card is unavailable.
- [ ] Task: Conductor - User Manual Verification 'Phase 1: Image Decoding & Fallback' (Protocol in workflow.md)

## Phase 2: JSON Theming & Asset Management
- [ ] Task: Implement `theme.json` Parser
    - [ ] Define a standard `theme.json` schema for color overrides.
    - [ ] Add logic to `DisplayManager` to load and apply these colors during the `draw` cycle.
- [ ] Task: Implement Theme Switching
    - [ ] Add MQTT handler for theme path updates.
    - [ ] Update `config.json` to store the last selected theme path.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: JSON Theming & Asset Management' (Protocol in workflow.md)

## Phase 3: Boot Screen & Final Polish
- [ ] Task: Custom Boot Screen
    - [ ] Update `DisplayManager::drawBootScreen` to look for `/boot.jpg` on the SD card before drawing the default.
- [ ] Task: Performance Optimization
    - [ ] Optimize image loading speed to ensure page transitions remain smooth (~500ms target).
- [ ] Task: Conductor - User Manual Verification 'Phase 3: Boot Screen & Final Polish' (Protocol in workflow.md)
