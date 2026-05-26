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
- **C++17** – the library targets C++17, with limited use of newer features.
- **ESP32 (Arduino framework)** – the primary target, though many modules are platform‑agnostic.
- **Polymorphism** – use **CRTP** for static polymorphism when compile‑time knowledge is sufficient; use **dynamic polymorphism (virtual functions)** only where runtime flexibility is genuinely required (e.g., UI widgets, type‑erased callables).
- **Error handling** – never throw exceptions; use `Result<T, E>` and `Option<T>` instead.
- **Performance** – prefer `constexpr`, `noexcept`, and `[[nodiscard]]` where appropriate.
- **Testability** – algorithmic parts must be testable on native (x86) with Unity; hardware‑dependent code should be isolated and have separate hardware tests.
- **Minimal dependencies** – keep `Arduino.h` out of algorithmic code; use `kf::clamp`, `std::isnan`, etc. instead of Arduino‑specific macros.

---

## Repository Workflow

### Branches
- `main` – **stable branch**. Only for releases. Direct pushes are forbidden.
- `dev` – **development branch**. All changes land here via **squash‑merged** feature branches. Protected from deletion and force push.
- Feature branches – name them `feature/short-description` or `fix/short-description`. Create from `dev`.

### Issues and Milestones
- Every task must have a corresponding **Issue**.
- Large tasks should be split into sub‑issues under a parent Issue (meta‑issue). Use the label `meta` for parent issues.
- Mark sub‑issues as `ready` when they are ready for review/merge.
- Use **Milestones** for version planning (e.g., `v0.3.0`).

### Pull Requests
1. Create a feature branch from the latest `dev`.
2. After finishing the work, open a Pull Request targeting `dev`.
3. In the PR description, list the Issues it closes (`Closes #...`).
4. Before merging, **run tests locally** and ensure CI passes.
5. Merging into `dev` uses **squash and merge** (one commit with a clear message).
6. When enough changes accumulate in `dev`, create a PR into `main` with a regular **merge commit** (not squash). Update the version in `library.json` and create a Git tag.

---

## Code Style

### Formatting
- Follow the rules defined in `.clang-format` and `.clang-tidy`. Run `clang-format` before committing.
- To format a single file: `clang-format -i <file>`.
- To format the whole codebase: `find src/kf -name "*.hpp" -o -name "*.cpp" | xargs clang-format -i`.

### Naming
- Classes, structs, template parameters – `PascalCase`.
- Functions, variables, fields – `camelCase`.
- Private fields – leading underscore (`_field`).
- Constants and macros – `UPPER_SNAKE_CASE`.

### Key Rules
- Use `[[nodiscard]]` for all functions whose result should not be ignored (e.g., methods returning `Option`/`Result`, success booleans).
- Add `constexpr` and `noexcept` when possible.
- **Namespaces and `using`**:
  - Avoid `using namespace` in the global scope of headers.
  - Type aliases (e.g., `using u8 = uint8_t;`) are allowed inside the `kf` namespace.
  - `using` declarations for individual names from `std` (e.g., `using std::abs;`) are allowed inside the `kf` namespace.
  - Do not place `using` declarations in the global namespace in public headers.
- For standard headers, use `#include <...>`; for own headers, use `#include "..."`.

### Comments and Documentation
- Public API must be documented with **Doxygen** style:
  ```cpp
  /// @brief Brief description.
  /// @param param1 Description of parameter.
  /// @return Description of return value.
  ```
- Code comments are in **English**, brief, explaining **why** (the code itself explains what).
- Private/internal methods need not be documented if their purpose is obvious.

---

## Architectural Conventions

### CRTP (Curiously Recurring Template Pattern)

Use CRTP for static polymorphism. The base class provides an interface that calls `methodImpl()` of Impl:
```cpp
template<typename Impl> struct Base {    
    void doSomething() { static_cast<Impl *>(this)->doSomethingImpl(); }

    void doOther() const { static_cast<const Impl *>(this)->doOtherImpl(); }

    static void classMethod() { Impl::classMethodImpl(); }
};

struct MyBase : Base<MyBase> {

private:
    // impl 
    
    friend struct Base<MyBase>;

    void doSomethingImpl() { /* ... */ }
    
    void doOtherImpl() const { /* ... */ }

    static void classMethodImpl() { /* ... */ }
};
```

### Mixins

Reusable behaviours are implemented as CRTP mixins in `kf/mixin/`.  
Each mixin provides a macro (e.g., `KF_IMPL_RESETTABLE`) to establish friendship.  
Mixins are meant to be inherited by the class that needs the behaviour.

Example:
```cpp
struct MyClass : mixin::Resettable<MyClass> {
private:
    KF_IMPL_RESETTABLE(MyClass);
    void resetImpl() noexcept { /* custom reset logic */ }
};
```

### Error Handling
- `Option<T>` – optional value (like `std::optional`).
- `Result<T, E>` – operation that may fail (like `std::expected`). Use `isOk()` / `isError()` to check.
- Do not throw exceptions; prefer returning `Result` or `Option`.

### Validation
- Use `Validator` and `Validatable` CRTP for self‑validating configurations.
- Use the macro `KF_VALIDATOR_CHECK(validator, logger, condition)` inside `checkImpl()`.

### Hardware Abstraction
- Isolate hardware‑dependent code in separate headers (e.g., `kf/gpio/arduino.hpp`).
- Algorithmic code should not directly include `<Arduino.h>`; use portable abstractions.

### Testing
- Unit tests (native) in `test/unit/` using **Unity** framework.
- Hardware tests are currently not implemented. When added, they will be placed in `test/hardware/` and run with `pio test -e esp32dev` (skipped in CI if no board connected).

---

## Development Process

### Steps for a New Feature or Fix
1. **Create an Issue** (or find an existing one). For complex tasks, create a parent Issue and sub‑issues.
2. **Create a branch** from `dev`:
   ```bash
   git checkout dev && git pull && git checkout -b feature/name
   ```
3. **Make changes** following the code style.
4. **Write tests**:
   - For algorithmic code – in `test/unit/` with appropriate subdirectory.
   - For new examples – in `examples/`.
5. **Test locally**:
   ```bash
   make test        # run unit tests
   # If you have hardware connected: pio test -e esp32dev
   ```
6. **Push the branch** and open a Pull Request targeting `dev`.
7. **Wait for CI to pass** (GitHub Actions runs linter, unit tests, example builds).
8. **Get approval** (from yourself or another developer) and perform a **squash merge** into `dev`.
9. **Close related Issues** – they will close automatically if the PR description includes `Closes #...`.
10. For sub‑issues, mark them as `ready` (e.g., comment or move to `Ready` column on project board).

### Syncing with `dev`
Before creating a PR, always update your branch:
```bash
git fetch origin
git rebase origin/dev
```
Resolve conflicts locally. Force push is allowed only on your own feature branches.

---

## Testing

### Unit Tests (native)
- Located in `test/unit/` with subdirectories by module (`common/`, `math/`, `ui/`, etc.).
- Use the **Unity** test framework. Include `unity.h` and define `main()` that runs tests.
- Run: `make test` (or `pio test -e native` directly).

### Hardware Tests (embedded)
- Hardware tests are currently not implemented. When added, they will be placed in `test/hardware/`.
- To run hardware tests on a connected board: `pio test -e esp32dev`.
- In CI, hardware tests are skipped (by `test_ignore` or `test_filter`).

### Examples
- Each example is a separate project in `examples/` with its own `platformio.ini` and a `makefile` providing convenient shortcuts (`make`, `make upload`, `make monitor`, etc.).
- Examples must be self‑contained and demonstrate a specific component.
- Code in examples should be clean and include brief comments (in English).
- After library changes, all examples must compile (checked in CI). To build all examples from the root, use `make build-examples` or `make be`.

---

## Documentation

- **README.md** – overview, quick start, modules table, license.
- **CONTRIBUTING.md** – this file.
- **CHANGELOG.md** – maintained during release preparation, listing changes since the last release.
- **Public API** – documented with Doxygen comments directly in headers.
- **Examples** – serve as the primary usage documentation.

---

## Versioning and Releases

This project is in **0.x** development stage. Until version 1.0, 
**MINOR** version increments may include breaking changes. 
**PATCH** increments are reserved for bug fixes and non‑breaking improvements.
Version is specified in `library.json`.

- Before merging `dev` into `main`:
  1. Update version in `library.json` (bump MINOR for significant changes, PATCH for fixes).
  2. Update `CHANGELOG.md` with a summary of changes.
  3. Create a PR from `dev` to `main` (regular merge commit, not squash).
  4. After merging, create a Git tag matching the version (e.g., version `0.3.0` → tag `v0.3.0`) and publish a release on GitHub.

---

## Useful Scripts

The repository includes several helper scripts in the root directory.  
**For everyday development, use the provided `make` targets** – they wrap the scripts with sensible defaults.

### Root Makefile Targets

| Make target                        | Description                                                                                 |
| ---------------------------------- | ------------------------------------------------------------------------------------------- |
| `make test`                        | Runs all unit tests (native).                                                               |
| `make build-examples` or `make be` | Builds all example projects.                                                                |
| `make lint`                        | Runs clang‑tidy style checks on the entire codebase.                                        |
| `make snapshot` or `make s`       | Generates a `snapshot.txt` with a full code dump (used for debugging or external analysis). |
| `make new-example` or `make ne`    | Creates a new example skeleton. Requires `group` and `name` variables.                      |
| `make new-test` or `make nt`       | Creates a new unit test skeleton. Requires `group` and `name` variables.                    |
| `make monitor`                     | Opens the PlatformIO serial monitor (default baud: 115200).                                 |
| `make all`                         | Alias for `make test`.                                                                      |

**Example usage:**
```bash
# Run unit tests
make test

# Build all examples
make be

# Create a new example in examples/input/my-example
make ne group=input name=my-example

# Create a new test in test/unit/math/test_mything
make nt group=math name=mything

# Run style checks
make lint
```

### Example‑specific Makefiles

Each example project in `examples/` contains its own `makefile` with common shortcuts:

| Target                     | Description                              |
| -------------------------- | ---------------------------------------- |
| `make` or `make all`       | Build the example.                       |
| `make clean` or `make c`   | Clean the build.                         |
| `make upload` or `make u`  | Build and upload to the connected board. |
| `make monitor` or `make m` | Open the serial monitor (baud: 115200).  |

These per‑example makefiles are useful when working on a single example. For bulk operations (building all examples, running all tests) use the root `make` targets.

---

Thank you for contributing to KiraFlux Toolkit!