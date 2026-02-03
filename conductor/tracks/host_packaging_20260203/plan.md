# Implementation Plan: Host - Linux System Integration & Packaging

## Phases

### Phase 1: Configuration & Assets
- [ ] Create `host/assets/side-eye.service` template.
- [ ] Create `host/assets/99-side-eye.rules`.
- [ ] Verify udev rules against the actual hardware.

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
