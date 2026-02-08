# Implementation Plan: Dockerize Rust Host Agent

## Phase 1: Build Environment & Scaffolding [checkpoint: 8434a26]
- [x] Task: Create `Dockerfile` with multi-stage setup 3a9d384
    - [x] Define the `chef` stage using `rust:latest`.
    - [x] Install `cargo-chef`, `libudev-dev`, and `pkg-config` in the build environment.
    - [x] Define the `planner` and `builder` stages.
- [x] Task: Configure dependency caching 3a9d384
    - [x] Implement `cargo chef prepare` and `cargo chef cook` steps. 3a9d384
- [x] Task: Conductor - User Manual Verification 'Build Environment & Scaffolding' (Protocol in workflow.md) 8434a26

## Phase 2: Runtime Optimization
- [~] Task: Implement minimal runtime stage
    - [ ] Use `gcr.io/distroless/cc-debian12` as the runtime base.
    - [ ] Copy the `side-eye-host` binary from the builder stage.
    - [ ] Configure the default configuration path via environment variable or documentation.
- [ ] Task: Add `.dockerignore`
    - [ ] Create a `.dockerignore` file to exclude `target/`, `firmware/`, and other unnecessary files.
- [ ] Task: Conductor - User Manual Verification 'Runtime Optimization' (Protocol in workflow.md)

## Phase 3: Integration & Testing
- [ ] Task: Perform test build
    - [ ] Run `docker build -t side-eye-host .` and ensure it completes without errors.
- [ ] Task: Validate hardware access and configuration
    - [ ] Document and test the `docker run` command with `--privileged` and `-v`.
    - [ ] Verify the agent starts and detects hardware in the container logs.
- [ ] Task: Conductor - User Manual Verification 'Integration & Testing' (Protocol in workflow.md)
