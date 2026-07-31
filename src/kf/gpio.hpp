// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "kf/primitives.hpp"
#include "kf/units.hpp"

#include "kf/mixin/Configured.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"

#ifdef IRAM_ATTR
#define KF_PLACE_IRAM IRAM_ATTR
#else
#define KF_PLACE_IRAM
#endif

namespace kf::internal {

struct PwmOutputConfig final {
    u32 frequency_hz;  ///< PWM frequency (Hz)
    u8 resolution_bits;///< resolution in bits (1..16)

    /// @brief Maximum PWM value for given resolution (2^bits - 1)
    [[nodiscard]] constexpr u16 maxDuty() const noexcept {
        return static_cast<u16>((1u << (resolution_bits)) - 1u);
    }
};

}// namespace kf::internal

namespace kf::gpio {

/// @brief GPIO pin numbers for ESP32 (original chip, not S2/S3/C3)
/// @note Available pins: 0–19, 21–23, 25–27, 32–39
/// @note Pins 20, 24, 28, 29, 30, 31 are not physically present on the package
/// @note Pins 6–11 are typically used for internal SPI flash and may not be broken out on modules
/// @note Pins 34–39 are input‑only – no output driver, no internal pull‑up/down
/// @note ADC2 (pins 0, 2, 4, 12–15, 25–27) is unavailable when Wi‑Fi is active
/// @note Strapping pins (0, 2, 5, 12, 15) affect boot mode – handle carefully
enum class GpioNumber : u8 {

    /// @brief GPIO 0
    /// @note STRAPPING PIN, `ADC2_CH1`, `TOUCH`, `UART`, `PWM`
    G0 = 0,

    /// @brief GPIO 1
    /// @note `UART0_TX`, `ADC2_CH0`, `TOUCH`, `PWM`
    G1 = 1,

    /// @brief GPIO 2
    /// @note STRAPPING PIN, `ADC2_CH2`, `TOUCH`, `PWM`, `I2C`, `SPI`
    G2 = 2,

    /// @brief GPIO 3
    /// @note `UART0_RX`, `ADC2_CH3`, `TOUCH`, `PWM`
    G3 = 3,

    /// @brief GPIO 4
    /// @note `ADC2_CH4`, `TOUCH`, `PWM`, `I2C_SDA`
    G4 = 4,

    /// @brief GPIO 5
    /// @note STRAPPING PIN, `ADC2_CH5`, `TOUCH`, `PWM`, `SPI_SS`
    G5 = 5,

    /// @brief GPIO 6
    /// @note Reserved for internal SPI flash, `ADC2_CH6`, `TOUCH`, `PWM`, `SPI_MISO`
    G6 = 6,

    /// @brief GPIO 7
    /// @note Reserved for internal SPI flash, `ADC2_CH7`, `TOUCH`, `PWM`, `SPI_MOSI`
    G7 = 7,

    /// @brief GPIO 8
    /// @note Reserved for internal SPI flash, `ADC2_CH8`, `TOUCH`, `PWM`, `SPI_SCK`
    G8 = 8,

    /// @brief GPIO 9
    /// @note Reserved for internal SPI flash, `ADC2_CH9`, `TOUCH`, `PWM`, `SPI`
    G9 = 9,

    /// @brief GPIO 10
    /// @note Reserved for internal SPI flash, `ADC2_CH10`, `TOUCH`, `PWM`, `SPI`
    G10 = 10,

    /// @brief GPIO 11
    /// @note Reserved for internal SPI flash, `ADC2_CH11`, `TOUCH`, `PWM`, `SPI`
    G11 = 11,

    /// @brief GPIO 12
    /// @note STRAPPING PIN (VDD_SDIO voltage), `ADC2_CH12`, `TOUCH`, `PWM`, `JTAG`
    G12 = 12,

    /// @brief GPIO 13
    /// @note `ADC2_CH13`, `TOUCH`, `PWM`, `JTAG`
    G13 = 13,

    /// @brief GPIO 14
    /// @note `ADC2_CH14`, `TOUCH`, `PWM`, `JTAG`
    G14 = 14,

    /// @brief GPIO 15
    /// @note STRAPPING PIN, `ADC2_CH15`, `TOUCH`, `PWM`, `JTAG`
    G15 = 15,

    /// @brief GPIO 16
    /// @note `SPI0/1` (flash/PSRAM), `UART2_RX`, `ADC1_CH0`, `PWM`, `SPI`
    G16 = 16,

    /// @brief GPIO 17
    /// @note `SPI0/1` (flash/PSRAM), `UART2_TX`, `ADC1_CH1`, `PWM`
    G17 = 17,

    /// @brief GPIO 18
    /// @note `SPI_MOSI`, `ADC1_CH2`, `PWM`
    G18 = 18,

    /// @brief GPIO 19
    /// @note `SPI_MISO`, `ADC1_CH3`, `PWM`
    G19 = 19,

    // GPIO 20 is not physically available on the package

    /// @brief GPIO 21
    /// @note `I2C_SDA`, `ADC1_CH4`, `PWM`
    G21 = 21,

    /// @brief GPIO 22
    /// @note `I2C_SCL`, `ADC1_CH5`, `PWM`
    G22 = 22,

    /// @brief GPIO 23
    /// @note `SPI_MOSI` (alternate), `ADC1_CH6`, `PWM`
    G23 = 23,

    // GPIO 24 is not physically available on the package

    /// @brief GPIO 25
    /// @note `DAC1`, `ADC2_CH8`, `PWM`, `I2S`
    G25 = 25,

    /// @brief GPIO 26
    /// @note `DAC2`, `ADC2_CH9`, `PWM`, `I2S`
    G26 = 26,

    /// @brief GPIO 27
    /// @note `ADC2_CH10`, `PWM`, `I2S`, `TOUCH`
    G27 = 27,

    // GPIOs 28, 29, 30, 31 are not physically available on the package

    /// @brief GPIO 32
    /// @note `ADC1_CH4`, `TOUCH`, `PWM`, `I2S`, `XTAL32K`
    G32 = 32,

    /// @brief GPIO 33
    /// @note `ADC1_CH5`, `TOUCH`, `PWM`, `I2S`, `XTAL32K`
    G33 = 33,

    /// @brief GPIO 34
    /// @note Input‑only, `ADC1_CH6`
    G34 = 34,

    /// @brief GPIO 35
    /// @note Input‑only, `ADC1_CH7`
    G35 = 35,

    /// @brief GPIO 36
    /// @note Input‑only, `ADC1_CH0`, `SENSOR_VP`
    G36 = 36,

    /// @brief GPIO 37
    /// @note Input‑only, `ADC1_CH1`, `SENSOR_CAPP`
    G37 = 37,

    /// @brief GPIO 38
    /// @note Input‑only, `ADC1_CH2`, `SENSOR_CAPN`
    G38 = 38,

    /// @brief GPIO 39
    /// @note Input‑only, `ADC1_CH3`, `SENSOR_VN`
    G39 = 39,
};

using enum GpioNumber;

/// @brief CRTP base for GPIO object
/// @tparam Impl        The implementation class (CRTP)
/// @tparam InitResult  Result type of the initialization (default void)
/// @note Stores the GPIO number and provides `init()` via `mixin::Initable`.
template<typename Impl, typename InitResult = void> struct GPIO :

    mixin::NonCopyable,
    mixin::Initable<Impl, InitResult()>

{
    explicit constexpr GPIO(GpioNumber gpio_num) noexcept :
        _gpio_num{gpio_num} {}

    /// @brief Get GPIO's number
    [[nodiscard]] constexpr GpioNumber num() const noexcept {
        return _gpio_num;
    }

private:
    GpioNumber const _gpio_num;
};

#define KF_IMPL_GPIO(__impl__, __init_result__) KF_IMPL_INITABLE(__impl__, __init_result__())

/// @brief Digital input pin with configurable pull-up/down and interrupt support
/// @note The pin must be initialized with `init()` before use.
/// @note Interrupts are supported on platforms with Arduino API (attachInterruptArg).
struct DigitalInput : GPIO<DigitalInput> {

    /// @brief Pull configuration options
    enum class Pull : u8 {
        External = (
#ifdef INPUT
            INPUT
#else
            0
#endif
            ),

        InternalDown = (
#ifdef INPUT_PULLDOWN
            INPUT_PULLDOWN
#else
            1
#endif
            ),

        InternalUp = (
#ifdef INPUT_PULLUP
            INPUT_PULLUP
#else
            2
#endif
            ),
    };

    /// @brief Interrupt trigger modes
    enum class Interrupt : u8 {
        Disabled = (
#ifdef DISABLED
            DISABLED
#else
            0
#endif
            ),

        OnRising = (
#ifdef RISING
            RISING
#else
            1
#endif
            ),

        OnFalling = (
#ifdef FALLING
            FALLING
#else
            2
#endif
            ),

        OnChange = (
#ifdef CHANGE
            CHANGE
#else
            3
#endif
            ),

        OnLow = (
#ifdef ONLOW
            ONLOW
#else
            4
#endif
            ),

        OnHigh = (
#ifdef ONHIGH
            ONHIGH
#else
            5
#endif
            ),
    };

    using InterruptHandler = void (*)(void *argument);

    explicit constexpr DigitalInput(GpioNumber gpio_num, Pull pull) noexcept :
        GPIO<DigitalInput>{gpio_num}, _pull{pull} {}

    /// @brief Get GPIO logical level
    [[nodiscard]] bool level() const noexcept {
        return (
#ifdef ARDUINO
            static_cast<bool>(::digitalRead(static_cast<u8>(this->num())))
#else
            false
#endif
        );
    }

    /// @brief Attach interrupt handler to this pin
    void attachInterrupt(InterruptHandler handler, void *arg, Interrupt mode) noexcept {
#ifdef ARDUINO
        ::attachInterruptArg(static_cast<u8>(this->num()), handler, arg, static_cast<int>(mode));
#else
        (void) handler;
        (void) arg;
        (void) mode;
#endif
    }

    /// @brief Detach any previously attached interrupt
    void detachInterrupt() noexcept {
#ifdef ARDUINO
        ::detachInterrupt(static_cast<u8>(this->num()));
#endif
    }

private:
    Pull const _pull;

    KF_IMPL_GPIO(DigitalInput, void);

    void initImpl() noexcept {
#ifdef ARDUINO
        ::pinMode(static_cast<u8>(this->num()), static_cast<u8>(_pull));
#endif
    }
};

/// @brief Analog input pin (ADC) with global resolution control
/// @note Resolution is set globally for all ADC pins via static `resolution()`.
/// @note The pin must be initialized with `init()` (sets pin to INPUT mode).
struct AdcInput : GPIO<AdcInput> {

    using Value = u16;

    using GPIO<AdcInput>::GPIO;

    /// @brief Set the ADC resolution (bits) globally
    static void resolution(usize new_resolution_bits) noexcept {
        if (resolution_bits != new_resolution_bits) {
            resolution_bits = new_resolution_bits;
#ifdef ARDUINO
            ::analogReadResolution(resolution_bits);
#endif
        }
    }

    /// @brief Get Number of bits used for analog readings
    [[nodiscard]] static usize resolution() noexcept {
        return resolution_bits;
    }

    /// @brief Get Maximum value for the current resolution
    [[nodiscard]] static Value maxValue() noexcept {
        return static_cast<Value>((1u << resolution()) - 1u);
    }

    /// @brief Read analog value (0..maxValue)
    [[nodiscard]] Value read() const noexcept {
        return (
#ifdef ARDUINO
            ::analogRead(static_cast<u8>(this->num()))
#else
            0
#endif
        );
    }

private:
    inline static usize resolution_bits{12};// Arduino default resolution on ESP32 is 12-bits

    KF_IMPL_GPIO(AdcInput, void);

    void initImpl() noexcept {
#ifdef ARDUINO
        ::pinMode(static_cast<u8>(this->num()), INPUT);
#endif
    }
};

/// @brief Digital output pin
/// @note The pin must be initialized with `init()` before use.
struct DigitalOutput : GPIO<DigitalOutput> {

    using GPIO<DigitalOutput>::GPIO;

    /// @brief Set output level
    void level(bool value) const noexcept {
#ifdef ARDUINO
        ::digitalWrite(static_cast<u8>(this->num()), static_cast<u8>(value));
#else
        (void) value;
#endif
    }

private:
    KF_IMPL_GPIO(DigitalOutput, void);

    void initImpl() noexcept {
#ifdef ARDUINO
        ::pinMode(static_cast<u8>(this->num()), OUTPUT);
#endif
    }
};

/// @brief PWM output pin using hardware PWM (LEDC on ESP32)
/// @note Requires configuration.
/// @note The pin must be initialized with `init()` before use.
/// @note Supports writing duty cycle directly or via pulse width in microseconds.
struct PwmOutput :

    GPIO<PwmOutput, bool>,
    mixin::Configured<internal::PwmOutputConfig>

{

    /// @brief Configuration for an ESP32 LEDC PWM channel
    using Config = internal::PwmOutputConfig;

    using Duty = u16;

    explicit constexpr PwmOutput(Config const &config, GpioNumber gpio_num) noexcept :
        GPIO<PwmOutput, bool>{gpio_num}, mixin::Configured<Config>{config} {}

    /// @brief Write duty cycle (0..maxDuty)
    void write(Duty duty) const noexcept {
#ifdef ARDUINO
        ::ledcWrite(static_cast<u8>(this->num()), static_cast<u32>(duty));
#else
        (void) duty;
#endif
    }

    /// @brief Write pulse width in microseconds (converted to duty cycle)
    void writePulse(units::Microseconds pulse_width) const noexcept {
        auto const t = static_cast<u64>(pulse_width) * this->config().frequency_hz * this->config().maxDuty();
        this->write(static_cast<Duty>(t / 1'000'000u));
    }

private:
    KF_IMPL_GPIO(PwmOutput, bool);

    bool initImpl() noexcept {
        return (
#ifdef ARDUINO
            ::ledcAttach(static_cast<u8>(this->num()), this->config().frequency_hz, this->config().resolution_bits)
#else
            true
#endif
        );
    }
};

}// namespace kf::gpio