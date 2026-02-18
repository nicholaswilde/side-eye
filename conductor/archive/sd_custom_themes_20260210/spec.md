# Specification - SD Card Custom Themes

## Overview
Transform the SideEye's visual identity by allowing users to load custom background images, color schemes, and boot screens from the integrated SD card. This enables high levels of personalization for aesthetic desk setups.

## Functional Requirements
- **Theme Assets:**
  - **Backgrounds:** Support loading `.jpg` (240x135) images for each dashboard page from a theme folder.
  - **Color/Font Overrides:** Support a `theme.json` file to define hex colors for text, bars, and headers, as well as font size scaling.
  - **Boot Screen:** Support a `boot.jpg` to replace the default startup screen.
- **Theme Management:**
  - **Default Load:** On startup, the device checks `/themes/active/` for assets.
  - **Dynamic Switching:** The device shall subscribe to `side-eye/DEVICE_ID/set/theme` to change the active theme path on the SD card.
- **Resilience:**
  - If the SD card is unmounted or files are missing, the device shall fall back to the internal `catppuccin_colors.h` theme.

## Acceptance Criteria
- [ ] Placing a `background.jpg` in the active theme folder on the SD card displays it behind the system stats.
- [ ] Modifying `theme.json` successfully changes the text color on the display without a re-flash.
- [ ] Changing the theme via MQTT (`/set/theme`) reloads the UI assets immediately.
- [ ] The device boots normally with default colors if no SD card is present.
