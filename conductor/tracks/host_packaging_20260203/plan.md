# Implementation Plan: Host - Linux System Integration & Packaging

## Phases

### Phase 1: Configuration & Assets [checkpoint: 62928ee]
- [x] Create `host/assets/side-eye.service` template. d28b283
- [x] Create `host/assets/99-side-eye.rules`. 74406d3
- [x] Verify udev rules against the actual hardware. 5670841

### Phase 2: Cargo Deb Setup
- [ ] Add `cargo-deb` configuration to `host/Cargo.toml`.
    - [ ] Map binary to `/usr/bin/side-eye-host`.
    - [ ] Map assets to `/etc/systemd/system/` and `/etc/udev/rules.d/`.
    - [ ] Define dependencies (`libudev`, etc.).
- [ ] Test `cargo deb` build command.

### Phase 3: RPM & Arch (Optional)
- [ ] Add `cargo-generate-rpm` configuration (if feasible, otherwise prioritize Deb).
- [ ] Document manual installation steps for Arch/Aur (PKGBUILD reference).

### Phase 4: CI/CD Integration
- [ ] Update build workflow to generate these artifacts on release.
