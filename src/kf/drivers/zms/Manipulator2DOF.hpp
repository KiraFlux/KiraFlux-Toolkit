// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/attributes.hpp"
#include "kf/drivers/zms/PwmPositionServo.hpp"
#include "kf/math/units.hpp"
#include "kf/validation.hpp"

namespace kf {

/// @brief Two-degree-of-freedom robotic manipulator with servo control
/// @note Controls arm and claw axes using PWM-position servo drivers
struct Manipulator2DOF {

    /// @brief Configuration settings for 2DOF manipulator
    struct Settings : Validable<Settings> {
        PwmPositionServo::PwmSettings servo_pwm;                     ///< PWM signal configuration
        PwmPositionServo::PulseSettings servo_generic_pulse_settings;///< Pulse timing settings
        PwmPositionServo::DriverSettings claw_axis;                  ///< Claw axis servo configuration
        PwmPositionServo::DriverSettings arm_axis;                   ///< Arm axis servo configuration

        /// @brief Validate all configuration parameters
        /// @param validator Validation context
        void check(Validator &validator) const {
            kf_Validator_check(validator, arm_axis.isValid());
            kf_Validator_check(validator, claw_axis.isValid());
            kf_Validator_check(validator, servo_pwm.isValid());
            kf_Validator_check(validator, servo_generic_pulse_settings.isValid());
        }
    };

private:
    const Settings &settings;  ///< Reference to configuration settings
    PwmPositionServo arm_axis; ///< Arm axis servo driver
    PwmPositionServo claw_axis;///< Claw axis servo driver

public:
    /// @brief Construct manipulator instance
    /// @param settings Configuration settings for both axes
    explicit Manipulator2DOF(const Settings &settings) :
        settings{settings},
        arm_axis{settings.servo_pwm, settings.arm_axis, settings.servo_generic_pulse_settings},
        claw_axis{settings.servo_pwm, settings.claw_axis, settings.servo_generic_pulse_settings} {}

    /// @brief Initialize both servo axes
    /// @return true if both servos initialized successfully
    /// @note Logs error message if initialization fails
    kf_nodiscard bool init() {
        if (not arm_axis.init()) {
            kf_Logger_error("arm axis fail");
            return false;
        }

        if (not claw_axis.init()) {
            kf_Logger_error("claw axis fail");
            return false;
        }

        return true;
    }

    /// @brief Set arm axis angle
    /// @param angle Target angle in degrees
    inline void setArm(Degrees angle) { arm_axis.set(angle); }

    /// @brief Set claw axis angle
    /// @param angle Target angle in degrees
    inline void setClaw(Degrees angle) { claw_axis.set(angle); }

    /// @brief Disable arm axis servo (stop PWM)
    inline void disableArm() { arm_axis.disable(); }

    /// @brief Disable claw axis servo (stop PWM)
    inline void disableClaw() { claw_axis.disable(); }
};

}// namespace kf