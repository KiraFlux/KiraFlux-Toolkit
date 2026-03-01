// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/drivers/zms/PwmPositionServo.hpp"
#include "kf/math/units.hpp"
#include "kf/validation.hpp"

namespace kf {

/// @brief Two-degree-of-freedom robotic manipulator with servo control
/// @note Controls arm and claw axes using PWM-position servo drivers
struct Manipulator2DOF {

    static constexpr auto logger{Logger::create("Encoder")};

    /// @brief Configuration settings for 2DOF manipulator
    struct Settings : Validatable<Settings> {
        PwmPositionServo::PwmSettings servo_pwm;                     ///< PWM signal configuration
        PwmPositionServo::PulseSettings servo_generic_pulse_settings;///< Pulse timing settings
        PwmPositionServo::DriverSettings claw_axis;                  ///< Claw axis servo configuration
        PwmPositionServo::DriverSettings arm_axis;                   ///< Arm axis servo configuration

        /// @brief Validate all configuration parameters
        void checkImpl(Validator &validator) const noexcept {
            kf_Validator_check(validator, logger, arm_axis.check());
            kf_Validator_check(validator, logger, claw_axis.check());
            kf_Validator_check(validator, logger, servo_pwm.check());
            kf_Validator_check(validator, logger, servo_generic_pulse_settings.check());
        }
    };

private:
    const Settings &settings;///< Reference to configuration settings
    PwmPositionServo _arm;   ///< Arm axis servo driver
    PwmPositionServo _claw;  ///< Claw axis servo driver

public:
    /// @brief Construct manipulator instance
    /// @param settings Configuration settings for both axes
    explicit Manipulator2DOF(const Settings &settings) noexcept :
        settings{settings},
        _arm{settings.servo_pwm, settings.arm_axis, settings.servo_generic_pulse_settings},
        _claw{settings.servo_pwm, settings.claw_axis, settings.servo_generic_pulse_settings} {}

    /// @brief Initialize both servo axes
    /// @return true if both servos initialized successfully
    /// @note Logs error message if initialization fails
    [[nodiscard]] bool init() noexcept {
        if (not _arm.init()) {
            logger.error("arm axis fail");
            return false;
        }

        if (not _claw.init()) {
            logger.error("claw axis fail");
            return false;
        }

        return true;
    }

    /// @brief Set arm axis angle
    void arm(Degrees angle) noexcept { _arm.set(angle); }

    /// @brief Set claw axis angle
    void claw(Degrees angle) noexcept { _claw.set(angle); }

    /// @brief Disable arm axis servo (stop PWM)
    void disableArm() noexcept { _arm.disable(); }

    /// @brief Disable claw axis servo (stop PWM)
    void disableClaw() noexcept { _claw.disable(); }
};

}// namespace kf