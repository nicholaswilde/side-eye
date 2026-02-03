# Specification: UI Theme - Catppuccin Color Palette

## Context
The user requested that the SideEye LCD interface match the aesthetics of the `frame-fi` project by using the Catppuccin color palette. This involves importing the 16-bit RGB565 color definitions and updating the `main.cpp` display logic to use them.

## Goals
1.  **Visual Consistency:** The UI should look modern and consistent with the `frame-fi` ecosystem.
2.  **Code Reusability:** Use the same color definition file format as `frame-fi`.

## Detailed Requirements

### Firmware (C++)
-   **Header File:** Create `firmware/include/catppuccin_colors.h` containing the RGB565 definitions for the Catppuccin Mocha theme.
-   **UI Updates:**
    -   **Background:** Change `BLACK` to `CATPPUCCIN_BASE`.
    -   **Header Text:** Change `CYAN` to `CATPPUCCIN_MAUVE` (or `LAVENDER`).
    -   **Divider Line:** Change `DARKGREY` to `CATPPUCCIN_SURFACE0` (or `OVERLAY0` - need to verify available colors, fallback to `TEXT` or `SUBTEXT`). *Self-correction: The provided file doesn't have Surface/Overlay, so use `CATPPUCCIN_TEXT` or `CATPPUCCIN_CRUST`.* Let's use `CATPPUCCIN_LAVENDER` for the header and `CATPPUCCIN_TEXT` for standard text.
    -   **Hostname:** Change `WHITE` to `CATPPUCCIN_TEXT`.
    -   **IP Label:** Change `GREEN` to `CATPPUCCIN_GREEN`.
    -   **MAC Label:** Change `MAGENTA` to `CATPPUCCIN_PINK` (or `FLAMINGO`).
-   **Text Visibility:** Ensure good contrast is maintained with the new `BASE` background.

## Non-Functional Requirements
-   **Performance:** No impact on rendering speed (these are compile-time constants).
