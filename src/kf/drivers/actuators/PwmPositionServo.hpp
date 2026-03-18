// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/algorithm.hpp"
#include "kf/gpio/GPIO.hpp"
#include "kf/math/units.hpp"
#include "kf/meta/type_check.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/validation.hpp"

namespace kf::drivers::actuators {

/// @brief PWM-controlled position servo driver for ESP32 LEDC hardware
/// @note Converts angular positions to PWM pulse widths for standard RC servos
template<typename I> struct PwmPositionServo final : mixin::Initable<PwmPositionServo, bool> {
    kf_crtp_check(I, kf::gpio::PwmOutputTag);
    using PwmPinImpl = I;

    static constexpr auto logger{Logger::create("PwmPositionServo")};

    /// @brief Servo driver hardware configuration
    struct DriverConfig : Validatable<DriverConfig> {
        math::Degrees min_angle;///< Minimum servo rotation angle
        math::Degrees max_angle;///< Maximum servo rotation angle

        /// @brief Validate driver configuration parameters
        void checkImpl(Validator &validator) const noexcept {
            kf_Validator_check(validator, logger, min_angle < max_angle);
        }
    };

    /// @brief Pulse width mapping configuration for servo angles
    struct PulseConfig : Validatable<PulseConfig> {
        /// @brief Angle-to-pulse width mapping point
        struct Pulse {
            math::Microseconds pulse;///< Pulse width in microseconds
            math::Degrees angle;     ///< Corresponding servo angle
        };

        Pulse min_pulse, max_pulse;///< Position mapping (angle <-> pulse width)

        /// @brief Convert angle to pulse width using linear interpolation
        /// @param angle Target servo angle
        /// @return Required pulse width in microseconds
        [[nodiscard]] math::Microseconds pulseWidthFromAngle(math::Degrees angle) const noexcept {
            return linearMap<i32>(
                kf::clamp(angle, min_pulse.angle, max_pulse.angle),
                min_pulse.angle,
                max_pulse.angle,
                min_pulse.pulse,
                max_pulse.pulse);
        }

        /// @brief Validate pulse mapping configuration
        void checkImpl(Validator &validator) const noexcept {
            kf_Validator_check(validator, logger, min_pulse.pulse < max_pulse.pulse);
            kf_Validator_check(validator, logger, min_pulse.angle < max_pulse.angle);
        }
    };

    /// @brief Construct servo driver instance
    /// @param pwm_settings PWM signal configuration
    /// @param driver_settings Servo hardware configuration
    /// @param pulse_settings Angle-pulse mapping configuration
    explicit constexpr PwmPositionServo(
        const DriverConfig &driver_settings,
        const PulseConfig &pulse_settings,
        PwmPinImpl &&pin) noexcept :
        _driver_settings{driver_settings}, _pulse_settings(pulse_settings), _pin{pin} {}

    /// @brief Set servo to target angle
    /// @param angle Target angle in degrees
    /// @note Automatically converts angle to PWM duty cycle
    void write(math::Degrees angle) noexcept {
        _pin.write(_pin.dutyFromPulseWidth(_pulse_settings.pulseWidthFromAngle(angle)));
    }

    /// @brief Disable servo (stop PWM signal)
    void disable() noexcept {
        _pin.write(0);
    }

private:
    const DriverConfig &_driver_settings;///< Servo hardware configuration
    const PulseConfig &_pulse_settings;  ///< Angle-pulse mapping configuration

    PwmPinImpl _pin;

    // Initable impl
    friend struct kf::mixin::Initable<PwmPositionServo, bool>;

    bool initImpl() noexcept {
        return _pin.init();
    }
};

}// namespace kf::drivers::actuators