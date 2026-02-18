# SideEye Example Themes

This directory contains example themes for SideEye.

## Cyberpunk Theme

A high-contrast, neon-on-black theme inspired by retro-futuristic aesthetics.

### Contents
- `theme.json`: Defines the RGB565 color palette (Neon Green text, Black background).
- `generate_cyberpunk.py`: A Python script to generate the matching background JPEG images.

### Usage

1.  **Generate Images:**
    Run the Python script to generate the `.jpg` assets. Requires the `Pillow` library.
    ```bash
    pip install Pillow
    python3 generate_cyberpunk.py
    ```

2.  **Copy to SD Card:**
    Copy the entire `cyberpunk` folder (which will contain `theme.json` and the generated `.jpg` files) to the `/themes/` directory on your SideEye's SD card.
    ```
    /themes/cyberpunk/
      ├── theme.json
      ├── boot.jpg
      ├── identity.jpg
      ├── resources.jpg
      ...
    ```

3.  **Activate:**
    Use MQTT to switch to the new theme:
    ```bash
    mosquitto_pub -t "side-eye/<DEVICE_ID>/set/theme" -m "/themes/cyberpunk"
    ```
