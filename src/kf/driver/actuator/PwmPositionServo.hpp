// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Range.hpp"
#include "kf/gpio.hpp"
#include "kf/math.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/units.hpp"

#include "kf/driver/actuator/ActuatorDriver.hpp"

namespace kf::internal {

struct PwmPositionServoConfig final {
    using AngleRange = Range<units::Degrees>;
    using PulseRange = Range<units::Microseconds>;

    AngleRange angle_range;
    PulseRange pulse_range;
    gpio::PwmOutput::Config pwm;

    /// @brief Convert angle to pulse width using linear interpolation
    /// @param angle Target servo angle
    /// @return Required pulse width in microseconds
    [[nodiscard]] constexpr units::Microseconds pulseWidthFromAngle(units::Degrees angle) const noexcept {
        return math::linearMap<i32>(angle, angle_range.start, angle_range.end, pulse_range.start, pulse_range.end);
    }
};

}// namespace kf::internal

namespace kf::driver::actuator {

/// @brief PWM-controlled position servo driver for ESP32 LEDC hardware
/// @note Converts angular positions to PWM pulse widths for standard RC servos
struct PwmPositionServo final :

    ActuatorDriver<PwmPositionServo, units::Degrees, bool()>,
    mixin::Configured<internal::PwmPositionServoConfig>

{

    /// @brief Configuration for PWM position servo (angle <-> pulse width mapping)
    /// @note Contains hardware‑independent mapping and is self‑validating
    using Config = internal::PwmPositionServoConfig;

    /// @brief Construct servo with the same angle range for both mapping and safe operation
    /// @param config Mapping between angle and pulse width
    /// @param gpio_num PWM output gpio number
    explicit constexpr PwmPositionServo(Config const &config, gpio::GpioNumber gpio_num) noexcept :
        mixin::Configured<Config>{config}, _angle_safe_range{config.angle_range}, _pwm_output{config.pwm, gpio_num} {}

    /// @brief Construct servo with separate safe angle range (may be narrower than config range)
    /// @param config Mapping between angle and pulse width
    /// @param gpio_num PWM output gpio number
    /// @param angle_safe_range Additional clamping range for safety (e.g., to avoid mechanical limits)
    explicit constexpr PwmPositionServo(Config const &config, gpio::GpioNumber gpio_num, Config::AngleRange angle_safe_range) noexcept :
        mixin::Configured<Config>{config}, _angle_safe_range{angle_safe_range}, _pwm_output{config.pwm, gpio_num} {}

private:
    Config::AngleRange _angle_safe_range;///< Safe operating angle range (clamped before mapping)
    gpio::PwmOutput _pwm_output;         ///< PWM output gpio

    KF_IMPL_ACTUATOR_DRIVER(PwmPositionServo, units::Degrees, bool());

    bool initImpl() noexcept {
        return _pwm_output.init();
    }

    void setImpl(units::Degrees angle) noexcept {
        _pwm_output.writePulse(this->config().pulseWidthFromAngle(_angle_safe_range.clamped(angle)));
    }

    void stopImpl() noexcept {
        _pwm_output.write(0);
    }
};

}// namespace kf::driver::actuator