// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/aliases.hpp"

#include "kf/gpio/GPIO.hpp"

namespace kf::gpio::arduino {

struct DigitalInput : gpio::DigitalInput<DigitalInput, void> {
    enum class Pull : u8 {
        External = INPUT,
        InternalUp = INPUT_PULLUP,
        InternalDown = INPUT_PULLDOWN,
    };

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

struct AdcInput : gpio::AdcInput<AdcInput, void> {

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

struct DigitalOutput : gpio::DigitalOutput<DigitalOutput, void> {

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

struct PwmOutput : gpio::PwmOutput<PwmOutput, bool> {

    struct Config {
        u32 frequency_hz;
        u8 resolution_bits;
        u8 pin;
        u8 channel;

        u16 maxPwm() const noexcept {
            return static_cast<u16>((1u << (resolution_bits)) - 1u);
        }
    };

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