// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/aliases.hpp"
#include "kf/math/Timer.hpp"
#include "kf/math/units.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf::input {

template<typename I> struct JoystickListener final : kf::mixin::Resettable<JoystickListener<I>> {
    using JoystickImpl = I;

    enum class Direction : u8 {
        Up = 0,
        Down = 1,
        Left = 2,
        Right = 3,
        Home
    };

    explicit JoystickListener(JoystickImpl &joystick, f32 threshold = 0.6f) noexcept :
        _joystick{joystick}, _threshold{threshold} {}

    void poll(math::Milliseconds now) noexcept {
        const auto x = _joystick.axis_x.read();
        const auto y = _joystick.axis_y.read();

        const auto new_direction = calculateDirection(x, y);

        if (new_direction != _current_direction) {
            _current_direction = new_direction;
            _has_changed = true;
            _in_repeat_mode = false;

            if (_current_direction != Direction::Home) {
                _initial_delay.start(now);
                _repeat_timer.start(now);
            }
        } else if (_current_direction != Direction::Home) {
            if (_in_repeat_mode) {
                if (_repeat_timer.expired(now)) {
                    _has_changed = true;
                    _repeat_timer.start(now);
                }
            } else {
                if (_initial_delay.expired(now)) {
                    _in_repeat_mode = true;
                    _has_changed = true;
                    _repeat_timer.start(now);// restart repeat timer for subsequent repeats
                }
            }
        } else {
            _in_repeat_mode = false;
        }
    }

    [[nodiscard]] Direction direction() const noexcept { return _current_direction; }

    [[nodiscard]] bool repeating() const noexcept { return _in_repeat_mode; }

    /// @note Resets has_changed
    [[nodiscard]] bool changed() noexcept {
        bool ch = _has_changed;
        _has_changed = false;
        return ch;
    }

private:
    JoystickImpl &_joystick;
    const f32 _threshold;

    math::Timer _repeat_timer{static_cast<kf::math::Milliseconds>(100)};
    math::Timer _initial_delay{static_cast<kf::math::Milliseconds>(400)};
    Direction _current_direction{Direction::Home};
    bool _in_repeat_mode{false};
    bool _has_changed{false};

    [[nodiscard]] Direction calculateDirection(f32 x, f32 y) const noexcept {
        const auto ax = kf::abs(x);
        const auto ay = kf::abs(y);

        if (ax < _threshold and ay < _threshold) {
            return Direction::Home;
        }
        if (ax > ay) {
            return x > 0 ? Direction::Right : Direction::Left;
        } else {
            return y > 0 ? Direction::Up : Direction::Down;
        }
    }

    //impl

    friend struct kf::mixin::Resettable<JoystickListener<I>>;

    void resetImpl() noexcept {
        _current_direction = Direction::Home;
        _has_changed = false;
        _in_repeat_mode = false;
    }
};

}// namespace kf::input