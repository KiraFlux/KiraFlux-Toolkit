# Contributing to KiraFlux Toolkit

Thank you for your interest in contributing! This document provides comprehensive guidelines for developing the KiraFlux Toolkit library.

## Table of Contents
- [Core Principles](#core-principles)
- [Repository Workflow](#repository-workflow)
- [Code Style](#code-style)
- [Architectural Conventions](#architectural-conventions)
- [Development Process](#development-process)
- [Testing](#testing)
- [Documentation](#documentation)
- [Versioning and Releases](#versioning-and-releases)
- [Useful Scripts](#useful-scripts)

---

## Core Principles

- **Header‑only** – all code lives in headers; no `.cpp` files except for tests and examples.
- **C++17** – the library targets C++17 (GCC native / xtensa for ESP32). No C++20 features are used.
- **ESP32 (Arduino framework)** – primary target, many modules are platform‑agnostic.
- **Polymorphism** – prefer CRTP for static polymorphism; use virtual functions only where runtime flexibility is genuinely required (UI widgets, type‑erased callables).
- **Error handling** – never throw exceptions. Use `Result<T, E>` and `Option<T>` exclusively. Create via `kf::ok()`, `kf::error()`, `kf::some()`, `kf::none`. Use `isSome()`/`isNone()`, `unwrap()`/`unwrapOr()`, `map()`. `kf::Function` is never empty – use `Option<Function<...>>` for optional callbacks.
- **Performance** – prefer `constexpr`, `noexcept`, `[[nodiscard]]`. Use `alignas` for small buffer optimisation where applicable.
- **Testability** – algorithmic code must be testable natively (x86) with Unity; hardware‑dependent code isolated.
- **Minimal dependencies** – algorithmic code must not directly include `<Arduino.h>`; use portable abstractions (`kf::clamp`, `std::isnan`, etc.).
- **No default constructors for resource‑dependent types** – types that require external resources (framebuffer, display, hardware configuration) must not provide a default constructor. They must be constructed with all necessary dependencies.
- **Single internal namespace** – use `kf::internal` for all implementation details. Avoid nested `internal` namespaces inside modules.

---

## Repository Workflow

### Branches
- `main` – stable branch, only for releases. Direct pushes forbidden.
- `dev` – development branch. All changes land here via **regular merge commits** (never squash).
- Feature branches – name `feature/short-description` or `fix/short-description`. Create from `dev`.

### Issues and Milestones
- Every task must have a corresponding Issue.
- Large tasks (epics) use label `epic`. Sub‑issues are linked to the epic.
- Mark sub‑issues as `ready` when ready for review/merge.
- Use Milestones for version planning (e.g., `v0.3.2`).

### Pull Requests
1. Create a feature branch from latest `dev`.
2. Open a Pull Request targeting `dev`.
3. In PR description, list issues it closes (`Closes #...`).
4. Run tests locally and ensure CI passes.
5. Merge into `dev` using **regular merge commit** (not squash).
6. For releases: create PR from `dev` to `main` (regular merge), update `library.json`, create Git tag.

---

## Code Style

### Formatting
- Follow `.clang-format` and `.clang-tidy`. Run `clang-format` before committing:
  ```bash
  clang-format -i <file>
  # or whole codebase:
  ./clang-format-all.sh   # or make format
  ```

### Naming
- Classes, structs, template parameters – `PascalCase`.
- Functions, variables, fields – `camelCase`.
- Private fields – leading underscore (`_field`).
- Constants and macros – `UPPER_SNAKE_CASE`.
- **Type aliases for concrete implementations** – suffix `Impl` (e.g., `NodeImpl`, `PinImpl`, `RenderImpl`, `PixelImpl`).
- **Tag structs for CRTP interfaces** – suffix `Tag` (e.g., `GpioTag`, `SensorTag`).
- **Configuration instances in examples** – suffix `_config` (e.g., `pwm_config`, `bus_config`).

### Inheritance Formatting
Write each base class on its own line, with empty lines around the list:

```cpp
struct MyClass :

    Base1,
    Base2,
    Base3

{
    // ...
};
```

### Trailing Return Types
For functions with long or complex return types (especially `Result`, `Option`, `auto` with nested templates), use trailing return syntax:

```cpp
auto readBufferImpl(Slice<u8> buffer) noexcept -> Result<Slice<const u8>, Error>;
auto sub(...) const noexcept -> Result<Canvas, Error>;
auto map(F &&f) const & noexcept -> Option<decltype(f(std::declval<T>()))>;
```

### Initialisation Formatting
Use designated initialisers or aggregate initialisation with one field per line, aligned:

```cpp
PwmOutput::Config pwm_config{
    .frequency_hz = 30'000,
    .resolution_bits = 10,
    .pin = static_cast<kf::u8>(GPIO_NUM_26),
    .channel = 0,
};
```

### Include Order
1. System headers (e.g., `<Arduino.h>`)
2. Blank line
3. Project headers grouped by module, alphabetically within group

Use `#pragma once` in all headers.

### Key Rules
- Use `[[nodiscard]]` for functions whose result should not be ignored.
- Add `constexpr` and `noexcept` whenever possible.
- **Namespaces and `using`**:
  - Avoid `using namespace` in global scope of headers.
  - Type aliases (`using u8 = uint8_t;`) allowed inside `kf` namespace.
  - `using` declarations for individual `std` names (e.g., `using std::abs;`) allowed inside `kf`.
  - Do not place `using` in global namespace in public headers.
- For standard headers, use `#include <...>`; for own headers, use `#include "..."`.

### Comments and Documentation
- Public API must be documented with Doxygen style:
  ```cpp
  /// @brief Brief description.
  /// @param param1 Description.
  /// @return Description.
  ```
- Code comments in English, brief, explaining **why**.
- Private/internal methods need not be documented if obvious.

---

## Architectural Conventions

### CRTP (Curiously Recurring Template Pattern)

Base class calls `methodImpl()` of derived:

```cpp
template<typename Impl> struct Base {    
    void doSomething() { static_cast<Impl *>(this)->doSomethingImpl(); }
    void doOther() const { static_cast<const Impl *>(this)->doOtherImpl(); }
    static void classMethod() { Impl::classMethodImpl(); }
};

struct MyBase : Base<MyBase> {
private:
    friend struct Base<MyBase>;
    void doSomethingImpl() { /* ... */ }
    void doOtherImpl() const { /* ... */ }
    static void classMethodImpl() { /* ... */ }
};
```

Always define a tag struct (e.g., `MyBaseTag`) for interface checking.

### Mixins

Reusable behaviours are CRTP mixins in `kf/mixin/`. Each provides a macro `KF_IMPL_*` that declares `friend struct` of the base. Place the macro in the `private` section of the derived class.

Available mixins:
- `NonCopyable`, `Initable`, `Resettable`, `Pollable`, `TimedPollable`, `Quitable`, `Configurable`
- `Callbacked<Args...>` (supports variable arguments)
- `ValueCallbacked<T>`
- `Labeled`
- `Singleton`
- `Invariant` – adds `isSome()`/`isNone()`
- `StringRepresentable<Impl, StringType>` – adds `toString()`
- `MacAddressed` – for objects that own a MAC address

Example:
```cpp
struct MyClass : mixin::Resettable<MyClass> {
private:
    KF_IMPL_RESETTABLE(MyClass);
    void resetImpl() noexcept { /* ... */ }
};
```

### Error Handling
- `Option<T>` – create with `kf::some(value)` or `kf::none`. Check `isSome()`/`isNone()`. Access with `unwrap()` (aborts if None) or `unwrapOr(default)`.
- `Result<T, E>` – create with `kf::ok(value)` or `kf::error(error)`. Check `isOk()`/`isError()`. Access value with `ok()` (aborts if error), error with `error()` (aborts if ok). Use `map()` and `mapError()`.
- Never throw exceptions.
- `kf::Function<R(Args...)>` is **never empty**. For optional callbacks, use `Option<Function<...>>`.

### Validation
- Use `Validator` and `Validatable` CRTP for self‑validating configurations.
- Use macro `KF_VALIDATOR_CHECK(validator, logger, condition)` inside `checkImpl()`.

### Hardware Abstraction
- Isolate hardware‑dependent code in separate headers (e.g., `kf/gpio/ArduinoGPIO.hpp`).
- Algorithmic code must not directly include `<Arduino.h>`.
- Use single namespace `kf::internal` for implementation details.
- For ISRs, use `IRAM_ATTR` attribute and mark shared variables as `volatile`.

### UI Framework Guidelines
- `UI` is **not a singleton** – instantiate with a renderer.
- `Page` constructor receives a reference to the `UI` instance.
- Renderers are passed by reference and must outlive the `UI`.
- For hardware displays, the renderer callback should perform screen update.

### Type Traits and Checks
- Use `static_assert` to enforce type properties (e.g., trivially copyable, destructible) in generic containers.
- Use `KF_CHECK_IMPL(T, RequiredTag)` to verify that a template parameter implements a required CRTP interface.

### Performance
- Use `alignas` for small buffer optimisation in type-erased wrappers (e.g., `Function`).
- Prefer `constexpr` factory methods (e.g., `static constexpr Config create(...)`) over public constructors for configuration structures when possible.

### Testing
- Unit tests (native) in `test/unit/` using Unity framework.
- For tracking construction/destruction in tests, use `inline static` counters (see `test/structures.hpp`).
- Hardware tests not yet implemented; they will be in `test/hardware/`.

---

## Development Process

### Steps for a New Feature or Fix
1. **Create an Issue** (or find existing). For complex tasks, create an `epic` issue and sub‑issues.
2. **Create a branch** from `dev`:
   ```bash
   git checkout dev && git pull && git checkout -b feature/name
   ```
3. **Make changes** following code style.
4. **Write tests**:
   - Algorithmic code → `test/unit/<module>/test_*.cpp`
   - New examples → `examples/<group>/<name>/`
5. **Test locally**:
   ```bash
   make test
   # For hardware: pio test -e esp32dev (if board connected)
   ```
6. **Push branch** and open a Pull Request targeting `dev`.
7. **Wait for CI** (lint, unit tests, example builds).
8. **Get approval** and perform a **regular merge** into `dev`.
9. **Close issues** – automatically if PR description includes `Closes #...`.

### Syncing with `dev`
Always rebase your feature branch before PR:
```bash
git fetch origin
git rebase origin/dev
```
Resolve conflicts. Force push allowed only on feature branches.

---

## Testing

### Unit Tests (native)
- Located in `test/unit/` with subdirectories by module (`common/`, `math/`, `ui/`, etc.).
- Use Unity framework. Define `main()` that calls `UNITY_BEGIN()` and `RUN_TEST` macros.
- Run: `make test` or `pio test -e native`.

### Hardware Tests (embedded)
- Not yet implemented. Future: `test/hardware/`, run with `pio test -e esp32dev`.

### Examples
- Each example is a separate PlatformIO project in `examples/` with its own `platformio.ini` and `makefile`.
- Must be self‑contained and compile after library changes.
- Build all examples: `make build-examples` or `make be`.

---

## Documentation

- **README.md** – overview, modules table, quick start, license.
- **CONTRIBUTING.md** – this file.
- **CHANGELOG.md** – updated before each release, grouped by Added/Changed/Fixed/Removed.
- **Public API** – Doxygen comments in headers.
- **Examples** – primary usage documentation.

---

## Versioning and Releases

- **0.x** – MINOR may include breaking changes, PATCH for fixes.
- Version specified in `library.json`.
- Before merging `dev` into `main`:
  1. Bump version in `library.json`.
  2. Update `CHANGELOG.md`.
  3. Create PR to `main` (regular merge commit).
  4. After merge, create Git tag (e.g., `v0.3.2`) and GitHub release.

---

## Useful Scripts

### Root Makefile Targets

| Target                       | Description                                       |
| ---------------------------- | ------------------------------------------------- |
| `make test`                  | Run unit tests (native).                          |
| `make build-examples` / `be` | Build all examples.                               |
| `make lint`                  | Run clang‑tidy style checks.                      |
| `make snapshot` / `s`        | Generate `snapshot.txt` (full code dump).         |
| `make format` / `f`          | Run `clang-format-all.sh` to format code.         |
| `make new-example` / `ne`    | Create new example (requires `group` and `name`). |
| `make new-test` / `nt`       | Create new test (requires `group` and `name`).    |
| `make monitor`               | Open serial monitor (115200 baud).                |
| `make all`                   | Alias for `make test`.                            |

Example usage:
```bash
make test
make be
make ne group=input name=my-example
make format
make lint
```

### Example‑specific Makefiles

Each example has its own `makefile` with the following shortcuts:

| Shortcut             | Description                          |
| -------------------- | ------------------------------------ |
| `make` / `all`       | Build the example.                   |
| `make clean` / `c`   | Clean build.                         |
| `make upload` / `u`  | Build and upload to connected board. |
| `make monitor` / `m` | Open serial monitor.                 |

These per‑example makefiles are useful when working on a single example.

---

Thank you for contributing to KiraFlux Toolkit!