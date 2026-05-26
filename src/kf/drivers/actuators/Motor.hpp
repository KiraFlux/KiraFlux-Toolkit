// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "kf/algorithm.hpp"
#include "kf/math/units.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/primitives.hpp"

#include "kf/drivers/actuators/Actuator.hpp"

namespace kf::drivers::actuators {
namespace internal {

struct MotorConfig final : mixin::NonCopyable {
    /// @brief PWM value type for control
    using DutyType = u16;

    constexpr static auto normalized_dead_zone{1e-2f};

    /// @brief Positive rotation direction definition
    enum class Direction : u8 {
        CW = 0x00,///< Clockwise rotation is positive
        CCW = 0x01///< Counter-clockwise rotation is positive
    };

    DutyType dead_zone;
    DutyType max_duty;
    Direction normal_direction;///< Positive rotation direction

    [[nodiscard]] bool getValueLevel(f32 value) const noexcept {
        return (normal_direction == Direction::CW) == (value > 0);
    }

    [[nodiscard]] DutyType dutyfromNormalized(f32 value) const noexcept {
        const auto abs_value = kf::clamp(kf::abs(value), 0.0f, 1.0f);
        if (abs_value < normalized_dead_zone) {
            return 0;
        } else {
            return static_cast<DutyType>(i32(abs_value * f32(max_duty - dead_zone)) + dead_zone);
        }
    }
};

}// namespace internal

template<typename PwmPinImpl, typename DigitalPinImpl>
struct L298nMotor final : Actuator<L298nMotor<PwmPinImpl, DigitalPinImpl>, bool>, mixin::Configurable<internal::MotorConfig> {

    /// @brief Motor driver hardware configuration
    using Config = internal::MotorConfig;

    explicit constexpr L298nMotor(const Config &config, PwmPinImpl &&pin_pwm, DigitalPinImpl &&pin_dir) noexcept :
        mixin::Configurable<internal::MotorConfig>(config), _pin_pwm{std::move(pin_pwm)}, _pin_dir{std::move(pin_dir)} {}

    /// @brief Set motor speed from normalized value
    /// @param value Normalized speed (-1.0 to 1.0)
    /// @note Applies dead zone and converts to PWM with direction
    void write(f32 value) const noexcept {
        _pin_dir.write(this->config().getValueLevel(value));
        _pin_pwm.write(this->config().dutyfromNormalized(value));
    }

    /// @brief Stop motor (set PWM to zero)
    void disable() const noexcept {
        _pin_pwm.write(0);
    }

private:
    PwmPinImpl _pin_pwm;
    DigitalPinImpl _pin_dir;

    // impl
    using This = L298nMotor<PwmPinImpl, DigitalPinImpl>;

    KF_IMPL_INITABLE(This, bool);
    bool initImpl() noexcept {
        _pin_dir.init();
        return _pin_pwm.init();
    }

    KF_IMPL(Actuator<This, bool>);
};

}// namespace kf::drivers::actuators