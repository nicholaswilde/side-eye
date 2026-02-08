# Specification: Dockerize Rust Host Agent

## Overview
This track aims to provide a Dockerfile for the `side-eye-host` Rust agent, enabling users to run the system monitoring tool in a containerized environment. The build process will leverage `cargo-chef` for efficient dependency caching, and the runtime will use a minimal distroless image for security and size optimization.

## Functional Requirements
1.  **Multi-Stage Build:**
    -   Implement a `chef` stage using `rust:latest` to install `cargo-chef`.
    -   Implement a `planner` stage to prepare the build recipe.
    -   Implement a `builder` stage to cook dependencies and build the `side-eye-host` binary in release mode.
    -   Implement a `runtime` stage using `gcr.io/distroless/cc-debian12`.
2.  **Hardware Access:**
    -   The Docker image must be compatible with `--privileged` mode to allow the containerized host agent to access USB serial devices (e.g., `/dev/ttyACM0` or `/dev/side-eye`).
3.  **Configuration Management:**
    -   The runtime environment should be configured to look for `config.toml` at a standard path (e.g., `/etc/side-eye/config.toml`), which users are expected to provide via a volume mount.
4.  **Binary Integration:**
    -   The final image should contain only the compiled `side-eye-host` binary and necessary shared libraries.

## Non-Functional Requirements
-   **Security:** Use a distroless base image to minimize the attack surface.
-   **Performance:** Optimize build times using Docker layer caching for Rust dependencies.
-   **Maintainability:** Follow idiomatic Dockerfile patterns for Rust applications.

## Acceptance Criteria
- [ ] A `Dockerfile` exists in the repository root.
- [ ] The `Dockerfile` builds successfully using `docker build -t side-eye-host .`.
- [ ] The resulting image is minimal (contains only the binary and essential libs).
- [ ] The container can be started with `--privileged` and successfully detects connected SideEye hardware (verified via logs).
- [ ] The container correctly loads configuration from a volume-mounted `config.toml`.

## Out of Scope
-   Automated image publishing to Docker Hub (this may be a future track).
-   Multi-architecture Docker builds (e.g., ARM64 support) unless natively supported by the build environment.
