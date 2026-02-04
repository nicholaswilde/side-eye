# Specification - Multi-Architecture Cross-Compilation

## Overview
This track adds the capability to cross-compile the SideEye host binary for multiple Linux architectures using the `cross` tool. This will enable users to build binaries for AMD64, ARM64, ARMv7, and ARMv6 (ARMv5te) platforms directly from their primary development machine.

## Functional Requirements
- **Support Multiple Target Architectures:**
    - `x86_64-unknown-linux-gnu` (AMD64)
    - `aarch64-unknown-linux-gnu` (ARM64)
    - `armv7-unknown-linux-gnueabihf` (ARMv7)
    - `arm-unknown-linux-gnueabihf` (ARMv6/v5te)
- **Task Automation:**
    - Integrate architecture-specific build tasks into `Taskfile.yml` under the `host` namespace.
    - Provide a consolidated `host:build:all` task to build all supported architectures in sequence.
- **Tooling Integration:**
    - Utilize `cross` for all cross-compilation commands to ensure consistent build environments via Docker/Podman.

## Non-Functional Requirements
- **Consistency:** Build commands should mirror the patterns established in the `proxmox-mcp-rs` reference.
- **Efficiency:** Use `--release` mode for all cross-compiled binaries to ensure optimal performance on target devices.

## Acceptance Criteria
- [ ] `Taskfile.yml` contains `host:build:amd64`, `host:build:arm64`, `host:build:armv7`, and `host:build:armv6` tasks.
- [ ] `Taskfile.yml` contains a `host:build:all` task.
- [ ] Running `task host:build:amd64` successfully invokes `cross build --release --target x86_64-unknown-linux-gnu`.
- [ ] Running `task host:build:arm64` successfully invokes `cross build --release --target aarch64-unknown-linux-gnu`.
- [ ] Running `task host:build:armv7` successfully invokes `cross build --release --target armv7-unknown-linux-gnueabihf`.
- [ ] Running `task host:build:armv6` successfully invokes `cross build --release --target arm-unknown-linux-gnueabihf`.

## Out of Scope
- Installation of `cross` or its dependencies (Docker/Podman).
- Cross-compilation for non-Linux platforms (e.g., Windows, macOS).
- Automated CI integration for these new tasks (will be handled in a separate track if needed).
