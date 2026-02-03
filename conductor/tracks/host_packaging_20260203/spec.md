# Specification: Host - Linux System Integration & Packaging

## Context
Currently, running `side-eye-host` requires manually executing the binary or setting up a custom script. Users (especially on Linux Desktops and Servers) expect software to be installable via package managers and manageable via system standard tools like `systemd`. Permission issues with USB serial ports often plague manual setups.

## Goals
1.  **Auto-Start:** The host agent should start automatically on boot.
2.  **Permission Management:** Users should not need `sudo` to run the host; the system should grant access to the specific USB device.
3.  **Easy Installation:** Provide `.deb` (Debian/Ubuntu) and `.rpm` (Fedora/RHEL) packages.

## Detailed Requirements

### Systemd Integration
-   Create a `side-eye.service` unit file.
-   Run as a dedicated user (or the primary user) depending on configuration.
-   Restart policy: `Restart=on-failure` with a reasonable delay.
-   Logging: Standard Output to journald.

### Udev Rules
-   Create `99-side-eye.rules`.
-   Target the Waveshare ESP32-C6-GEEK VID/PID (0x303A / ...).
-   Action: Set group to `dialout` or `plugdev` (or a custom `sideeye` group) and mode `0660`.
-   Symlink: Create a stable symlink `/dev/side-eye` for consistent addressing.

### Packaging
-   **Tooling:** Use `cargo-deb` and `cargo-generate-rpm`.
-   **Assets:** Include the binary, the systemd unit, the udev rule, and the default config file.
-   **Post-Install Scripts:** Reload udev rules, enable/start the service.

## Non-Functional Requirements
-   **Security:** The service should *not* run as root if possible.
-   **Cleanliness:** Uninstallation should remove all created files (except potentially user config).
