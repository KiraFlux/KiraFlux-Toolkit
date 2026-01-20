// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Function.hpp"
#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"
#include "kf/drivers/input/Joystick.hpp"

namespace kf {

/// @brief Listener for joystick directional change events
/// @note Event handler triggers only once per direction change
struct JoystickListener {

private:
    /// @brief Default activation threshold for joystick movement
    static constexpr auto default_threshold = 0.6;

public:
    /// @brief Joystick direction event types
    /// @note Events trigger only once when direction changes
    enum class Direction : u8 {
        Home = 0x00, ///< Joystick returned to center position
        Up = 0x10,   ///< Joystick moved upward
        Down = 0x20, ///< Joystick moved downward
        Left = 0x30, ///< Joystick moved left
        Right = 0x40,///< Joystick moved right
    };

    Function<void(Direction)> handler{nullptr};///< Callback for direction change events
    const float threshold{default_threshold};  ///< Activation threshold (0.0 to 1.0)

private:
    Direction last_direction{Direction::Home};///< Previous detected direction
    Joystick &joystick;                       ///< Reference to monitored joystick

public:
    /// @brief Construct listener for specific joystick
    /// @param joy Joystick instance to monitor
    explicit JoystickListener(Joystick &joy) :
        joystick{joy} {}

    /// @brief Poll joystick state and trigger events on direction change
    /// @note Must be called regularly in main loop for event detection
    void poll() {
        if (nullptr == handler) { return; }

        const auto current_direction = getCurrentDirection();

        if (current_direction != last_direction) {
            handler(current_direction);
            last_direction = current_direction;
        }
    }

private:
    /// @brief Calculate current direction based on joystick axes
    /// @return Current direction with threshold filtering
    kf_nodiscard Direction getCurrentDirection() {
        const auto x = joystick.axis_x.read();
        const auto y = joystick.axis_y.read();
        const auto abs_x = std::abs(x);
        const auto abs_y = std::abs(y);

        if (abs_x < threshold and abs_y < threshold) {
            return Direction::Home;
        }

        if (abs_x > abs_y) {
            return x > 0 ? Direction::Right : Direction::Left;
        } else {
            return y > 0 ? Direction::Up : Direction::Down;
        }
    }
};

}// namespace kf