// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/aliases.hpp"
#include "kf/gpio/GPIO.hpp"

namespace kf::gpio::arduino {

/// Digital input with configurable pull‑up/pull‑down.
/// @note Pull mode is set during init(); after that the pin stays configured.
struct DigitalInput : gpio::DigitalInput<DigitalInput, void> {
    /// Pull configuration – values correspond to Arduino pinMode constants.
    enum class Pull : u8 {
        External = INPUT,             ///< no internal pull, external resistor assumed
        InternalUp = INPUT_PULLUP,    ///< enable internal pull‑up
        InternalDown = INPUT_PULLDOWN,///< enable internal pull‑down (if supported)
    };

    /// @param pin      GPIO number (e.g. GPIO_NUM_4)
    /// @param pull_mode pull configuration (default: External)
    explicit DigitalInput(gpio_num_t pin, Pull pull_mode = Pull::External) noexcept
        : _pin{static_cast<u8>(pin)}, _pull_mode{pull_mode} {}

private:
    const Pull _pull_mode;
    const u8 _pin;

    // Initable impl
    friend struct kf::mixin::Initable<DigitalInput, void>;

    void initImpl() noexcept {
        pinMode(_pin, static_cast<u8>(_pull_mode));
    }

    // input impl
    friend struct kf::gpio::Input<DigitalInput, bool, void>;

    [[nodiscard]] bool readImpl() const noexcept {
        return digitalRead(_pin);
    }
};

/// 12‑bit ADC input (ESP32 typical).
/// @note AnalogReadResolution should be set globally (default 12 bits).
struct AdcInput : gpio::AdcInput<AdcInput, void> {

    /// @param pin GPIO number with ADC capability (e.g. GPIO_NUM_34)
    explicit AdcInput(gpio_num_t pin) noexcept :
        _pin{static_cast<u8>(pin)} {}

private:
    const u8 _pin;

    // Initable impl
    friend struct kf::mixin::Initable<AdcInput, void>;

    void initImpl() noexcept {
        pinMode(_pin, INPUT);
    }

    // input impl
    friend struct kf::gpio::Input<AdcInput, u16, void>;

    [[nodiscard]] u16 readImpl() const noexcept {
        return analogRead(_pin);
    }
};

/// Digital output
struct DigitalOutput : gpio::DigitalOutput<DigitalOutput, void> {

    /// @param pin GPIO number (e.g. GPIO_NUM_2)
    explicit DigitalOutput(gpio_num_t pin) noexcept :
        _pin{static_cast<u8>(pin)} {}

private:
    const u8 _pin;

    // Initable impl
    friend struct kf::mixin::Initable<DigitalOutput, void>;

    void initImpl() noexcept {
        pinMode(_pin, OUTPUT);
    }

    // output impl
    friend struct kf::gpio::Output<DigitalOutput, bool, void>;

    void writeImpl(bool level) const noexcept {
        digitalWrite(_pin, level);
    }
};

/// PWM output using ESP32 LEDC hardware.
/// @note One LEDC channel can control multiple pins (same frequency/resolution).
struct PwmOutput : gpio::PwmOutput<PwmOutput, bool> {

    struct Config {
        u32 frequency_hz;  ///< PWM frequency (Hz)
        u8 resolution_bits;///< resolution in bits (1..16)
        u8 pin;            ///< GPIO pin number
        u8 channel;        ///< LEDC channel (0..15)

        /// Maximum PWM value for given resolution (2^bits - 1).
        u16 maxPwm() const noexcept {
            return static_cast<u16>((1u << (resolution_bits)) - 1u);
        }
    };

    /// @param config reference to configuration (must outlive this object)
    explicit PwmOutput(Config &config) noexcept : _config{config} {}

private:
    Config &_config;

    // Initable impl
    friend struct kf::mixin::Initable<PwmOutput, bool>;

    [[nodiscard]] bool initImpl() noexcept {
        if (ledcSetup(_config.channel, _config.frequency_hz, _config.resolution_bits) == 0) {
            return false;
        }

        ledcAttachPin(_config.pin, _config.channel);

        return true;
    }

    // output impl
    friend struct kf::gpio::Output<PwmOutput, u16, bool>;

    void writeImpl(u16 level) const noexcept {
        ledcWrite(_config.channel, level);
    }
};

}// namespace kf::gpio::arduino