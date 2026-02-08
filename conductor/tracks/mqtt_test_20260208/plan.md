# Implementation Plan: MQTT Integration Testing Harness

## Phase 1: Environment Scaffolding [checkpoint: a7d95c4]
- [x] 7c88fe5 Task: Create MQTT test directory structure
    - [ ] Create `tests/mqtt/` directory.
    - [ ] Create `tests/mqtt/compose.yaml` with a basic Mosquitto service.
    - [ ] Create a minimal `mosquitto.conf` allowing anonymous access for testing.
- [x] Task: Conductor - User Manual Verification 'Environment Scaffolding' (Protocol in workflow.md)

## Phase 2: Validation Tooling [checkpoint: 2d8bbe1]
- [x] 2a6b4ca Task: Create a simple MQTT validator script
    - [x] Use Python (with `paho-mqtt`) or a simple shell script (using `mosquitto_sub`) to listen for SideEye topics.
    - [x] Implement basic JSON validation to ensure payloads are well-formed.
- [x] 2a6b4ca Task: Add Documentation
    - [x] Create `tests/mqtt/README.md` with instructions on:
        - Starting the harness.
        - Configuring the firmware `secrets.h` to use the host's IP.
        - Verifying results.
- [x] Task: Conductor - User Manual Verification 'Validation Tooling' (Protocol in workflow.md)

## Phase 3: Live Verification
- [ ] Task: Perform End-to-End Test
    - [ ] Flash firmware with test broker settings.
    - [ ] Verify connectivity and payload receipt in the validator script.
    - [ ] Stop and restart the Mosquitto container to verify firmware reconnection.
- [ ] Task: Conductor - User Manual Verification 'Live Verification' (Protocol in workflow.md)
