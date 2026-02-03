# Implementation Plan: UI Theme - Catppuccin Color Palette

## Phase 1: Setup Resources
- [x] Task: Firmware - Create `catppuccin_colors.h`. c1bc0ee
    - [x] Create `firmware/include/catppuccin_colors.h` with the content provided from `frame-fi` (Mocha Theme).

## Phase 2: Apply Theme
- [ ] Task: Firmware - Update `main.cpp` to include the new header.
    - [ ] Add `#include "catppuccin_colors.h"`.
- [ ] Task: Firmware - Refactor `updateDisplay` and `setup` to use Catppuccin colors.
    - [ ] `gfx->fillScreen(CATPPUCCIN_BASE);`
    - [ ] Header "SideEye": `CATPPUCCIN_LAVENDER`
    - [ ] Divider: `CATPPUCCIN_CRUST` or `CATPPUCCIN_TEXT` (dimmed).
    - [ ] Labels (Host/IP/MAC): `CATPPUCCIN_SUBTEXT` (if avail) or `CATPPUCCIN_TEXT`.
    - [ ] Values:
        - Hostname: `CATPPUCCIN_MAUVE`
        - IP: `CATPPUCCIN_GREEN`
        - MAC: `CATPPUCCIN_PINK`

## Phase 3: Verification
- [ ] Task: Firmware - Compile and visual check.
    - [ ] Run `pio run` to ensure no syntax errors.
- [ ] Task: Conductor - User Manual Verification 'Theme Application' (Protocol in workflow.md)
