# Specification: Firmware Modularization

## Context
The current `main.cpp` has exceeded 800 lines and contains all logic for display management, button handling, Wi-Fi connectivity, MQTT communication, and SD card synchronization. This monolithic structure makes maintenance and testing difficult.

## Goals
1.  **Code Organization:** Split the monolith into reusable classes/modules.
2.  **Maintainability:** Simplify `main.cpp` to act primarily as an orchestrator.
3.  **Extensibility:** Make it easier to add new UI pages or support different hardware variants.

## Detailed Requirements

### Refactoring Strategy
- **`DisplayManager`:** Encapsulate all `Arduino_GFX` logic, page drawing, and progress bars.
- **`InputHandler`:** Move the `Button` class and screen timeout logic.
- **`NetworkManager`:** Handle `WiFiManager`, `PubSubClient` (MQTT), and persistent configuration.
- **`SyncManager`:** Manage SD card initialization and file operations.

### Structure (C++)
- Use a `src/` and `include/` header-only or header/source split approach.
- Leverage singleton or instance-based patterns for managers.

## Non-Functional Requirements
- **Zero Regression:** The refactored code must behave identically to the current monolith.
- **Binary Size:** Ensure the modularized structure doesn't significantly increase Flash usage.
