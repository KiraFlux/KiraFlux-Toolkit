// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "kf/Range.hpp"
#include "kf/concepts.hpp"
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
/// @tparam G Implementation of GPIO with PWM output support
template<implements<gpio::PwmOutputTag> G> struct PwmPositionServo final :

    ActuatorDriver<PwmPositionServo<G>, units::Degrees, bool()>,
    mixin::Configured<internal::PwmPositionServoConfig>

{
    using PwmOutputImpl = G;

    /// @brief Configuration for PWM position servo (angle <-> pulse width mapping)
    /// @note Contains hardware‑independent mapping and is self‑validating
    using Config = internal::PwmPositionServoConfig;

    /// @brief Construct servo with the same angle range for both mapping and safe operation
    /// @param config Mapping between angle and pulse width
    /// @param gpio    PWM output gpio
    explicit constexpr PwmPositionServo(Config const &config, PwmOutputImpl &&gpio) noexcept :
        mixin::Configured<Config>{config}, _angle_safe_range{config.angle_range}, _pwm_gpio{std::move(gpio)} {}

    /// @brief Construct servo with separate safe angle range (may be narrower than config range)
    /// @param config Mapping between angle and pulse width
    /// @param gpio PWM output gpio
    /// @param angle_safe_range Additional clamping range for safety (e.g., to avoid mechanical limits)
    explicit constexpr PwmPositionServo(Config const &config, PwmOutputImpl &&gpio, Config::AngleRange angle_safe_range) noexcept :
        mixin::Configured<Config>{config}, _angle_safe_range{angle_safe_range}, _pwm_gpio{std::move(gpio)} {}

private:
    Config::AngleRange _angle_safe_range;///< Safe operating angle range (clamped before mapping)
    PwmOutputImpl _pwm_gpio;             ///< PWM output gpio

    KF_IMPL_ACTUATOR_DRIVER(PwmPositionServo<G>, units::Degrees, bool());

    bool initImpl() noexcept {
        return _pwm_gpio.init();
    }

    void setImpl(units::Degrees angle) noexcept {
        _pwm_gpio.writePulse(this->config().pulseWidthFromAngle(_angle_safe_range.clamped(angle)));
    }

    void stopImpl() noexcept {
        _pwm_gpio.write(0);
    }
};

}// namespace kf::driver::actuator