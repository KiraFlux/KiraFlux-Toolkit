# Changelog

All notable changes to the KiraFlux Toolkit from version 0.0.0 to 0.3.3 are documented in this file.

## [0.3.3] - 2026-06-14

### Breaking Changes

- [kf::ui::UiTraits](src/kf/ui/UiTraits.hpp) - now takes a single `Widget` template parameter instead of separate `Render` and `Event` types. The widget base class has been moved to [kf::ui::widgets::Widget<Render, Event>](src/kf/ui/widgets/Widget.hpp). All UI code must be updated accordingly.
- [UI::bindPage()](src/kf/ui/UI.hpp) renamed to `activePage()` - the method that sets the active page has been renamed.
- [Page::onUpdate()](src/kf/ui/UI.hpp) renamed to `onPoll()` - the page polling method name changed for consistency with other mixins.
- [Render::beginWidget()](src/kf/ui/render/Renderer.hpp) signature changed - now receives a `const Style&` instead of separate `foreground` and `background` colors. Custom renderers must update their `beginWidgetImpl()`.
- Render decoration/block methods replaced - removed `arrow()`, `colon()`, `beginBlock()`, `endBlock()`, `beginAltBlock()`, `endAltBlock()`. Added `decoration(Decoration)` and `beginBlock(Block)` / `endBlock(Block)` ([kf::ui::render::Render](src/kf/ui/render/Renderer.hpp)).
- [kf::gfx::Palette](src/kf/gfx/Palette.hpp) colour names renamed:
  - `red` -> `dark_red`
  - `green` -> `dark_green`
  - `yellow` -> `dark_yellow`
  - `blue` -> `dark_blue`
  - `purple` -> `dark_purple`
  - `cyan` -> `dark_cyan`
  - `white` -> `light_gray`
  - `bright_black` -> `dark_gray`
  - `bright_red` -> `light_red`
  - `bright_green` -> `light_green`
  - `bright_yellow` -> `light_yellow`
  - `bright_blue` -> `light_blue`
  - `bright_purple` -> `light_purple`
  - `bright_cyan` -> `light_cyan`
  - `bright_white` -> `white`
  Existing code using the old names will break.
- [kf::drivers::display::DisplayDriver](src/kf/drivers/display/DisplayDriver.hpp) now has a third template parameter `ResultType`. Built-in drivers ([SSD1306](src/kf/drivers/display/SSD1306.hpp), [ST7735](src/kf/drivers/display/ST7735.hpp)) now return `Result<void, Error>` instead of `bool`. Custom drivers must be updated.
- [Peer::create()](src/kf/network/EspNow.hpp) now requires a `Config` struct with `mac_address` and `wifi_interface_sta`. The internal peer registry has been removed; single callback for all peers. The peer-specific callback (`Peer::callback()`) no longer exists.

### Added

- [kf::math::Quaternion::toEulers()](src/kf/math/Quaternion.hpp) - converts a quaternion to Euler angles (roll, pitch, yaw) in radians.
- Semantic UI elements - [kf::ui::Block](src/kf/ui/Block.hpp) (`Standard`, `Alternative`) and [kf::ui::Decoration](src/kf/ui/Decoration.hpp) (`Arrow`, `Colon`) for layout and visual markers.
- [kf::ui::Color::Highlight](src/kf/ui/Color.hpp) - high-impact accent colour for special actions or highlights.
- [kf::ui::Color::Danger](src/kf/ui/Color.hpp) - alias for `Error`, used for destructive actions.
- [kf::ui::Style](src/kf/ui/Style.hpp) struct - groups `foreground_color` and `background_color` into a single style object.
- [kf::mixin::Styled](src/kf/mixin/Styled.hpp) - CRTP mixin providing `style()`, `foreground()`, `background()` for widgets and combo box items.
- [Page::selectedWidget()](src/kf/ui/UI.hpp) const version - now available as `const` overload.
- Per-item styling in [ComboBox](src/kf/ui/widgets/ComboBox.hpp) - each combo box item can have its own `Style`.
- `build_cache_dir` in all example `platformio.ini` - enables PlatformIO build caching for faster CI.
- [test/runner.hpp](test/runner.hpp) - unifies test entry point for both native and Arduino environments.

### Changed

- Math filters ([ComplementaryFilter](src/kf/math/filters/ComplementaryFilter.hpp), [ExponentialFilter](src/kf/math/filters/ExponentialFilter.hpp), [LowFrequencyFilter](src/kf/math/filters/LowFrequencyFilter.hpp)) now use `TrivialOption` internally instead of a `bool` flag, improving correctness.
- [SSD1306](src/kf/drivers/display/SSD1306.hpp) driver - all methods (`init()`, `setContrast()`, `setPower()`, `invert()`, `send()`, `orientation()`) now return `Result<void, Error>` instead of `bool`.
- [ST7735](src/kf/drivers/display/ST7735.hpp) driver - updated to the new display driver API; `init()`, `send()`, `orientation()` return `Result<void, Error>`.
- SPI and IIC bus nodes - added `using Error = ...` for clearer error type aliases ([kf::bus::spi::ArduinoSPI::Node](src/kf/bus/spi/ArduinoSPI.hpp), [kf::bus::iic::ArduinoIIC::Node](src/kf/bus/iic/ArduinoIIC.hpp)).
- [kf::Function](src/kf/Function.hpp) - internal buffer enlarged from 2 to 4 words; improved handling of empty state via a flag stored in the buffer.
- [kf::ui::CheckBox::onEventValue()](src/kf/ui/widgets/CheckBox.hpp) - now only triggers redraw when the value actually changes.
- [kf::ui::Labeled](src/kf/ui/widgets/Labeled.hpp) - restores widget colours after rendering the wrapped widget, avoiding colour leakage.
- Example projects - added `monitor_speed = 115200` where missing; `espnow-ping` example updated to the new ESP-NOW API.

### Fixed

- Focused highlight colour in `ColoredTextRender` - changed to dark purple for better visibility.
- [kf::ui::widgets::ComboBox](src/kf/ui/widgets/ComboBox.hpp) - removed redundant nested `Item` type alias.
- `kf::ui::simple` example - updated `ComboBox::Item` to `ComboBox::Config::Item`.
- [test/static_string](test/unit/common/test_static_string/static_string.cpp) - renamed ambiguous `access` namespace to `test_access`.

### Removed

- `kf::ui::Render::arrow()`, `colon()` - replaced by `decoration(Decoration)`.
- `kf::ui::Render::beginBlock()`, `endBlock()`, `beginAltBlock()`, `endAltBlock()` - replaced by `beginBlock(Block)` / `endBlock(Block)`.
- ESP-NOW peer registry - removed internal storage of peer references; unknown peer callback is now the only callback.

[0.3.3]: https://github.com/KiraFlux/KiraFlux-Toolkit/releases/tag/v0.3.3

## [0.3.2] - 2026-06-11

### Breaking Changes

- **UI is no longer a singleton** - `kf::ui::UI` must be instantiated explicitly with a renderer reference. The `instance()` method has been removed. ([#187](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/187))
- **`Canvas` and `DynamicImage` lost default constructors** - both classes now require explicit initialisation with a framebuffer or parent image. ([#183](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/183), [#184](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/184))
- **`ArrayString` renamed to `StaticString`** - `kf::memory::ArrayString<N>` is now `kf::memory::StaticString<N>`. All uses must be updated.
- **`L298nMotor` removed** - replaced by `kf::drivers::actuators::DRV8871` which uses two PWM outputs instead of PWM+digital direction.
- **`Button` removed** - replaced by `kf::input::LogicalLevelListener` (debounced digital input with click detection).
- **`Encoder` removed** - replaced by `kf::drivers::sensors::QuadratureEncoder` (4X decoding with interrupt support).
- **`kf/gpio/arduino.hpp` moved to `ArduinoGPIO.hpp`** - all GPIO types are now under `kf::gpio::ArduinoGPIO`.
- **`Timer` now requires a `Config` struct** - constructor `Timer(period)` is gone; use `Timer::Config{.period = ms}` instead. ([#161](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/161))
- **`JoystickListener` configuration changed** - `repeat_timeout` and `delay` are now nested `repeat_timer` and `delay_timer` of type `Timer::Config`.
- **`Option` and `Result` API overhaul** - construct via `kf::some()`, `kf::none`, `kf::someRef()`, `kf::someTrivial()`, `kf::ok()`, `kf::error()`. Methods renamed: `hasValue()` -> `isSome()`, `value()` -> `unwrap()`, `valueOr()` -> `unwrapOr()`. Added `map()` for `Option` and `map()`/`mapError()` for `Result`. Support for move‑only and non‑trivial types. **Trivial types** (including floats) should use `TrivialOption<T>` created with `kf::someTrivial()`. ([#171](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/171), [#172](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/172), [#178](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/178), [#179](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/179))
- **`ColoredTextRender` palette reworked** - now has configurable palettes (`normal_foreground_palette`, `focused_foreground_palette`, `normal_background_palette`, `focused_background_palette`). The previous hard‑coded ANSI mapping is gone.
- **`Function` is never empty** - default constructor removed; use `Option<Function<...>>` for optional callables. ([#181](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/181))
- **`io::Writable` template parameter changed** - now accepts a `ResultType` (usually `Result<void, Error>`) instead of `ErrorImpl`. ([#166](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/166))

### Added

#### Core & Utilities
- **`kf::Quaternion`** - quaternion for 3D rotations ([`src/kf/math/Quaternion.hpp`](src/kf/math/Quaternion.hpp)).
- **`kf::drivers::sensors::QuadratureEncoder`** - rotary encoder with 4X decoding and ISR support ([`src/kf/drivers/sensors/QuadratureEncoder.hpp`](src/kf/drivers/sensors/QuadratureEncoder.hpp)).
- **`kf::drivers::actuators::DRV8871`** - H‑bridge motor driver using two PWM outputs ([`src/kf/drivers/actuators/DRV8871.hpp`](src/kf/drivers/actuators/DRV8871.hpp)).
- **Option specialisations** - `Option<Slice<T>>`, `Option<Function<R(Args...)>>`, `Option<T&>` (reference), `TrivialOption<T>` for trivially copyable types, and real‑number specialisations for `float`, `double`, `long double`. Factory functions: `kf::some()`, `kf::someRef()`, `kf::someTrivial()`, `kf::none`.
- **`kf::NoneType` and `kf::none` constant** - tag type for constructing empty `Option` ([`src/kf/NoneType.hpp`](src/kf/NoneType.hpp)).
- **New mixins** - `mixin::Invariant` (adds `isSome()`/`isNone()`), `mixin::StringRepresentable` (adds `toString()`), `mixin::MacAddressed` (for objects with a MAC address).
- **`Callbacked` now supports variable arguments** - `Callbacked<Args...>`.
- **`kf::Range`** - validated closed interval with `clamp()` ([`src/kf/Range.hpp`](src/kf/Range.hpp)).
- **`kf::linearMap`** - linear interpolation between ranges ([`src/kf/algorithm.hpp`](src/kf/algorithm.hpp)).
- **Validation framework** - `Validator` and `Validatable` CRTP with `KF_VALIDATOR_CHECK` macro ([`src/kf/validation.hpp`](src/kf/validation.hpp)).
- **`make format` / `make f` target** - runs `clang-format-all.sh` to format code.

#### UI & Graphics
- **`ColoredTextRender` - configurable palettes** - four palettes for normal/focused state, foreground/background.
- **`Page::selectedWidget()`** - returns the currently focused widget as an `Option<const Widget &>` ([#177](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/177)).
- **UI ignores duplicate `Update` events** - adding consecutive `Update` events no longer floods the queue ([#156](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/156)).
- **`Widget::hint()` and colour properties** - widgets now store a hint string and foreground/background colours (semantic `Color`).
- **`Canvas::glyph()`** - draw a single glyph at specified position ([#154](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/154)).
- **`Canvas::State`** - groups active font, colours, and auto‑wrap flag; removed default constructor ([#183](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/183)).
- **Named colours in `gfx::Palette`** - `black`, `red`, `green`, `blue`, `bright_white`, etc.

#### Documentation & Examples
- **Updated `CONTRIBUTING.MD`** - added rules for inheritance formatting, trailing return types, `Impl` suffix, single `kf::internal` namespace, and other conventions.
- **Updated `README.MD`** - fixed module table, added quaternion entry, fixed `lib_deps` to pin version `#0.3.2`.
- **Renamed example** - `examples/actuators/motor-l293n/` -> `examples/actuators/motor-drv8871/` (new example using `DRV8871`).
- **All examples updated to new API** - use `kf::some()`, `unwrap()`, `ok()`, `error()`, new `Timer`, `JoystickListener`, `EspNow`, `LogicalLevelListener` syntax, etc.

#### CI & Tooling
- **Simplified `lint.yml`** - removed `generate-db` and `collect-files` jobs; linting now runs a single `./lint.sh`.
- **Fixed script name** - `shapshot.py` -> `snapshot.py`; corresponding make target `snapshot` (was `shapshot`).
- **Added `clang-format-all.sh`** - for bulk code formatting.

### Changed

- **`kf::Function`** - now never empty; use `Option<Function<...>>` for optional callbacks. Improved small‑buffer handling (2 words).
- **`kf::Slice`** - moved from `kf::memory::Slice` to `kf::Slice` (top‑level).
- **`kf::ui::Placement`** - moved from `internal::Placement` to `kf::ui::Placement`.
- **`kf::input::LogicalLevelListener`** - fixed first sample logic; now correctly debounces without initial false click.
- **`kf::math::PID`** - replaced `NaN` sentinel with `TrivialOption<f32>` for previous error value.
- **`kf::network::EspNow`** - `Peer::add()` replaced by `Peer::create()`; callbacks use `Callbacked` mixin; unknown peer callback via `EspNow::instance().callback(...)`. Added `MacAddress` type and `EspNowError` with `toString()`.
- **`kf::network::MizLangBridge`** - `poll()` now correctly returns error only on actual failure (fixed inverted condition) ([#148](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/148)).
- **`kf::memory::Storage`** - fixed return status for `load()`/`save()`/`erase()`.
- **`kf::io::Writable`** - template parameter changed to `ResultType` (e.g., `Result<void, Error>`).
- **Internal namespaces** - unified under `kf::internal` (no more nested `internal` inside modules).
- **`kf::gfx::Font`** - font data moved to `kf/gfx/fonts/gyver_5x7.hpp`; removed `extern` declaration (now `constexpr` in header).
- **`kf::image::DynamicImage`** - error `BufferNotInit` replaced by `BufferTooSmall`; removed `isValid()` method.
- **`kf::math::Vector2`/`Vector3`** - now use `TrivialOption` for `divChecked()` and `normalized()`; added `zero()` static method.
- **`kf::Logger`** - writer is now a static callback; removed dependency on `millis()`; uses `StaticString` internally.

### Fixed

- **`UI::Page::cursor` not clamped after layout change** - cursor now respects new widget list bounds ([#158](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/158)).
- **`ArithmeticPositiveOnlyAdjuster` missing return** - fixed fall‑through that caused undefined behaviour ([#147](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/147)).
- **`Canvas` and `DynamicImage` compilation without default constructor** - all code updated to construct explicitly.
- **`EspNow` strict‑aliasing violation** - fixed by copying MAC address bytes instead of reinterpreting pointers.
- **`PID::calc`** - replaced `isnan` macro with `std::isnan`.
- **`DynamicImage::fill`** - coordinate swapping corrected.
- **`SSD1306`** - buffer overflow in `sendImpl()` fixed.
- **`kf::linearMap`** - correct implementation and tests added.
- **`kf::ui::PlainTextRender`** - title centering, newline handling, and slider value placement fixes.
- **`kf::gpio::ArduinoGPIO::DigitalInput`** - inversion logic corrected.
- **`kf::tuner::SampleCollectingTuner`** - state machine transition fixed.
- **`kf::image::DynamicImage::sub`** - bounds checking corrected.
- **`kf::memory::Storage`** - `load()`/`save()`/`erase()` now correctly return success status.
- **`kf::io::ArduinoStream`** - fixed error handling in `readBufferImpl()`.

### Removed

- **`L298nMotor`** (use `DRV8871` instead).
- **`Button`** (use `LogicalLevelListener` instead).
- **`Encoder`** (use `QuadratureEncoder` instead).
- **`ArrayString`** (renamed to `StaticString`; old name removed).
- **`kf/gpio/arduino.hpp`** (use `ArduinoGPIO.hpp`).
- **Default constructors of `Canvas` and `DynamicImage`**.
- **`kf/ui/StepMode.hpp`, `StepAdjuster.hpp`, `ValueAdjuster.hpp`** (integrated into `Adjuster` hierarchy).
- **`kf/ui/TextBufferRenderer.hpp`** (renamed to `PlainTextRenderer.hpp`).
- **Old timer classes** - `kf/math/time/Chronometer.hpp`, `TimeoutManager.hpp`, `Timer.hpp` (replaced by single `kf::math::Timer`).
- **`kf/Tuner.hpp`** (replaced by `kf/tuner/Tuner.hpp` and `SampleCollectingTuner.hpp`).
- **`kf/input/AnalogAxis.hpp`** (superseded by `NormalizedAdcInput`).
- **`kf/input/Joystick.hpp`** (moved to `drivers/sensors`).
- **`kf/drivers/zms/`** (renamed to `actuators`).
- **`kf/pattern/Singleton.hpp`** (moved to `mixin/Singleton.hpp`).
- **`kf/memory/io/InputStream.hpp` and `OutputStream.hpp`** (replaced by `Readable`/`Writable`).
- **`kf/core/bit_traits.hpp` and `function_traits.hpp`** (moved to `meta/`).
- **`kf/gfx.hpp`** (removed; include individual graphics headers).

[0.3.2]: https://github.com/KiraFlux/KiraFlux-Toolkit/releases/tag/v0.3.2

---

## [0.3.1] - 2026-04-03

### Changed

- **Configuration structures now aggregate** - `ArduinoIIC::BusConfig`, `ArduinoSPI::BusConfig` and `ArduinoSPI::NodeConfig` are now true aggregates (no user‑defined constructors). Static factory methods `create()` replace the previous constructors. This enables designated initializers and improves constexpr usage. Existing code must be updated to call `Config::create(...)` instead of direct construction. ([#146](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/146))

### Fixed

- **ExponentialFilter initial transient** - The filter no longer starts from zero; the first input value is returned unchanged, eliminating an unwanted initial transient. ([#149](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/149))
- **MizLangBridge::poll() inverted condition** - Fixed a logic error where a successful read was treated as failure, causing incorrect error returns. ([#148](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/148))
- **ArithmeticPositiveOnlyAdjuster missing return** - The `adjustImpl` function now correctly returns the clamped value instead of falling through. ([#147](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/147))
- **UI::Page::link() constexpr and nodiscard** - The `link()` method is now `constexpr` and marked `[[nodiscard]]` to prevent accidental discarding of the returned widget reference. ([#145](https://github.com/KiraFlux/KiraFlux-Toolkit/issues/145))

---

[0.3.1]: https://github.com/KiraFlux/KiraFlux-Toolkit/releases/tag/v0.3.1

## [0.3.0] - 2026-03-28

### Breaking Changes

- **UI Framework Rewrite** - the monolithic `kf/UI.hpp` has been replaced by a modular system under `kf/ui/`. Widgets, pages, renderers, and events are now separate components. The public API has changed significantly.
- **GPIO Abstraction Rework** - moved to a CRTP‑based static interface ([`kf/gpio/GPIO.hpp`](src/kf/gpio/GPIO.hpp)). Arduino‑specific implementations live in [`kf/gpio/arduino.hpp`](src/kf/gpio/arduino.hpp). Existing code that directly accessed `digitalRead` etc. must be updated to use the new `DigitalInput`, `DigitalOutput`, `AdcInput`, and `PwmOutput` classes.
- **Bus (I2C/SPI) Redesign** - new CRTP base classes ([`kf/bus/Bus.hpp`](src/kf/bus/Bus.hpp), [`kf/bus/iic/IIC.hpp`](src/kf/bus/iic/IIC.hpp), [`kf/bus/spi/SPI.hpp`](src/kf/bus/spi/SPI.hpp)) and concrete implementations `ArduinoIIC` ([`kf/bus/iic/ArduinoIIC.hpp`](src/kf/bus/iic/ArduinoIIC.hpp)) and `ArduinoSPI` ([`kf/bus/spi/ArduinoSPI.hpp`](src/kf/bus/spi/ArduinoSPI.hpp)). The old approach of passing `TwoWire` or `SPIClass` directly is no longer supported.
- **Timer Classes Removed** - [`kf/math/time/Chronometer.hpp`](src/kf/math/time/Chronometer.hpp), [`kf/math/time/TimeoutManager.hpp`](src/kf/math/time/TimeoutManager.hpp), and [`kf/math/time/Timer.hpp`](src/kf/math/time/Timer.hpp) have been replaced by a single [`kf::math::Timer`](src/kf/math/Timer.hpp) class with a simpler, unified interface.
- **Result and Option API Changes** - [`kf::Result`](src/kf/Result.hpp) and [`kf::Option`](src/kf/Option.hpp) now require trivially copyable/destructible types. The APIs have changed: `ok()` and `error()` are gone; use `value()` and `error()` directly (which abort if not applicable). `Option` now has move constructors and a `value(T&&)` setter.
- **ArrayString::format Returns Result** - [`kf::memory::ArrayString::format`](src/kf/memory/ArrayString.hpp) now returns `Result<usize, Error>` instead of just `usize`, to signal truncation or format errors.
- **Event Class Rename** - [`kf::ui::Event`](src/kf/ui/Event.hpp) renamed its `Type` enumeration to `Kind` and changed the template parameter from `u8` to `usize`.
- **Vector Class Renaming** - `vec2` and `vec3` have been renamed to `Vector2` and `Vector3` and moved into the `kf::math` namespace ([`kf/math/Vector2.hpp`](src/kf/math/Vector2.hpp), [`kf/math/Vector3.hpp`](src/kf/math/Vector3.hpp)).
- **AnalogAxis Removed** - [`kf/input/AnalogAxis.hpp`](src/kf/input/AnalogAxis.hpp) is removed; replaced by [`kf::drivers::sensors::NormalizedAdcInput`](src/kf/drivers/sensors/NormalizedAdcInput.hpp), which uses the new GPIO interface.
- **Joystick Moved** - [`kf/input/Joystick.hpp`](src/kf/input/Joystick.hpp) is removed; replaced by [`kf::drivers::sensors::Joystick`](src/kf/drivers/sensors/Joystick.hpp), which now follows the `Sensor` static interface.
- **Tuner Rewrite** - [`kf/Tuner.hpp`](src/kf/Tuner.hpp) is removed; replaced by [`kf/tuner/Tuner.hpp`](src/kf/tuner/Tuner.hpp) and [`kf/tuner/SampleCollectingTuner.hpp`](src/kf/tuner/SampleCollectingTuner.hpp) with a cleaner state machine.
- **Actuators Directory Rename** - `kf/drivers/zms/` has been renamed to `kf/drivers/actuators/`. All motor and servo classes moved accordingly.
- **Input/Output Streams Removal** - [`kf/memory/io/InputStream.hpp`](src/kf/memory/io/InputStream.hpp) and [`kf/memory/io/OutputStream.hpp`](src/kf/memory/io/OutputStream.hpp) are removed; replaced by [`kf/io/Readable.hpp`](src/kf/io/Readable.hpp) and [`kf/io/Writable.hpp`](src/kf/io/Writable.hpp) CRTP interfaces.
- **Singleton Moved** - [`kf/pattern/Singleton.hpp`](src/kf/pattern/Singleton.hpp) moved to [`kf/mixin/Singleton.hpp`](src/kf/mixin/Singleton.hpp) and now inherits from `kf::mixin::NonCopyable`.
- **Bit/Function Traits Moved** - [`kf/core/bit_traits.hpp`](src/kf/core/bit_traits.hpp) moved to [`kf/meta/BitTraits.hpp`](src/kf/meta/BitTraits.hpp); [`kf/core/function_traits.hpp`](src/kf/core/function_traits.hpp) moved to [`kf/meta/FunctionTraits.hpp`](src/kf/meta/FunctionTraits.hpp).
- **gfx.hpp Removed** - the convenience header [`kf/gfx.hpp`](src/kf/gfx.hpp) has been removed; include individual graphics headers directly.

### Added

#### Core & Utilities
- **`kf::Range`** - a validated closed interval with `clamp()` ([`kf/Range.hpp`](src/kf/Range.hpp)).
- **`kf::memory::Slice`** - lightweight non‑owning view of contiguous memory, with array constructor and const‑conversion ([`kf/memory/Slice.hpp`](src/kf/memory/Slice.hpp)).
- **`kf::Function`** - type‑erased callable with small‑buffer optimisation (improved version in [`kf/Function.hpp`](src/kf/Function.hpp)).
- **`kf::linearMap`** - linear interpolation between ranges ([`kf/algorithm.hpp`](src/kf/algorithm.hpp)).
- **`kf::meta::BitTraits`** - compile‑time bit‑width helpers ([`kf/meta/BitTraits.hpp`](src/kf/meta/BitTraits.hpp)).
- **`kf::math::Timer`** - unified timer for one‑shot, periodic, and stopwatch use ([`kf/math/Timer.hpp`](src/kf/math/Timer.hpp)).
- **`kf::validation`** - new `Validator` and `Validatable` CRTP for self‑validating configurations, with the `KF_VALIDATOR_CHECK` macro ([`kf/validation.hpp`](src/kf/validation.hpp)).

#### GPIO & Bus
- **`kf::gpio` static interfaces** - `Input`, `Output`, `DigitalInput`, `AdcInput`, `PwmOutput` ([`kf/gpio/GPIO.hpp`](src/kf/gpio/GPIO.hpp)).
- **`kf::bus` hierarchy** - `Bus`, `IIC`, `SPI` with concrete `ArduinoIIC` and `ArduinoSPI` implementations ([`kf/bus/Bus.hpp`](src/kf/bus/Bus.hpp), `kf/bus/iic/`, `kf/bus/spi/`).
- **`kf::io::Readable` and `kf::io::Writable`** - CRTP interfaces for byte streams ([`kf/io/Readable.hpp`](src/kf/io/Readable.hpp), [`kf/io/Writable.hpp`](src/kf/io/Writable.hpp)).

#### Drivers & Sensors
- **`kf::drivers::sensors::Sensor`** and **`kf::drivers::actuators::Actuator`** static interfaces ([`kf/drivers/sensors/Sensor.hpp`](src/kf/drivers/sensors/Sensor.hpp), [`kf/drivers/actuators/Actuator.hpp`](src/kf/drivers/actuators/Actuator.hpp)).
- **`kf::drivers::sensors::NormalizedAdcInput`** - filtered, calibrated analog axis ([`kf/drivers/sensors/NormalizedAdcInput.hpp`](src/kf/drivers/sensors/NormalizedAdcInput.hpp)).
- **`kf::drivers::sensors::Joystick`** - two‑axis joystick with built‑in tuner ([`kf/drivers/sensors/Joystick.hpp`](src/kf/drivers/sensors/Joystick.hpp)).
- **`kf::drivers::actuators::L298nMotor`** - PWM + direction motor driver ([`kf/drivers/actuators/Motor.hpp`](src/kf/drivers/actuators/Motor.hpp)).
- **`kf::drivers::actuators::PwmPositionServo`** - configurable servo driver with safe angle range ([`kf/drivers/actuators/PwmPositionServo.hpp`](src/kf/drivers/actuators/PwmPositionServo.hpp)).
- **`kf::controllers::Manipulator2DOF`** - now templated on actuator types ([`kf/controllers/Manipulator2DOF.hpp`](src/kf/controllers/Manipulator2DOF.hpp)).

#### Graphics & Display
- **`kf::drivers::display::Orientation`** - enum for display rotations ([`kf/drivers/display/Orientation.hpp`](src/kf/drivers/display/Orientation.hpp)).
- **`kf::image::ViewportImage::transposed()`** - property to swap logical width/height ([`kf/image/ViewportImage.hpp`](src/kf/image/ViewportImage.hpp)).
- **`kf::gfx::Palette`** - now exposes named colours (`black`, `bright_white`, etc.) and `ansiColor()` helper ([`kf/gfx/Palette.hpp`](src/kf/gfx/Palette.hpp)).

#### UI Framework
- Modular UI under `kf/ui/`:
  - **`kf::ui::UI`** - main controller (singleton) with event queue and page management ([`kf/ui/UI.hpp`](src/kf/ui/UI.hpp)).
  - **`kf::ui::Event<N>`** - packed event type ([`kf/ui/Event.hpp`](src/kf/ui/Event.hpp)).
  - **Widgets**: `Button`, `CheckBox`, `ComboBox`, `Display`, `Labeled`, `Slider`, `SpinBox` (each in `kf/ui/widgets/`).
  - **Renderers**: `PlainTextRender` (console) and `ColoredTextRender` (ANSI colours) (`kf/ui/render/`).
- **`kf::ui::internal::Adjuster`** - CRTP for different spin‑box step modes ([`kf/ui/internal/Adjuster.hpp`](src/kf/ui/internal/Adjuster.hpp)).
- **`kf::ui::internal::Placement`** - controls visibility of slider value ([`kf/ui/internal/Placement.hpp`](src/kf/ui/internal/Placement.hpp)).

#### Mixins & Utilities
- New mixins in `kf/mixin/`:
  - `NonCopyable`, `Initable`, `Resettable`, `Pollable`, `TimedPollable`, `Quitable`, `Configurable`, `Callbacked`, `Labeled`, `ValueCallbacked`, `Singleton`.
- **`kf::Logger`** - now without timestamp, with `create()` and levels (info, warn, error, debug) ([`kf/Logger.hpp`](src/kf/Logger.hpp)).
- **`kf::memory::Storage`** - wrapper for ESP32 Preferences ([`kf/memory/Storage.hpp`](src/kf/memory/Storage.hpp)).

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
- **`CONTRIBUTING.MD`** - detailed guidelines for development, code style, branching, and testing.
- **`README.MD`** - updated to reflect new architecture and module overview.
- Doxygen comments added to public APIs.

#### CI & Tooling
- Linting with `clang-tidy` (identifier naming checks) via GitHub Actions.
- Unit tests restructured under `test/unit/` with per‑module subdirectories; run via `make test`.
- Example builds parallelised, with per‑example `makefile`s and root `make build-examples`.
- Scripts: `find-examples.sh`, `find-test-suites.sh`, `collect-files.sh`, `generate-compile-db.sh`, `lint-file.sh`, `new-template.py`, `shapshot.py`.

### Changed

- **Code style** - enforced via `.clang-format` and `.clang-tidy`. Naming: `PascalCase` for types, `camelBack` for functions/variables, private fields start with `_`.
- **Namespaces** - aligned with directory structure (e.g., `kf::ui`, `kf::math`, `kf::io`, `kf::drivers`, etc.).
- **`kf::Function`** - now non‑copyable, move‑only; improved small‑buffer handling.
- **`kf::Logger::writer`** - now a static callback; removed dependency on `millis()`.
- **`kf::memory::ArrayString`** - `append()` for floats now handles `NaN`/`inf`; `format()` returns `Result`.
- **`kf::image::DynamicImage`** - `fill()` now correctly swaps coordinates when needed.
- **`kf::ui::Event`** - sign‑extension fixed; `kind()` replaces `type()`.
- **`kf::ui::PlainTextRender`** (formerly `TextBufferRender`) - improved cursor handling, centering, and slider rendering.
- **`kf::drivers::display::DisplayDriver`** - `init()`, `send()`, `orientation()` now return `bool` to indicate success.
- **`kf::network::EspNow`** - `Peer` now implements `Writable`, uses `onReceiveFromUnknown` instead of `setUnknownReceiveHandler` ([`kf/network/EspNow.hpp`](src/kf/network/EspNow.hpp)).
- **`kf::network::MizLangBridge`** - now templated on `Readable` and `Writable` types ([`kf/network/MizLangBridge.hpp`](src/kf/network/MizLangBridge.hpp)).
- **`kf::input::Button`** - uses `DigitalInput` and `TimedPollable` ([`kf/input/Button.hpp`](src/kf/input/Button.hpp)).
- **`kf::input::JoystickListener`** - uses `Timer` and `Configurable` ([`kf/input/JoystickListener.hpp`](src/kf/input/JoystickListener.hpp)).
- **`kf::math::PID`** - now uses `Configurable` and `Resettable` ([`kf/math/PID.hpp`](src/kf/math/PID.hpp)).
- **`kf::math::filters`** - all filters inherit from `Filter` and use `Configurable`/`Resettable` (`kf/math/filters/`).
- **`kf::tuner::SampleCollectingTuner`** - now inherits from `Tuner` and uses `Pollable`/`Resettable` ([`kf/tuner/SampleCollectingTuner.hpp`](src/kf/tuner/SampleCollectingTuner.hpp)).

### Removed

- **Monolithic `kf/UI.hpp`** - replaced by modular UI.
- **Old timer classes** - `kf/math/time/Chronometer.hpp`, `kf/math/time/TimeoutManager.hpp`, `kf/math/time/Timer.hpp`.
- **`kf/Tuner.hpp`** - replaced by `kf/tuner/Tuner.hpp`.
- **`kf/input/AnalogAxis.hpp`** - superseded by `NormalizedAdcInput`.
- **`kf/input/Joystick.hpp`** - superseded by `Joystick` in `drivers/sensors`.
- **`kf/drivers/zms/`** - renamed to `actuators`.
- **`kf/drivers/sensors/Encoder.hpp`** - moved to `input/Encoder.hpp`.
- **`kf/pattern/Singleton.hpp`** - moved to `mixin/Singleton.hpp`.
- **`kf/memory/io/InputStream.hpp` and `OutputStream.hpp`** - replaced by `Readable`/`Writable`.
- **`kf/core/bit_traits.hpp`** - moved to `meta/BitTraits.hpp`.
- **`kf/core/function_traits.hpp`** - moved to `meta/FunctionTraits.hpp`.
- **`kf/gfx.hpp`** - file‑includer removed; include specific headers directly.
- **`kf/ui/TextBufferRenderer.hpp`** - renamed to `PlainTextRenderer.hpp`.
- **`kf/ui/detail/`** - internal details moved into widget‑specific files.
- **`kf/ui/StepMode.hpp`** - replaced by `Adjuster` mechanism.
- **`kf/ui/StepAdjuster.hpp` / `ValueAdjuster.hpp`** - integrated into `Adjuster` hierarchy.

### Fixed

- **SSD1306 driver** - buffer overflow in `sendImpl()` fixed; added error handling ([`kf/drivers/display/SSD1306.hpp`](src/kf/drivers/display/SSD1306.hpp)).
- **EspNow** - strict‑aliasing violation in `onReceive` fixed by copying MAC address bytes ([`kf/network/EspNow.hpp`](src/kf/network/EspNow.hpp)).
- **`DynamicImage::fill`** - swapped coordinates now handled correctly ([`kf/image/DynamicImage.hpp`](src/kf/image/DynamicImage.hpp)).
- **`ArrayString::append`** - handling of `NaN`, `inf`, and integer overflow corrected ([`kf/memory/ArrayString.hpp`](src/kf/memory/ArrayString.hpp)).
- **`kf::linearMap`** - correct implementation and tests added ([`kf/algorithm.hpp`](src/kf/algorithm.hpp)).
- **`PID::calc`** - replaced `isnan` macro with `std::isnan` ([`kf/math/PID.hpp`](src/kf/math/PID.hpp)).
- **`Logger`** - removed unused timestamp; now safe to call without `millis()` ([`kf/Logger.hpp`](src/kf/Logger.hpp)).
- **`Validator`** - macro `KF_VALIDATOR_CHECK` logs correctly and increments error counter ([`kf/validation.hpp`](src/kf/validation.hpp)).
- **`kf::ui::Event`** - sign‑extension for negative values now works ([`kf/ui/Event.hpp`](src/kf/ui/Event.hpp)).
- **`kf::ui::PlainTextRender`** - title centering, newline handling, and slider value placement fixed ([`kf/ui/render/PlainTextRenderer.hpp`](src/kf/ui/render/PlainTextRenderer.hpp)).
- **`kf::gpio::arduino::DigitalInput`** - inversion logic corrected ([`kf/gpio/arduino.hpp`](src/kf/gpio/arduino.hpp)).
- **`kf::tuner::SampleCollectingTuner`** - state machine transition fixed ([`kf/tuner/SampleCollectingTuner.hpp`](src/kf/tuner/SampleCollectingTuner.hpp)).
- **`kf::memory::Storage`** - load/save/erase now return correct success status ([`kf/memory/Storage.hpp`](src/kf/memory/Storage.hpp)).
- **`kf::image::DynamicImage::sub`** - bounds checking corrected ([`kf/image/DynamicImage.hpp`](src/kf/image/DynamicImage.hpp)).

### Documentation

- Added `CONTRIBUTING.MD` with full development guidelines.
- Updated `README.MD` with module table and quick start.
- Added Doxygen comments to all public headers.
- Added per‑example `makefile`s and root `make` targets.

---

[0.3.0]: https://github.com/KiraFlux/KiraFlux-Toolkit/releases/tag/v0.3.0