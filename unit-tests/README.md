# relayTester Unit Tests

This is a standalone CMake project dedicated to unit testing logic from the main relayTester codebase.

## Why this project exists

- It keeps test builds isolated from the production application build.
- It can reuse selected source files from the main project.
- It uses the existing wiringpi_wrapper mock implementation on non-Linux platforms.

## Build and run (Windows)

From `unit-tests` directory:

```bash
cmake --preset windows-debug
cmake --build --preset windows-debug
ctest --preset windows-debug
```

## Build and run (Raspberry Pi native)

From `unit-tests` directory:

```bash
cmake --preset raspi-native
cmake --build --preset raspi-native
ctest --preset raspi-native
```

## Current coverage

- `ContactSelector` GPIO and signal behavior with mock GPIO backend.
