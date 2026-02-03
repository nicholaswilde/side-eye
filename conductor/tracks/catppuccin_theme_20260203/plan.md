# Implementation Plan: UI Theme - Catppuccin Color Palette

## Phase 1: Setup Resources
- [x] Task: Firmware - Create `catppuccin_colors.h`. c1bc0ee
    - [x] Create `firmware/include/catppuccin_colors.h` with the content provided from `frame-fi` (Mocha Theme).

## Phase 2: Apply Theme
- [x] Task: Firmware - Update `main.cpp` to include the new header. fc24c67
    - [x] Add `#include "catppuccin_colors.h"`.
- [x] Task: Firmware - Refactor `updateDisplay` and `setup` to use Catppuccin colors. b3baeb9
    - [x] `gfx->fillScreen(CATPPUCCIN_BASE);`
    - [x] Header "SideEye": `CATPPUCCIN_LAVENDER`
    - [x] Divider: `CATPPUCCIN_CRUST` or `CATPPUCCIN_TEXT` (dimmed).
    - [x] Labels (Host/IP/MAC): `CATPPUCCIN_SUBTEXT` (if avail) or `CATPPUCCIN_TEXT`.
    - [x] Values:
        - Hostname: `CATPPUCCIN_MAUVE`
        - IP: `CATPPUCCIN_GREEN`
        - MAC: `CATPPUCCIN_PINK`

## Phase 3: Verification
- [ ] Task: Firmware - Compile and visual check.
    - [ ] Run `pio run` to ensure no syntax errors.
- [ ] Task: Conductor - User Manual Verification 'Theme Application' (Protocol in workflow.md)
