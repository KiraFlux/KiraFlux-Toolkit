// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Timer.hpp"
#include "kf/listener/Listener.hpp"
#include "kf/math.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/primitives.hpp"
#include "kf/units.hpp"

namespace kf::internal {

enum class JoystickListenerDirection : u8 {
    Up = 0,
    Down = 1,
    Left = 2,
    Right = 3,
    Center
};

struct JoystickListenerConfig final {
    Timer::Config repeat_timer, delay_timer;
    f32 threshold;///< 0..1

    [[nodiscard]] JoystickListenerDirection calculateDirection(const math::Vector2f &v) const noexcept {
        const auto ax = math::abs(v.x);
        const auto ay = math::abs(v.y);
        if (ax < threshold and ay < threshold) {
            return JoystickListenerDirection::Center;
        }
        if (ax > ay) {
            return v.x > 0 ? JoystickListenerDirection::Right : JoystickListenerDirection::Left;
        } else {
            return v.y > 0 ? JoystickListenerDirection::Up : JoystickListenerDirection::Down;
        }
    }
};

}// namespace kf::internal

namespace kf::listener {

struct JoystickListener final :

    Listener<JoystickListener, math::Vector2f, void(internal::JoystickListenerDirection)>,
    mixin::Configured<internal::JoystickListenerConfig>

{
    using Config = internal::JoystickListenerConfig;
    using Direction = internal::JoystickListenerDirection;

    using mixin::Configured<Config>::Configured;

private:
    Timer _repeat_timer{this->config().repeat_timer};
    Timer _initial_delay{this->config().delay_timer};

    Direction _current_direction{Direction::Center};
    bool _in_repeat_mode{false}, _has_changed{false};

    KF_IMPL_LISTENER(JoystickListener);

    constexpr void resetImpl() noexcept {
        _current_direction = Direction::Center;
        _has_changed = false;
        _in_repeat_mode = false;
    }

    void pollImpl(units::Milliseconds now) noexcept {
        if (this->value().isNone()) { return; }

        const auto new_direction = this->config().calculateDirection(this->value().unwrap());

        if (new_direction != _current_direction) {
            _current_direction = new_direction;
            _has_changed = true;
            _in_repeat_mode = false;

            if (_current_direction != Direction::Center) {
                _initial_delay.start(now);
                _repeat_timer.start(now);
            }
        } else if (_current_direction != Direction::Center) {
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

        if (_has_changed) {
            _has_changed = false;
            this->invoke(_current_direction);
        }
    }
};

}// namespace kf::listener