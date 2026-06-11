# KiraFlux Toolkit

**Experimental header‑only C++17 library for ESP32 (Arduino framework)**  
A pragmatic blend of static polymorphism (CRTP) and dynamic polymorphism for embedded development.

## Overview

KiraFlux Toolkit provides a collection of modular components for ESP32, built around:

- **CRTP** - for zero‑cost abstractions (GPIO, buses, drivers, filters, mixins).
- **Dynamic polymorphism** - where runtime flexibility is required (UI widgets, type‑erased callables via `kf::Function`).

The library is **header‑only**, **C++17**, targets **ESP32 + Arduino framework**. Many core modules are platform‑agnostic.

**Key principles:**
- No exceptions - error handling via return values (always)
- `[[nodiscard]]`, `constexpr`, `noexcept` where applicable.
- Minimal hardware-specific dependencies.
- Make everything as testable as possible - unit tests run natively.

## Status

**Experimental** - APIs change between minor versions. Feedback and contributions welcome.

## Modules

| Module           | Location                               | Description                                                                                  |
| ---------------- | -------------------------------------- | -------------------------------------------------------------------------------------------- |
| Core utilities   | [`kf/`](src/kf)                        | `Option`, `Result`, `Range`, `Logger`, `Function`, algorithms, validation.                   |
| GPIO             | [`kf/gpio`](src/kf/gpio)               | Digital, analog, PWM I/O abstractions with Arduino backend.                                  |
| I/O streams      | [`kf/io`](src/kf/io)                   | `Readable`, `Writable`, `ArduinoStream` wrapper.                                             |
| Buses            | [`kf/bus`](src/kf/bus)                 | IIC and SPI abstractions with Arduino adapters.                                              |
| Input processing | [`kf/input`](src/kf/input)             | Button debouncing, joystick direction, encoders.                                             |
| Networking       | [`kf/network`](src/kf/network)         | ESP‑NOW peer management, structured serial protocol.                                         |
| Drivers          | [`kf/drivers`](src/kf/drivers)         | Actuators (servo, motor), sensors (Sharp, joystick, ADC), display drivers (SSD1306, ST7735). |
| Controllers      | [`kf/controllers`](src/kf/controllers) | Compositions of actuators (e.g., 2‑DOF manipulator).                                         |
| Graphics         | [`kf/gfx`](src/kf/gfx)                 | Drawing canvas, bitmap fonts, color palettes.                                                |
| Images           | [`kf/image`](src/kf/image)             | Static, dynamic, viewport image buffers.                                                     |
| Pixel formats    | [`kf/pixel`](src/kf/pixel)             | Monochrome, RGB565 pixel operations.                                                         |
| UI               | [`kf/ui`](src/kf/ui)                   | Widget‑based UI with configurable render backend.                                            |
| Math             | [`kf/math`](src/kf/math)               | Vectors, quaternion, timers, filters, PID.                                                   |
| Memory           | [`kf/memory`](src/kf/memory)           | Lightweight containers (`Slice`, `StaticString`, `Storage`) and STL aliases.                 |
| Mixins           | [`kf/mixin`](src/kf/mixin)             | Reusable CRTP mixins (`Resettable`, `Pollable`, `Callbacked`, etc.).                         |
| Tuners           | [`kf/tuner`](src/kf/tuner)             | Sensor calibration via sample collection.                                                    |
| Metaprogramming  | [`kf/meta`](src/kf/meta)               | CRTP base, bit/function traits.                                                              |

## Quick Start

### PlatformIO

```ini
lib_deps = https://github.com/KiraFlux/KiraFlux-Toolkit.git#0.3.2
```

Examples are in the [`examples`](examples) directory.

## Documentation

Public APIs are documented with **Doxygen comments** in the headers.

## Contributing

See [CONTRIBUTING.MD](CONTRIBUTING.MD).

## License

MIT - see [LICENSE](LICENSE) file.
