// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/algorithm.hpp"
#include "kf/aliases.hpp"
#include "kf/math/units.hpp"

namespace kf::drivers::actuators {

/// @brief Motor driver supporting IArduino Motor Shield and L298N/L293D H-bridge
/// @note Provides bidirectional PWM control with configurable dead zone and direction
template<typename PwmPinImpl, typename DigitalPinImpl> struct L298nMotor final : mixin::Initable<L298nMotor, bool> {

    /// @brief PWM value type for control
    using DutyType = u16;

    /// @brief Positive rotation direction definition
    enum class Direction : u8 {
        CW = 0x00,///< Clockwise rotation is positive
        CCW = 0x01///< Counter-clockwise rotation is positive
    };

    /// @brief Motor driver hardware configuration
    struct Config {
        DutyType dead_zone;
        Direction normal_direction;///< Positive rotation direction

        [[nodiscard]] bool getValueLevel(f32 value) const noexcept {
            return (normal_direction == Direction::CW) == (value > 0);
        }
    };

    explicit constexpr L298nMotor(const Config &config, PwmPinImpl &&pin_pwm, DigitalPinImpl &&pin_dir) noexcept :
        _config{config}, _pin_pwm{pin_pwm}, _pin_dir{pin_dir} {}

    /// @brief Set motor speed from normalized value
    /// @param value Normalized speed (-1.0 to 1.0)
    /// @note Applies dead zone and converts to PWM with direction
    void write(f32 value) const noexcept {
        _pin_dir.write(_config.getValueLevel(value));
        _pin_pwm.write(dutyfromNormalized(kf::clamp(kf::abs(value), 0.0f, 1.0f)));
    }

    /// @brief Stop motor (set PWM to zero)
    void stop() const noexcept {
        _pin_pwm.write(0);
    }

private:
    const Config &_config;
    PwmPinImpl _pin_pwm;
    DigitalPinImpl _pin_dir;
    DutyType _max_pwm{0};

    [[nodiscard]] DutyType dutyfromNormalized(f32 value) const noexcept {
        constexpr auto normalized_dead_zone = 1e-2f;

        if (value < normalized_dead_zone) {
            return 0;
        } else {
            return static_cast<DutyType>(i32(value * f32(_max_pwm - _config.dead_zone)) + _config.dead_zone);
        }
    }

    // Initable impl
    friend struct kf::mixin::Initable<L298nMotor, bool>;

    bool initImpl() noexcept {
        _pin_dir.init();
        if (not _pin_pwm.init()) { return false; }

        _max_pwm = _pin_pwm.maxDuty();

        return true;
    }
};

}// namespace kf::drivers::actuators