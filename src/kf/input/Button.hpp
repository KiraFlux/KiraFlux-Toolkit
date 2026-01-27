// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/aliases.hpp"
#include "kf/core/attributes.hpp"
#include "kf/math/units.hpp"


namespace kf {

/// @brief Minimal button with press detection only
struct Button {

    struct Config {
        Milliseconds debounce;
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
            Milliseconds debounce = 30
        ) noexcept:
            pin{static_cast<u8>(pin)}, mode{mode}, pull_type{pull_type}, debounce{debounce} {}

        kf_nodiscard bool normalize(bool state) const noexcept {
            return mode == Mode::PullDown == state;
        }

        kf_nodiscard u8 matchMode() const noexcept {
            if (pull_type == PullType::External) {
                return INPUT;
            }
            return (mode == Mode::PullUp) ? INPUT_PULLUP : INPUT_PULLDOWN;
        }
    };

private:

    const Config &config;
    Milliseconds next{0};
    bool last_stable{false};
    bool last_raw{false};
    bool click_ready{false};

public:
    explicit Button(const Config &config) noexcept:
        config{config} {}

    void init() const noexcept {
        pinMode(config.pin, config.matchMode());
    }

    /// @brief Poll button state - must be called regularly
    void poll(Milliseconds now) noexcept {
        const bool state = config.normalize(digitalRead(config.pin));

        if (state != last_raw) {
            last_raw = state;
            next = now + config.debounce;
        }

        if (now >= next) {
            if (last_stable != state) {
                last_stable = state;

                if (last_stable) {
                    click_ready = true;
                }
            }
        }
    }

    /// @brief Check if button was clicked (consumes the click)
    /// @return true if button was pressed since last call
    kf_nodiscard bool clicked() noexcept {
        if (click_ready) {
            click_ready = false;
            return true;
        }
        return false;
    }

    /// @brief Check current button state
    /// @return true if button is currently pressed (after debounce)
    kf_nodiscard bool pressed() const noexcept {
        return last_stable;
    }
};

} // namespace kf