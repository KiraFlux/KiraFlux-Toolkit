// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/aliases.hpp"
#include "kf/math/units.hpp"

namespace kf::input {

/// @brief Minimal button with press detection only
struct Button {

    struct Config {
        math::Milliseconds debounce;
        u8 pin;

        enum class Mode : u8 {
            PullUp,
            PullDown
        } mode;

        enum class PullType : u8 {
            External,
            Internal
        } pull_type;

        explicit Config(
            gpio_num_t pin,
            Mode mode,
            PullType pull_type,
            math::Milliseconds debounce = 30) noexcept :
            pin{static_cast<u8>(pin)}, mode{mode}, pull_type{pull_type}, debounce{debounce} {}

        [[nodiscard]] bool normalize(bool state) const noexcept {
            return mode == Mode::PullDown == state;
        }

        [[nodiscard]] u8 matchMode() const noexcept {
            if (pull_type == PullType::External) {
                return INPUT;
            }
            return (mode == Mode::PullUp) ? INPUT_PULLUP : INPUT_PULLDOWN;
        }
    };

private:
    const Config &_config;
    math::Milliseconds _next{0};
    bool _last_stable{false};
    bool _last_raw{false};
    bool _click_ready{false};

public:
    explicit Button(const Config &config) noexcept :
        _config{config} {}

    void init() const noexcept {
        pinMode(_config.pin, _config.matchMode());
    }

    /// @brief Poll button state - must be called regularly
    void poll(math::Milliseconds now) noexcept {
        const bool state = _config.normalize(digitalRead(_config.pin));

        if (state != _last_raw) {
            _last_raw = state;
            _next = now + _config.debounce;
        }

        if (now >= _next) {
            if (_last_stable != state) {
                _last_stable = state;

                if (_last_stable) {
                    _click_ready = true;
                }
            }
        }
    }

    /// @brief Check if button was clicked (consumes the click)
    /// @return true if button was pressed since last call
    [[nodiscard]] bool clicked() noexcept {
        if (_click_ready) {
            _click_ready = false;
            return true;
        }
        return false;
    }

    /// @brief Check current button state
    /// @return true if button is currently pressed (after debounce)
    [[nodiscard]] bool pressed() const noexcept {
        return _last_stable;
    }
};

}// namespace kf