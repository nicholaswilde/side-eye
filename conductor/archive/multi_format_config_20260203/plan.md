# Implementation Plan: Multi-Format Configuration Support

## Phase 1: Dependencies & Struct Definition
- [x] Task: Host - Add `figment` and `dotenvy` dependencies.
- [x] Task: Host - Refactor `Config` struct for multi-format compatibility.
    - [x] Update `src/config.rs` to support `serde` deserialization for all fields.
    - [x] Add `interval` and `filters` to the `Config` struct.
- [x] Task: Conductor - User Manual Verification 'Dependencies & Structs' (Confirmed by tests)

## Phase 2: Configuration Loading Logic
- [x] Task: Host - Implement Figment-based loading strategy.
- [x] Task: Host - Integrate `.env` file support.
- [x] Task: Conductor - User Manual Verification 'Loading Logic' (Confirmed by tests)

## Phase 3: CLI Integration & Precedence
- [x] Task: Host - Update CLI argument parsing to override configuration.
    - [x] **Red Phase:** Write an integration test in `tests/cli.rs` that ensures a CLI flag overrides a value in a config file.
    - [x] **Green Phase:** Update `main.rs` to merge `clap` arguments with the `Figment` result.
- [x] Task: Host - Map environment variables to configuration fields.
    - [x] Ensure `SIDEEYE_PORT`, `SIDEEYE_INTERVAL`, etc., are correctly picked up.
- [x] Task: Conductor - User Manual Verification 'CLI & Precedence' (Confirmed by tests)
