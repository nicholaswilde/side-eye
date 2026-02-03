# Implementation Plan: Multi-Format Configuration Support

## Phase 1: Dependencies & Struct Definition
- [ ] Task: Host - Add `figment` and `dotenvy` dependencies.
    - [ ] Update `Cargo.toml` with `figment` (including `toml`, `yaml`, `json`, `env` providers) and `dotenvy`.
- [ ] Task: Host - Refactor `Config` struct for multi-format compatibility.
    - [ ] Update `src/config.rs` to support `serde` deserialization for all fields.
    - [ ] Add `interval` and `filters` to the `Config` struct.
- [ ] Task: Conductor - User Manual Verification 'Dependencies & Structs' (Protocol in workflow.md)

## Phase 2: Configuration Loading Logic
- [ ] Task: Host - Implement Figment-based loading strategy.
    - [ ] **Red Phase:** Write a test in `tests/config.rs` that expects settings to be loaded from a specific TOML/YAML file.
    - [ ] **Green Phase:** Implement the hierarchical loading (File -> Env -> Defaults) in `config.rs`.
- [ ] Task: Host - Integrate `.env` file support.
    - [ ] **Red Phase:** Write a test that expects values from a `.env` file to be present in the final config.
    - [ ] **Green Phase:** Use `dotenvy` to load the `.env` file before Figment processing.
- [ ] Task: Conductor - User Manual Verification 'Loading Logic' (Protocol in workflow.md)

## Phase 3: CLI Integration & Precedence
- [ ] Task: Host - Update CLI argument parsing to override configuration.
    - [ ] **Red Phase:** Write an integration test in `tests/cli.rs` that ensures a CLI flag overrides a value in a config file.
    - [ ] **Green Phase:** Update `main.rs` to merge `clap` arguments with the `Figment` result.
- [ ] Task: Host - Map environment variables to configuration fields.
    - [ ] Ensure `SIDEEYE_PORT`, `SIDEEYE_INTERVAL`, etc., are correctly picked up.
- [ ] Task: Conductor - User Manual Verification 'CLI & Precedence' (Protocol in workflow.md)
