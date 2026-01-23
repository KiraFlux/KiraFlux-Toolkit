// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"
#include "kf/drivers/input/Joystick.hpp"
#include "kf/math/time/TimeoutManager.hpp"
#include "kf/math/time/Timer.hpp"

namespace kf {

/// @brief Monitors joystick for discrete directional changes with autorepeat
/// @note Provides stateful tracking of joystick position with thresholding and autorepeat
struct JoystickListener {
    /// @brief Joystick direction event types
    enum class Direction : u8 {
        Up = 1,   ///< Joystick moved upward
        Down = 2, ///< Joystick moved downward
        Left = 3, ///< Joystick moved left
        Right = 4,///< Joystick moved right
        Home,     ///< Joystick returned to center position
    };

private:
    Joystick &joystick;   ///< Reference to monitored joystick
    const float threshold;///< Activation threshold (0.0 to 1.0)

    // Autorepeat state
    Timer repeat_timer{static_cast<kf::Milliseconds>(100)};///< Timer for repeat events (100ms interval)
    TimeoutManager initial_delay{400};                     ///< Initial delay before repeat (500ms)
    bool in_repeat_mode{false};                            ///< Whether we're in repeat mode
    bool has_changed{false};                               ///< Flag indicating direction change since last poll
    Direction current_direction{Direction::Home};          ///< Current logical direction

public:
    /// @brief Construct listener for specific joystick
    /// @param joy Joystick instance to monitor
    /// @param threshold Activation threshold (0.0 to 1.0, default: 0.6)
    explicit JoystickListener(Joystick &joy, float threshold = 0.6f) :
        joystick{joy}, threshold{threshold} {}

    /// @brief Poll joystick state and update internal direction with autorepeat
    /// @param now Current time in milliseconds (e.g., from millis())
    /// @note Must be called regularly to track direction changes and autorepeat
    void poll(Milliseconds now) {
        const Direction new_direction = calculateDirection();

        // Check for real direction change
        if (new_direction != current_direction) {
            current_direction = new_direction;
            has_changed = true;
            in_repeat_mode = false;

            // Reset timers on direction change
            if (current_direction != Direction::Home) {
                initial_delay.update(now);                // Reset initial delay
                repeat_timer = Timer{repeat_timer.period};// Reset repeat timer
            }
        }
        // Same direction, check for autorepeat
        else if (current_direction != Direction::Home) {
            if (not in_repeat_mode) {
                // Waiting for initial delay (500ms)
                if (initial_delay.expired(now)) {
                    in_repeat_mode = true;
                    has_changed = true;// First repeat after delay
                }
            } else {
                // In repeat mode, check for repeat interval (100ms)
                if (repeat_timer.ready(now)) {
                    has_changed = true;// Subsequent repeats
                }
            }
        } else {
            // Home direction - no autorepeat
            in_repeat_mode = false;
        }
    }

    /// @brief Get current logical direction based on threshold
    kf_nodiscard Direction direction() const { return current_direction; }

    /// @brief Check if currently in autorepeat mode
    kf_nodiscard bool repeating() const { return in_repeat_mode; }

    /// @brief Check if direction has changed since last poll()
    kf_nodiscard bool changed() {
        const bool changed = has_changed;
        has_changed = false;
        return changed;
    }

    /// @brief Calculate raw direction without updating internal state
    kf_nodiscard Direction calculateDirection() const {
        const auto x = joystick.axis_x.read();
        const auto y = joystick.axis_y.read();
        const auto abs_x = std::abs(x);
        const auto abs_y = std::abs(y);

        if (abs_x < threshold && abs_y < threshold) {
            return Direction::Home;
        }

        if (abs_x > abs_y) {
            return x > 0 ? Direction::Right : Direction::Left;
        } else {
            return y > 0 ? Direction::Up : Direction::Down;
        }
    }

    /// @brief Reset internal state to Home direction
    void reset() {
        current_direction = Direction::Home;
        has_changed = false;
        in_repeat_mode = false;
    }
};

}// namespace kf