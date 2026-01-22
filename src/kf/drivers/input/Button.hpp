// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"


namespace kf {

/// @brief Minimal button with press detection only
struct Button {
    enum class Mode : u8 { PullUp, PullDown };
    enum class PullType : u8 { External, Internal };

private:
    static constexpr Milliseconds debounce_ms = 30;

    u32 last_change{0};
    bool last_stable{false};
    bool click_ready{false};
    bool last_raw{false};
    const u8 pin;
    const Mode mode;

public:
    explicit Button(gpio_num_t pin, Mode mode = Mode::PullDown) :
        pin{static_cast<u8>(pin)}, mode{mode} {}

    void init(PullType pull_type) const {
        pinMode(pin, matchMode(pull_type));
    }

    /// @brief Poll button state - must be called regularly
    void poll() {
        const auto now = millis();
        const bool raw = readRaw();

        if (raw != last_raw) {
            last_raw = raw;
            last_change = now;
        }

        if (now - last_change >= debounce_ms) {
            if (last_stable != raw) {
                last_stable = raw;

                if (last_stable) {
                    click_ready = true;
                }
            }
        }
    }

    /// @brief Check if button was clicked (consumes the click)
    /// @return true if button was pressed since last call
    kf_nodiscard bool clicked() {
        if (click_ready) {
            click_ready = false;
            return true;
        }
        return false;
    }

    /// @brief Check current button state
    /// @return true if button is currently pressed (after debounce)
    kf_nodiscard bool pressed() const {
        return last_stable;
    }

private:
    kf_nodiscard bool readRaw() const {
        const bool raw = digitalRead(pin);
        if (mode == Mode::PullUp) {
            return not raw;
        } else {
            return raw;
        }
    }

    kf_nodiscard u8 matchMode(PullType pull_type) const {
        if (pull_type == PullType::External) {
            return INPUT;
        }
        return (mode == Mode::PullUp) ? INPUT_PULLUP : INPUT_PULLDOWN;
    }
};

} // namespace kf