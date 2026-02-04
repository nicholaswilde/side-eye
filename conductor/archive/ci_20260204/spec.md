# Specification - GitHub CI Workflow

## Overview
This track involves creating a GitHub Actions workflow named `ci` to automate the execution of the `task test:ci` suite. This ensures that every release candidate (tagged version) and manually triggered run maintains the project's quality standards for both host and firmware code.

## Functional Requirements
- **Triggers:**
    - Trigger on push of tags matching the `v*.*.*` pattern (SemVer).
    - Trigger on manual dispatch via the GitHub UI (`workflow_dispatch`).
- **Execution Environment:**
    - Use the `ubuntu-latest` runner.
- **Workflow Steps:**
    - Checkout the repository.
    - Install system dependencies: `pkg-config`, `libudev-dev`.
    - Install `go-task`.
    - Setup Rust toolchain.
    - Setup Python and PlatformIO Core.
    - Execute `task test:ci`.

## Non-Functional Requirements
- **Efficiency:** The workflow should be optimized for speed (e.g., caching dependencies if feasible).
- **Reliability:** The workflow must accurately report failures to prevent broken releases.

## Acceptance Criteria
- [ ] A file `.github/workflows/ci.yml` exists.
- [ ] The workflow successfully triggers on a `v*.*.*` tag push.
- [ ] The workflow successfully triggers on a manual dispatch.
- [ ] The workflow fails if any part of `task test:ci` (fmt, lint, clippy, tests, firmware check) fails.
- [ ] The workflow passes when all project standards are met.

## Out of Scope
- Automated deployment or publishing of artifacts (handled by `release.yml`).
- Integration tests requiring physical hardware.
