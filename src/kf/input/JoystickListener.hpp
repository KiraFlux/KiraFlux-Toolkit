// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/primitives.hpp"
#include "kf/math/Timer.hpp"
#include "kf/math/units.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Resettable.hpp"
#include "kf/mixin/TimedPollable.hpp"

namespace kf::input {
namespace internal::jl {

enum class Direction : u8 {
    Up = 0,
    Down = 1,
    Left = 2,
    Right = 3,
    Home
};

struct Config final : mixin::NonCopyable {
    f32 threshold;///< 0..1
    kf::math::Milliseconds repeat_timeout, delay;

    [[nodiscard]] Direction calculateDirection(f32 x, f32 y) const noexcept {
        const auto ax = kf::abs(x);
        const auto ay = kf::abs(y);
        // todo array-map
        if (ax < threshold and ay < threshold) {
            return Direction::Home;
        }
        if (ax > ay) {
            return x > 0 ? Direction::Right : Direction::Left;
        } else {
            return y > 0 ? Direction::Up : Direction::Down;
        }
    }
};
}// namespace internal::jl

template<typename I>
struct JoystickListener final : mixin::Configurable<internal::jl::Config>,
                                mixin::NonCopyable,
                                mixin::Resettable<JoystickListener<I>>,
                                mixin::TimedPollable<JoystickListener<I>> {
    using JoystickImpl = I;
    using Config = internal::jl::Config;
    using Direction = internal::jl::Direction;

    explicit JoystickListener(JoystickImpl &joystick, const Config &config) noexcept :
        _joystick{joystick}, mixin::Configurable<Config>{config}, _repeat_timer{config.repeat_timeout}, _initial_delay{config.delay} {}

    [[nodiscard]] Direction direction() const noexcept { return _current_direction; }

    [[nodiscard]] bool repeating() const noexcept { return _in_repeat_mode; }

    /// @note Resets has_changeda
    [[nodiscard]] bool changed() noexcept {
        const bool ret = _has_changed;
        _has_changed = false;
        return ret;
    }

private:
    JoystickImpl &_joystick;
    math::Timer _repeat_timer;
    math::Timer _initial_delay;

    Direction _current_direction{Direction::Home};
    bool _in_repeat_mode{false};
    bool _has_changed{false};

    //impl
    using This = JoystickListener<I>;

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        _current_direction = Direction::Home;
        _has_changed = false;
        _in_repeat_mode = false;
    }

    KF_IMPL_TIMED_POLLABLE(This);
    void pollImpl(math::Milliseconds now) noexcept {
        const auto x = _joystick.axis_x.read();
        const auto y = _joystick.axis_y.read();

        const auto new_direction = this->config().calculateDirection(x, y);

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
};

}// namespace kf::input