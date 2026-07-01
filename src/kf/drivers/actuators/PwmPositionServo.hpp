// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "kf/Range.hpp"
#include "kf/algorithm.hpp"
#include "kf/gpio/GPIO.hpp"
#include "kf/math/units.hpp"
#include "kf/mixin/Configurable.hpp"

#include "kf/drivers/actuators/Actuator.hpp"

namespace kf::internal {

struct PwmPositionServoConfig final {
    using AngleRange = Range<math::Degrees>;
    using PulseRange = Range<math::Microseconds>;

    AngleRange angle_range;
    PulseRange pulse_range;

    /// @brief Convert angle to pulse width using linear interpolation
    /// @param angle Target servo angle
    /// @return Required pulse width in microseconds
    [[nodiscard]] math::Microseconds pulseWidthFromAngle(math::Degrees angle) const noexcept {
        return linearMap<i32>(angle, angle_range.start, angle_range.end, pulse_range.start, pulse_range.end);
    }
};

}// namespace kf::internal

namespace kf::drivers::actuators {

/// @brief PWM-controlled position servo driver for ESP32 LEDC hardware
/// @note Converts angular positions to PWM pulse widths for standard RC servos
/// @tparam G Implementation of GPIO with PWM output support
template<typename G> struct PwmPositionServo final :

    Actuator<PwmPositionServo<G>, bool()>,
    mixin::Configurable<internal::PwmPositionServoConfig>

{
    KF_CHECK_IMPL(G, ::kf::gpio::GPIO::PwmOutputTag);
    using PwmPinImpl = G;

    /// @brief Configuration for PWM position servo (angle <-> pulse width mapping).
    /// @note Contains hardware‑independent mapping and is self‑validating.
    using Config = internal::PwmPositionServoConfig;

    /// @brief Construct servo with the same angle range for both mapping and safe operation.
    /// @param config Mapping between angle and pulse width.
    /// @param pin    PWM output pin.
    explicit constexpr PwmPositionServo(const Config &config, PwmPinImpl &&pin) noexcept :
        mixin::Configurable<Config>{config}, _angle_safe_range{config.angle_range}, _pin{std::move(pin)} {}

    /// @brief Construct servo with separate safe angle range (may be narrower than config range).
    /// @param config            Mapping between angle and pulse width.
    /// @param pin               PWM output pin.
    /// @param angle_safe_range  Additional clamping range for safety (e.g., to avoid mechanical limits).
    explicit constexpr PwmPositionServo(const Config &config, PwmPinImpl &&pin, Config::AngleRange angle_safe_range) noexcept :
        mixin::Configurable<Config>{config}, _angle_safe_range{angle_safe_range}, _pin{std::move(pin)} {}

    /// @brief Set servo to target angle.
    /// @param angle Target angle in degrees (will be clamped to safe range).
    /// @note Converts angle to pulse width using the configuration and writes the corresponding PWM duty cycle.
    void write(math::Degrees angle) noexcept {
        _pin.write(_pin.dutyFromPulseWidth(this->config().pulseWidthFromAngle(_angle_safe_range.clamped(angle))));
    }

    /// @brief Disable servo (stop PWM signal)
    void disable() noexcept { _pin.write(0); }

private:
    Config::AngleRange _angle_safe_range;///< Safe operating angle range (clamped before mapping).
    PwmPinImpl _pin;                     ///< PWM output pin.

    // impl
    using This = PwmPositionServo<G>;

    KF_IMPL_INITABLE(This, bool());
    bool initImpl() noexcept {
        return _pin.init();
    }

    KF_IMPL(Actuator<This, bool>);
};

}// namespace kf::drivers::actuators