// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/Function.hpp"
#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"

namespace kf {

/// @brief Push button with debouncing and configurable pull-up/down
/// @note Handles debouncing and supports both internal and external pull resistors
struct Button {

    /// @brief Button electrical configuration mode
    enum class Mode : u8 {
        PullUp,  ///< Button connects to GND (active LOW)
        PullDown,///< Button connects to VCC (active HIGH)
    };

    /// @brief Pull resistor type selection
    enum class PullType : u8 {
        External,///< Use external pull resistor
        Internal,///< Use MCU internal pull resistor
    };

private:
    /// @brief Debounce duration in milliseconds
    static constexpr auto debounce_ms = 50;

public:
    Function<void()> handler{nullptr};///< Callback invoked on button press

private:
    u32 last_press_ms{0};  ///< Timestamp of last valid press
    const u8 pin;          ///< GPIO pin number
    const Mode mode;       ///< Button electrical mode
    bool last_state{false};///< Previous filtered button state

public:
    /// @brief Construct button instance
    /// @param pin GPIO pin connected to button
    /// @param mode Electrical configuration (default: PullDown)
    explicit Button(gpio_num_t pin, Mode mode = Mode::PullDown) :
        pin{static_cast<u8>(pin)}, mode{mode} {}

    /// @brief Initialize button hardware
    /// @param pull_type Select internal or external pull resistor
    inline void init(PullType pull_type) const noexcept {
        pinMode(pin, matchMode(pull_type));
    }

    /// @brief Poll button state and trigger handler on press
    /// @note Must be called regularly (typically in main loop)
    void poll() noexcept {
        const auto current_state = read();
        const auto now = millis();

        if (current_state and not last_state) {
            if (now - last_press_ms > debounce_ms) {
                if (nullptr != handler) {
                    handler();
                }
                last_press_ms = now;
            }
        }

        last_state = current_state;
    }

    /// @brief Read current button state
    /// @return True if button is pressed (after mode conversion)
    kf_nodiscard bool read() const noexcept {
        if (mode == Mode::PullUp) {
            return not digitalRead(pin);
        }
        return digitalRead(pin);
    }

private:
    /// @brief Map pull type to Arduino pin mode constant
    /// @param pull_type Internal or external pull resistor
    /// @return Corresponding INPUT mode constant
    kf_nodiscard inline u8 matchMode(PullType pull_type) const noexcept {
        if (PullType::External == pull_type) {
            return INPUT;
        }

        // else - Internal
        if (mode == Mode::PullUp) {
            return INPUT_PULLUP;
        }

        return INPUT_PULLDOWN;
    }
};
}// namespace kf
