# Specification: Multi-Format Configuration Support

## Overview
This feature expands the host binary's configuration capabilities by supporting TOML, YAML, and JSON formats, as well as `.env` files. It introduces a flexible loading strategy where command-line arguments, environment variables, and configuration files work together with a clear precedence order.

## Functional Requirements

### Host Agent (Rust)
- **Multi-Format Support:** Use a configuration crate (e.g., `config` or `figment`) to parse `.toml`, `.yaml`, and `.json` files.
- **Environmental Files:** Support loading variables from a `.env` file in the current working directory.
- **Loading Strategy:**
    1. Check for a `.env` file in the CWD and load it into the environment.
    2. Search the current working directory for `side-eye.{toml,yaml,json}`.
    3. Search `~/.config/side-eye/config.{toml,yaml,json}`.
    4. If multiple formats exist in the same directory, prioritize TOML > YAML > JSON.
- **Precedence (Highest to Lowest):** 
    1. Command-line arguments (CLI flags)
    2. Environment variables (prefixed with `SIDEEYE_`)
    3. Configuration file (TOML/YAML/JSON)
    4. `.env` file values
    5. Internal Defaults
- **Configurable Settings:**
    - `port`: Serial port path.
    - `verbose`: Boolean or log level string.
    - `interval`: Data refresh interval in milliseconds (default 1000).
    - `filters`: List of USB VIDs/PIDs to search for.

## Non-Functional Requirements
- **Error Handling:** Provide clear error messages if a config file is malformed or inaccessible.
- **Maintainability:** Refactor `config.rs` to use the new unified configuration object.

## Acceptance Criteria
- The host binary successfully loads settings from a `config.yaml` file.
- The host binary successfully loads settings from a `config.json` file.
- Values in a `.env` file are correctly loaded and applied.
- Command-line flags correctly override settings found in any configuration file or environment variable.
- The binary falls back gracefully if no configuration file or `.env` file is found.
