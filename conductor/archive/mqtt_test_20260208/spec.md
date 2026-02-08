# Specification: MQTT Integration Testing Harness

## Overview
This track establishes a reproducible way to test the MQTT functionality of the SideEye firmware. It focuses on an end-to-end integration testing approach using a local Docker-based environment, allowing developers to verify actual network communication and payload formats without needing a public MQTT broker.

## Functional Requirements
1.  **Test Broker:** Provide a Docker Compose configuration to spin up a local Mosquitto MQTT broker.
2.  **Validation Tool:** Include a lightweight validation tool (e.g., a Python script or a simple CLI command) that subscribes to the test broker and logs received messages.
3.  **Connectivity Verification:** The harness must be able to verify that the firmware (running on hardware or in an emulator) can successfully connect to the local broker.
4.  **Payload Verification:** The harness must be able to verify that the published JSON payloads (stats, identity) match the expected schema.
5.  **Reconnection Logic:** Provide a way to simulate broker downtime (e.g., stopping the container) to verify the firmware's reconnection resilience.

## Non-Functional Requirements
-   **Reproducibility:** The test environment must be easily startable with a single command.
-   **Isolation:** The test broker should not interfere with any production or home automation brokers on the same network.

## Acceptance Criteria
- [ ] A `tests/mqtt/` directory exists with a `compose.yaml` for Mosquitto.
- [ ] A README or script exists explaining how to configure the firmware to point to the local test broker.
- [ ] The validation tool can successfully capture and display a "Stats" or "Identity" message from the firmware.
- [ ] The firmware successfully reconnects when the test broker container is restarted.

## Out of Scope
-   Automated CI execution of hardware-based integration tests (this remains manual/local for now).
-   Comprehensive unit testing of the MQTT state machine (covered by a separate track if needed).
