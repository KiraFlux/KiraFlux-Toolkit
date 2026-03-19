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

    /// @param pin      GPIO number (e.g. GPIO_NUM_4)
    /// @param pull_mode pull configuration
    explicit DigitalInput(gpio_num_t pin, Pull pull_type) noexcept
        : _pin{static_cast<u8>(pin)}, _state{static_cast<u8>(pull_type)} {}

private:
    const u8 _pin;
    u8 _state;

    [[nodiscard]] u8 matchMode(bool inverted_reading) const noexcept {
        if (static_cast<bool>(_state & external_pull_bit)) {
            return INPUT;
        } else {
            return inverted_reading ? INPUT_PULLDOWN : INPUT_PULLUP;
        }
    }

    // impl
    using This = DigitalInput;

    KF_IMPL_INITABLE(This, void);
    void initImpl() noexcept {
        const bool inverted_reading = ((_state & pull_up_bit) == 0);
        pinMode(_pin, matchMode(inverted_reading));
        _state = static_cast<u8>(inverted_reading);
    }

    // input impl
    friend struct kf::gpio::Input<This, bool, void>;

    [[nodiscard]] bool readImpl() const noexcept {
        const auto level = static_cast<bool>(digitalRead(_pin));

        // if inverted
        if (static_cast<bool>(_state)) {
            return not level;
        } else {
            return level;
        }
    }
};

/// 12‑bit ADC input (ESP32 typical).
/// @note AnalogReadResolution should be set globally (default 12 bits).
struct AdcInput : gpio::AdcInput<AdcInput, void> {

    /// @param pin GPIO number with ADC capability (e.g. GPIO_NUM_34)
    explicit AdcInput(gpio_num_t pin) noexcept :
        _pin{static_cast<u8>(pin)} {}

private:
    static u8 resolution_bits;

    const u8 _pin;

    // impl

    using This = AdcInput;

    KF_IMPL_INITABLE(This, void);
    void initImpl() noexcept {
        pinMode(_pin, INPUT);
    }

    friend struct kf::gpio::Input<This, u16, void>;
    [[nodiscard]] u16 readImpl() const noexcept {
        return analogRead(_pin);
    }

    friend struct kf::gpio::AdcInput<This, void>;
    static void setResolutionImpl(u8 new_resolution_bits) noexcept {
        if (resolution_bits != new_resolution_bits) {
            resolution_bits = new_resolution_bits;
            analogReadResolution(resolution_bits);
        }
    }

    static u8 getResolutionImpl() noexcept { return resolution_bits; }
};

u8 AdcInput::resolution_bits{12};// Arduino default resolution on ESP32 is 12-bits

/// Digital output
struct DigitalOutput : gpio::DigitalOutput<DigitalOutput, void> {

    /// @param pin GPIO number (e.g. GPIO_NUM_2)
    explicit DigitalOutput(gpio_num_t pin) noexcept :
        _pin{static_cast<u8>(pin)} {}

private:
    const u8 _pin;

    // impl
    using This = DigitalOutput;

    KF_IMPL_INITABLE(This, void);
    void initImpl() noexcept {
        pinMode(_pin, OUTPUT);
    }

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
    const Config &_config;

    // impl

    using This = PwmOutput;

    KF_IMPL_INITABLE(This, bool);
    [[nodiscard]] bool initImpl() noexcept {
        if (ledcSetup(_config.channel, _config.frequency_hz, _config.resolution_bits) == 0) {
            return false;
        }

        ledcAttachPin(_config.pin, _config.channel);

        return true;
    }

    friend struct kf::gpio::Output<PwmOutput, u16, bool>;
    void writeImpl(u16 level) const noexcept {
        ledcWrite(_config.channel, level);
    }

    friend struct kf::gpio::PwmOutput<PwmOutput, bool>;
    u32 getFrequencyImpl() const noexcept { return _config.frequency_hz; }
    u8 getResolutionImpl() const noexcept { return _config.resolution_bits; }
};

}// namespace kf::gpio::arduino