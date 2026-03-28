# KiraFlux Toolkit

**Experimental header‑only C++17 library for ESP32 (Arduino framework)**  
A pragmatic blend of static polymorphism (CRTP) and dynamic polymorphism for embedded development.

## Overview

KiraFlux Toolkit provides a collection of modular components for ESP32, built around:

- **CRTP (Curiously Recurring Template Pattern)** – for zero‑cost abstractions where the type is known at compile time (GPIO, buses, drivers, filters, mixins).
- **Dynamic polymorphism (virtual functions)** – where runtime flexibility is genuinely required (UI widgets, page navigation, type‑erased callables via `kf::Function`).

The library is **header‑only**, written in **C++17**, and currently targets the **ESP32 with the Arduino framework**. Many core modules are platform‑agnostic.

**Key principles:**
- No exceptions, errno, or ignore – error handling via `Result<T, E>` and `Option<T>`.
- `[[nodiscard]]`, `constexpr`, `const`, `noexcept` where applicable.
- Minimal dependencies on `Arduino.h` – hardware‑independent code stays portable.
- Make testable as possible – implement unit tests run natively, hardware tests are separated.

## Status

**Experimental** – APIs are subject to change as we explore the limits of static polymorphism in embedded systems. Feedback and contributions are welcome.

## Modules

| Module           | Location           | Description                                                                                                     |
| ---------------- | ------------------ | --------------------------------------------------------------------------------------------------------------- |
| Core utilities   | `kf/` (root files) | Fundamental types: `Option`, `Result`, `Range`, `Logger`, `Function`, algorithms, type aliases, and validation. |
| GPIO             | `kf/gpio`          | Digital, analog, and PWM I/O abstractions with Arduino implementation.                                          |
| I/O streams      | `kf/io`            | Byte stream interfaces (`Readable`, `Writable`) and Arduino `Serial` wrapper.                                   |
| Buses            | `kf/bus`           | IIC and SPI bus abstractions with Arduino adapters.                                                             |
| Input processing | `kf/input`         | Button debouncing, rotary encoder, joystick direction handling.                                                 |
| Networking       | `kf/network`       | ESP‑NOW peer management and structured serial protocol (`MizLangBridge`).                                       |
| Drivers          | `kf/drivers`       | Actuators (servo, motor), sensors (Sharp, joystick, normalized ADC), display drivers (SSD1306, ST7735).         |
| Controllers      | `kf/controllers`   | Compositions of actuators (e.g., 2‑DOF manipulator).                                                            |
| Graphics         | `kf/gfx`           | Drawing canvas, bitmap fonts, color palettes.                                                                   |
| Images           | `kf/image`         | Static, dynamic, and viewport image buffers.                                                                    |
| Pixel formats    | `kf/pixel`         | Pixel format definitions (monochrome, RGB565) and operations.                                                   |
| UI               | `kf/ui`            | Widget‑based user interface with text renderers.                                                                |
| Math             | `kf/math`          | Vectors, timers, filters (EMA, low‑pass, complementary), PID controller.                                        |
| Memory           | `kf/memory`        | Lightweight containers (`Slice`, `ArrayString`, `Storage`) and STL container aliases.                           |
| Mixins           | `kf/mixin`         | Reusable CRTP mixins (`Resettable`, `Pollable`, `Initable`, `Configurable`, etc.).                              |
| Tuners           | `kf/tuner`         | Sensor calibration via sample collection and state machines.                                                    |
| Metaprogramming  | `kf/meta`          | CRTP base, bit/byte traits, function traits.                                                                    |

## Quick Start

### PlatformIO

Add the library to your `platformio.ini`:

```ini
lib_deps = https://github.com/KiraFlux/KiraFlux-Toolkit.git
```

Examples can be found in the [`examples`](examples) directory.

## Documentation

Public APIs are documented with **Doxygen comments** directly in the header files.

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.MD](CONTRIBUTING.MD) for guidelines.

## License

MIT License – see [LICENSE](LICENSE) file.