# Changelog

All notable changes to the KiraFlux Toolkit from version 0.0.0 to 0.3.1 are documented in this file.

## [0.3.1] – 2026-04-03

### Changed

- **Configuration structures now aggregate** – `ArduinoIIC::BusConfig`, `ArduinoSPI::BusConfig` and `ArduinoSPI::NodeConfig` are now true aggregates (no user‑defined constructors). Static factory methods `create()` replace the previous constructors. This enables designated initializers and improves constexpr usage. Existing code must be updated to call `Config::create(...)` instead of direct construction. ([#146](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/146))

### Fixed

- **ExponentialFilter initial transient** – The filter no longer starts from zero; the first input value is returned unchanged, eliminating an unwanted initial transient. ([#149](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/149))
- **MizLangBridge::poll() inverted condition** – Fixed a logic error where a successful read was treated as failure, causing incorrect error returns. ([#148](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/148))
- **ArithmeticPositiveOnlyAdjuster missing return** – The `adjustImpl` function now correctly returns the clamped value instead of falling through. ([#147](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/147))
- **UI::Page::link() constexpr and nodiscard** – The `link()` method is now `constexpr` and marked `[[nodiscard]]` to prevent accidental discarding of the returned widget reference. ([#145](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/145))

---

[0.3.1]: https://github.com/KiraFlux/KiraFlux-Toolkit/releases/tag/v0.3.1

## [0.3.0] – 2026-03-28

### Breaking Changes

- **UI Framework Rewrite** – the monolithic `kf/UI.hpp` has been replaced by a modular system under `kf/ui/`. Widgets, pages, renderers, and events are now separate components. The public API has changed significantly.
- **GPIO Abstraction Rework** – moved to a CRTP‑based static interface ([`kf/gpio/GPIO.hpp`](src/kf/gpio/GPIO.hpp)). Arduino‑specific implementations live in [`kf/gpio/arduino.hpp`](src/kf/gpio/arduino.hpp). Existing code that directly accessed `digitalRead` etc. must be updated to use the new `DigitalInput`, `DigitalOutput`, `AdcInput`, and `PwmOutput` classes.
- **Bus (I2C/SPI) Redesign** – new CRTP base classes ([`kf/bus/Bus.hpp`](src/kf/bus/Bus.hpp), [`kf/bus/iic/IIC.hpp`](src/kf/bus/iic/IIC.hpp), [`kf/bus/spi/SPI.hpp`](src/kf/bus/spi/SPI.hpp)) and concrete implementations `ArduinoIIC` ([`kf/bus/iic/ArduinoIIC.hpp`](src/kf/bus/iic/ArduinoIIC.hpp)) and `ArduinoSPI` ([`kf/bus/spi/ArduinoSPI.hpp`](src/kf/bus/spi/ArduinoSPI.hpp)). The old approach of passing `TwoWire` or `SPIClass` directly is no longer supported.
- **Timer Classes Removed** – [`kf/math/time/Chronometer.hpp`](src/kf/math/time/Chronometer.hpp), [`kf/math/time/TimeoutManager.hpp`](src/kf/math/time/TimeoutManager.hpp), and [`kf/math/time/Timer.hpp`](src/kf/math/time/Timer.hpp) have been replaced by a single [`kf::math::Timer`](src/kf/math/Timer.hpp) class with a simpler, unified interface.
- **Result and Option API Changes** – [`kf::Result`](src/kf/Result.hpp) and [`kf::Option`](src/kf/Option.hpp) now require trivially copyable/destructible types. The APIs have changed: `ok()` and `error()` are gone; use `value()` and `error()` directly (which abort if not applicable). `Option` now has move constructors and a `value(T&&)` setter.
- **ArrayString::format Returns Result** – [`kf::memory::ArrayString::format`](src/kf/memory/ArrayString.hpp) now returns `Result<usize, Error>` instead of just `usize`, to signal truncation or format errors.
- **Event Class Rename** – [`kf::ui::Event`](src/kf/ui/Event.hpp) renamed its `Type` enumeration to `Kind` and changed the template parameter from `u8` to `usize`.
- **Vector Class Renaming** – `vec2` and `vec3` have been renamed to `Vector2` and `Vector3` and moved into the `kf::math` namespace ([`kf/math/Vector2.hpp`](src/kf/math/Vector2.hpp), [`kf/math/Vector3.hpp`](src/kf/math/Vector3.hpp)).
- **AnalogAxis Removed** – [`kf/input/AnalogAxis.hpp`](src/kf/input/AnalogAxis.hpp) is removed; replaced by [`kf::drivers::sensors::NormalizedAdcInput`](src/kf/drivers/sensors/NormalizedAdcInput.hpp), which uses the new GPIO interface.
- **Joystick Moved** – [`kf/input/Joystick.hpp`](src/kf/input/Joystick.hpp) is removed; replaced by [`kf::drivers::sensors::Joystick`](src/kf/drivers/sensors/Joystick.hpp), which now follows the `Sensor` static interface.
- **Tuner Rewrite** – [`kf/Tuner.hpp`](src/kf/Tuner.hpp) is removed; replaced by [`kf/tuner/Tuner.hpp`](src/kf/tuner/Tuner.hpp) and [`kf/tuner/SampleCollectingTuner.hpp`](src/kf/tuner/SampleCollectingTuner.hpp) with a cleaner state machine.
- **Actuators Directory Rename** – `kf/drivers/zms/` has been renamed to `kf/drivers/actuators/`. All motor and servo classes moved accordingly.
- **Input/Output Streams Removal** – [`kf/memory/io/InputStream.hpp`](src/kf/memory/io/InputStream.hpp) and [`kf/memory/io/OutputStream.hpp`](src/kf/memory/io/OutputStream.hpp) are removed; replaced by [`kf/io/Readable.hpp`](src/kf/io/Readable.hpp) and [`kf/io/Writable.hpp`](src/kf/io/Writable.hpp) CRTP interfaces.
- **Singleton Moved** – [`kf/pattern/Singleton.hpp`](src/kf/pattern/Singleton.hpp) moved to [`kf/mixin/Singleton.hpp`](src/kf/mixin/Singleton.hpp) and now inherits from `kf::mixin::NonCopyable`.
- **Bit/Function Traits Moved** – [`kf/core/bit_traits.hpp`](src/kf/core/bit_traits.hpp) moved to [`kf/meta/BitTraits.hpp`](src/kf/meta/BitTraits.hpp); [`kf/core/function_traits.hpp`](src/kf/core/function_traits.hpp) moved to [`kf/meta/FunctionTraits.hpp`](src/kf/meta/FunctionTraits.hpp).
- **gfx.hpp Removed** – the convenience header [`kf/gfx.hpp`](src/kf/gfx.hpp) has been removed; include individual graphics headers directly.

### Added

#### Core & Utilities
- **`kf::Range`** – a validated closed interval with `clamp()` ([`kf/Range.hpp`](src/kf/Range.hpp)).
- **`kf::memory::Slice`** – lightweight non‑owning view of contiguous memory, with array constructor and const‑conversion ([`kf/memory/Slice.hpp`](src/kf/memory/Slice.hpp)).
- **`kf::Function`** – type‑erased callable with small‑buffer optimisation (improved version in [`kf/Function.hpp`](src/kf/Function.hpp)).
- **`kf::linearMap`** – linear interpolation between ranges ([`kf/algorithm.hpp`](src/kf/algorithm.hpp)).
- **`kf::meta::BitTraits`** – compile‑time bit‑width helpers ([`kf/meta/BitTraits.hpp`](src/kf/meta/BitTraits.hpp)).
- **`kf::math::Timer`** – unified timer for one‑shot, periodic, and stopwatch use ([`kf/math/Timer.hpp`](src/kf/math/Timer.hpp)).
- **`kf::validation`** – new `Validator` and `Validatable` CRTP for self‑validating configurations, with the `KF_VALIDATOR_CHECK` macro ([`kf/validation.hpp`](src/kf/validation.hpp)).

#### GPIO & Bus
- **`kf::gpio` static interfaces** – `Input`, `Output`, `DigitalInput`, `AdcInput`, `PwmOutput` ([`kf/gpio/GPIO.hpp`](src/kf/gpio/GPIO.hpp)).
- **`kf::bus` hierarchy** – `Bus`, `IIC`, `SPI` with concrete `ArduinoIIC` and `ArduinoSPI` implementations ([`kf/bus/Bus.hpp`](src/kf/bus/Bus.hpp), `kf/bus/iic/`, `kf/bus/spi/`).
- **`kf::io::Readable` and `kf::io::Writable`** – CRTP interfaces for byte streams ([`kf/io/Readable.hpp`](src/kf/io/Readable.hpp), [`kf/io/Writable.hpp`](src/kf/io/Writable.hpp)).

#### Drivers & Sensors
- **`kf::drivers::sensors::Sensor`** and **`kf::drivers::actuators::Actuator`** static interfaces ([`kf/drivers/sensors/Sensor.hpp`](src/kf/drivers/sensors/Sensor.hpp), [`kf/drivers/actuators/Actuator.hpp`](src/kf/drivers/actuators/Actuator.hpp)).
- **`kf::drivers::sensors::NormalizedAdcInput`** – filtered, calibrated analog axis ([`kf/drivers/sensors/NormalizedAdcInput.hpp`](src/kf/drivers/sensors/NormalizedAdcInput.hpp)).
- **`kf::drivers::sensors::Joystick`** – two‑axis joystick with built‑in tuner ([`kf/drivers/sensors/Joystick.hpp`](src/kf/drivers/sensors/Joystick.hpp)).
- **`kf::drivers::actuators::L298nMotor`** – PWM + direction motor driver ([`kf/drivers/actuators/Motor.hpp`](src/kf/drivers/actuators/Motor.hpp)).
- **`kf::drivers::actuators::PwmPositionServo`** – configurable servo driver with safe angle range ([`kf/drivers/actuators/PwmPositionServo.hpp`](src/kf/drivers/actuators/PwmPositionServo.hpp)).
- **`kf::controllers::Manipulator2DOF`** – now templated on actuator types ([`kf/controllers/Manipulator2DOF.hpp`](src/kf/controllers/Manipulator2DOF.hpp)).

#### Graphics & Display
- **`kf::drivers::display::Orientation`** – enum for display rotations ([`kf/drivers/display/Orientation.hpp`](src/kf/drivers/display/Orientation.hpp)).
- **`kf::image::ViewportImage::transposed()`** – property to swap logical width/height ([`kf/image/ViewportImage.hpp`](src/kf/image/ViewportImage.hpp)).
- **`kf::gfx::Palette`** – now exposes named colours (`black`, `bright_white`, etc.) and `ansiColor()` helper ([`kf/gfx/Palette.hpp`](src/kf/gfx/Palette.hpp)).

#### UI Framework
- Modular UI under `kf/ui/`:
  - **`kf::ui::UI`** – main controller (singleton) with event queue and page management ([`kf/ui/UI.hpp`](src/kf/ui/UI.hpp)).
  - **`kf::ui::Event<N>`** – packed event type ([`kf/ui/Event.hpp`](src/kf/ui/Event.hpp)).
  - **Widgets**: `Button`, `CheckBox`, `ComboBox`, `Display`, `Labeled`, `Slider`, `SpinBox` (each in `kf/ui/widgets/`).
  - **Renderers**: `PlainTextRender` (console) and `ColoredTextRender` (ANSI colours) (`kf/ui/render/`).
- **`kf::ui::internal::Adjuster`** – CRTP for different spin‑box step modes ([`kf/ui/internal/Adjuster.hpp`](src/kf/ui/internal/Adjuster.hpp)).
- **`kf::ui::internal::Placement`** – controls visibility of slider value ([`kf/ui/internal/Placement.hpp`](src/kf/ui/internal/Placement.hpp)).

#### Mixins & Utilities
- New mixins in `kf/mixin/`:
  - `NonCopyable`, `Initable`, `Resettable`, `Pollable`, `TimedPollable`, `Quitable`, `Configurable`, `Callbacked`, `Labeled`, `ValueCallbacked`, `Singleton`.
- **`kf::Logger`** – now without timestamp, with `create()` and levels (info, warn, error, debug) ([`kf/Logger.hpp`](src/kf/Logger.hpp)).
- **`kf::memory::Storage`** – wrapper for ESP32 Preferences ([`kf/memory/Storage.hpp`](src/kf/memory/Storage.hpp)).

#### Examples
- New examples covering all major modules:
  - GPIO ([`examples/misc/gpio/`](examples/misc/gpio/))
  - I2C scanner ([`examples/bus/iic-arduino/`](examples/bus/iic-arduino/))
  - Displays (ST7735, SSD1306) ([`examples/display/`](examples/display/))
  - Motor (L293N) ([`examples/actuators/motor-l293n/`](examples/actuators/motor-l293n/))
  - Servo ([`examples/actuators/pwm-servo-motor/`](examples/actuators/pwm-servo-motor/))
  - Button ([`examples/input/pin-button/`](examples/input/pin-button/))
  - Joystick ([`examples/input/joystick/`](examples/input/joystick/))
  - Timer ([`examples/math/timer/`](examples/math/timer/))
  - Vectors ([`examples/math/vectors/`](examples/math/vectors/))
  - Slice ([`examples/misc/slice/`](examples/misc/slice/))
  - Logger ([`examples/misc/logger/`](examples/misc/logger/))
  - Validation ([`examples/misc/validation/`](examples/misc/validation/))
  - Tuner ([`examples/misc/tuner/`](examples/misc/tuner/))
  - ESP‑NOW ([`examples/network/espnow-ping/`](examples/network/espnow-ping/))
  - MizLangBridge ([`examples/network/mizlang-bridge/`](examples/network/mizlang-bridge/))
  - UI (simple and with graphics) ([`examples/ui/`](examples/ui/))

#### Documentation
- **`CONTRIBUTING.MD`** – detailed guidelines for development, code style, branching, and testing.
- **`README.MD`** – updated to reflect new architecture and module overview.
- Doxygen comments added to public APIs.

#### CI & Tooling
- Linting with `clang-tidy` (identifier naming checks) via GitHub Actions.
- Unit tests restructured under `test/unit/` with per‑module subdirectories; run via `make test`.
- Example builds parallelised, with per‑example `makefile`s and root `make build-examples`.
- Scripts: `find-examples.sh`, `find-test-suites.sh`, `collect-files.sh`, `generate-compile-db.sh`, `lint-file.sh`, `new-template.py`, `shapshot.py`.

### Changed

- **Code style** – enforced via `.clang-format` and `.clang-tidy`. Naming: `PascalCase` for types, `camelBack` for functions/variables, private fields start with `_`.
- **Namespaces** – aligned with directory structure (e.g., `kf::ui`, `kf::math`, `kf::io`, `kf::drivers`, etc.).
- **`kf::Function`** – now non‑copyable, move‑only; improved small‑buffer handling.
- **`kf::Logger::writer`** – now a static callback; removed dependency on `millis()`.
- **`kf::memory::ArrayString`** – `append()` for floats now handles `NaN`/`inf`; `format()` returns `Result`.
- **`kf::image::DynamicImage`** – `fill()` now correctly swaps coordinates when needed.
- **`kf::ui::Event`** – sign‑extension fixed; `kind()` replaces `type()`.
- **`kf::ui::PlainTextRender`** (formerly `TextBufferRender`) – improved cursor handling, centering, and slider rendering.
- **`kf::drivers::display::DisplayDriver`** – `init()`, `send()`, `orientation()` now return `bool` to indicate success.
- **`kf::network::EspNow`** – `Peer` now implements `Writable`, uses `onReceiveFromUnknown` instead of `setUnknownReceiveHandler` ([`kf/network/EspNow.hpp`](src/kf/network/EspNow.hpp)).
- **`kf::network::MizLangBridge`** – now templated on `Readable` and `Writable` types ([`kf/network/MizLangBridge.hpp`](src/kf/network/MizLangBridge.hpp)).
- **`kf::input::Button`** – uses `DigitalInput` and `TimedPollable` ([`kf/input/Button.hpp`](src/kf/input/Button.hpp)).
- **`kf::input::JoystickListener`** – uses `Timer` and `Configurable` ([`kf/input/JoystickListener.hpp`](src/kf/input/JoystickListener.hpp)).
- **`kf::math::PID`** – now uses `Configurable` and `Resettable` ([`kf/math/PID.hpp`](src/kf/math/PID.hpp)).
- **`kf::math::filters`** – all filters inherit from `Filter` and use `Configurable`/`Resettable` (`kf/math/filters/`).
- **`kf::tuner::SampleCollectingTuner`** – now inherits from `Tuner` and uses `Pollable`/`Resettable` ([`kf/tuner/SampleCollectingTuner.hpp`](src/kf/tuner/SampleCollectingTuner.hpp)).

### Removed

- **Monolithic `kf/UI.hpp`** – replaced by modular UI.
- **Old timer classes** – `kf/math/time/Chronometer.hpp`, `kf/math/time/TimeoutManager.hpp`, `kf/math/time/Timer.hpp`.
- **`kf/Tuner.hpp`** – replaced by `kf/tuner/Tuner.hpp`.
- **`kf/input/AnalogAxis.hpp`** – superseded by `NormalizedAdcInput`.
- **`kf/input/Joystick.hpp`** – superseded by `Joystick` in `drivers/sensors`.
- **`kf/drivers/zms/`** – renamed to `actuators`.
- **`kf/drivers/sensors/Encoder.hpp`** – moved to `input/Encoder.hpp`.
- **`kf/pattern/Singleton.hpp`** – moved to `mixin/Singleton.hpp`.
- **`kf/memory/io/InputStream.hpp` and `OutputStream.hpp`** – replaced by `Readable`/`Writable`.
- **`kf/core/bit_traits.hpp`** – moved to `meta/BitTraits.hpp`.
- **`kf/core/function_traits.hpp`** – moved to `meta/FunctionTraits.hpp`.
- **`kf/gfx.hpp`** – file‑includer removed; include specific headers directly.
- **`kf/ui/TextBufferRender.hpp`** – renamed to `PlainTextRender.hpp`.
- **`kf/ui/detail/`** – internal details moved into widget‑specific files.
- **`kf/ui/StepMode.hpp`** – replaced by `Adjuster` mechanism.
- **`kf/ui/StepAdjuster.hpp` / `ValueAdjuster.hpp`** – integrated into `Adjuster` hierarchy.

### Fixed

- **SSD1306 driver** – buffer overflow in `sendImpl()` fixed; added error handling ([`kf/drivers/display/SSD1306.hpp`](src/kf/drivers/display/SSD1306.hpp)).
- **EspNow** – strict‑aliasing violation in `onReceive` fixed by copying MAC address bytes ([`kf/network/EspNow.hpp`](src/kf/network/EspNow.hpp)).
- **`DynamicImage::fill`** – swapped coordinates now handled correctly ([`kf/image/DynamicImage.hpp`](src/kf/image/DynamicImage.hpp)).
- **`ArrayString::append`** – handling of `NaN`, `inf`, and integer overflow corrected ([`kf/memory/ArrayString.hpp`](src/kf/memory/ArrayString.hpp)).
- **`kf::linearMap`** – correct implementation and tests added ([`kf/algorithm.hpp`](src/kf/algorithm.hpp)).
- **`PID::calc`** – replaced `isnan` macro with `std::isnan` ([`kf/math/PID.hpp`](src/kf/math/PID.hpp)).
- **`Logger`** – removed unused timestamp; now safe to call without `millis()` ([`kf/Logger.hpp`](src/kf/Logger.hpp)).
- **`Validator`** – macro `KF_VALIDATOR_CHECK` logs correctly and increments error counter ([`kf/validation.hpp`](src/kf/validation.hpp)).
- **`kf::ui::Event`** – sign‑extension for negative values now works ([`kf/ui/Event.hpp`](src/kf/ui/Event.hpp)).
- **`kf::ui::PlainTextRender`** – title centering, newline handling, and slider value placement fixed ([`kf/ui/render/PlainTextRender.hpp`](src/kf/ui/render/PlainTextRender.hpp)).
- **`kf::gpio::arduino::DigitalInput`** – inversion logic corrected ([`kf/gpio/arduino.hpp`](src/kf/gpio/arduino.hpp)).
- **`kf::tuner::SampleCollectingTuner`** – state machine transition fixed ([`kf/tuner/SampleCollectingTuner.hpp`](src/kf/tuner/SampleCollectingTuner.hpp)).
- **`kf::memory::Storage`** – load/save/erase now return correct success status ([`kf/memory/Storage.hpp`](src/kf/memory/Storage.hpp)).
- **`kf::image::DynamicImage::sub`** – bounds checking corrected ([`kf/image/DynamicImage.hpp`](src/kf/image/DynamicImage.hpp)).

### Documentation

- Added `CONTRIBUTING.MD` with full development guidelines.
- Updated `README.MD` with module table and quick start.
- Added Doxygen comments to all public headers.
- Added per‑example `makefile`s and root `make` targets.

---

[0.3.0]: https://github.com/KiraFlux/KiraFlux-Toolkit/releases/tag/v0.3.0