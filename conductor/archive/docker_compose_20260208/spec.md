# Specification: Docker Compose Configuration

## Overview
This track involves creating a standard `compose.yaml` file for the `side-eye-host` agent. The goal is to provide users with a "one-command" deployment option using the official container image from GHCR, while also including comprehensive documentation for optional configurations through commented-out sections.

## Functional Requirements
1.  **Primary Image:** The service must default to using `ghcr.io/nicholaswilde/side-eye-host:latest`.
2.  **Hardware Access:** 
    -   Include `privileged: true` by default to ensure seamless access to USB serial devices across different host distributions.
    -   Map the host's `/dev` directory to the container.
3.  **Environment Variables:** 
    -   Include `TZ` (Timezone) for accurate log timestamps.
    -   Include `RUST_LOG` (log level) to allow easy debugging.
4.  **Commented-Out Options (Optional/Advanced):**
    -   **Build from Source:** A commented section for building the image locally from the project's `Dockerfile`.
    -   **Custom Configuration:** A commented volume mount for a local `config.toml`.
    -   **Specific Device Mapping:** A commented `devices` section mapping `/dev/side-eye` as a more secure alternative to privileged mode.
    -   **Restart Policy:** A commented `restart: unless-stopped` policy.
    -   **Log Management:** Commented `logging` options for size-based rotation.

## Non-Functional Requirements
-   **Usability:** The file should be well-commented to guide users through its customization.
-   **Standards:** Follow the modern Docker Compose specification (no version key required).

## Acceptance Criteria
- [ ] A `compose.yaml` file exists in the project root.
- [ ] Running `docker compose up -d` (unmodified) successfully pulls the GHCR image and starts the agent with `/dev` access.
- [ ] The file contains all requested commented-out sections (Build, Config, Device Mapping, Restart, Logging).

## Out of Scope
-   Creating a Docker Compose file for the firmware build environment.
-   Automated testing of the compose file in CI (covered by separate docker runtime tests if applicable).
