# Implementation Plan: Host - Linux System Integration & Packaging

## Phases

### Phase 1: Configuration & Assets [checkpoint: 62928ee]
- [x] Create `host/assets/side-eye.service` template. d28b283
- [x] Create `host/assets/99-side-eye.rules`. 74406d3
- [x] Verify udev rules against the actual hardware. 5670841

### Phase 2: Cargo Deb Setup [checkpoint: 53fb49c]
- [x] Add `cargo-deb` configuration to `host/Cargo.toml`. 02c5a95
- [x] Test `cargo deb` build command. 10328


### Phase 3: RPM & Arch (Optional)
- [x] Add `cargo-generate-rpm` configuration (if feasible, otherwise prioritize Deb). 83d4012
- [x] Document manual installation steps for Arch/Aur (PKGBUILD reference). 83d4012

### Phase 4: CI/CD Integration
- [x] Update build workflow to generate these artifacts on release. fb82d59

