// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
#include "kf/input/Joystick.hpp"
#include "kf/math/Timer.hpp"
#include "kf/math/units.hpp"

namespace kf {

struct JoystickListener {
    enum class Direction : u8 {
        Up = 0,
        Down = 1,
        Left = 2,
        Right = 3,
        Home
    };

private:
    Joystick &joystick;
    const float threshold;

    math::Timer repeat_timer{static_cast<kf::Milliseconds>(100)};
    math::Timer initial_delay{static_cast<kf::Milliseconds>(400)};

    bool in_repeat_mode{false};
    bool has_changed{false};
    Direction current_direction{Direction::Home};

public:
    explicit JoystickListener(Joystick &joy, float threshold = 0.6f) noexcept
        : joystick{joy}, threshold{threshold} {}

    void poll(Milliseconds now) noexcept {
        const auto new_direction = calculateDirection();

        if (new_direction != current_direction) {
            current_direction = new_direction;
            has_changed = true;
            in_repeat_mode = false;

            if (current_direction != Direction::Home) {
                initial_delay.start(now);
                repeat_timer.start(now);
            }
        } else if (current_direction != Direction::Home) {
            if (in_repeat_mode) {
                if (repeat_timer.expired(now)) {
                    has_changed = true;
                    repeat_timer.start(now);
                }
            } else {
                if (initial_delay.expired(now)) {
                    in_repeat_mode = true;
                    has_changed = true;
                    repeat_timer.start(now);// restart repeat timer for subsequent repeats
                }
            }
        } else {
            in_repeat_mode = false;
        }
    }

    [[nodiscard]] Direction direction() const noexcept { return current_direction; }

    [[nodiscard]] bool repeating() const noexcept { return in_repeat_mode; }

    /// @note Resets has_changed
    [[nodiscard]] bool changed() noexcept {
        bool ch = has_changed;
        has_changed = false;
        return ch;
    }

    [[nodiscard]] Direction calculateDirection() const noexcept {
        const auto x = joystick.axis_x.read();
        const auto y = joystick.axis_y.read();
        const auto ax = std::abs(x);
        const auto ay = std::abs(y);

        if (ax < threshold and ay < threshold) {
            return Direction::Home;
        }
        if (ax > ay) {
            return x > 0 ? Direction::Right : Direction::Left;
        } else {
            return y > 0 ? Direction::Up : Direction::Down;
        }
    }

    void reset() noexcept {
        current_direction = Direction::Home;
        has_changed = false;
        in_repeat_mode = false;
    }
};

}// namespace kf