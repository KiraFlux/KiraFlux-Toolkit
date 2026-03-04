// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/drivers/zms/PwmPositionServo.hpp"
#include "kf/math/units.hpp"
#include "kf/validation.hpp"

namespace kf::controllers {

/// @brief Two-degree-of-freedom robotic manipulator with servo control
/// @note Controls arm and claw axes using PWM-position servo drivers
struct Manipulator2DOF {

    static constexpr auto logger{Logger::create("Encoder")};

    /// @brief Configuration for 2DOF manipulator
    struct Config : Validatable<Config> {
        drivers::zms::PwmPositionServo::PwmConfig servo_pwm;                     ///< PWM signal
        drivers::zms::PwmPositionServo::PulseConfig servo_generic_pulse_settings;///< Pulse timing
        drivers::zms::PwmPositionServo::DriverConfig claw_axis;                  ///< Claw axis servo
        drivers::zms::PwmPositionServo::DriverConfig arm_axis;                   ///< Arm axis servo

        /// @brief Validate all configuration parameters
        void checkImpl(Validator &validator) const noexcept {
            kf_Validator_check(validator, logger, arm_axis.check());
            kf_Validator_check(validator, logger, claw_axis.check());
            kf_Validator_check(validator, logger, servo_pwm.check());
            kf_Validator_check(validator, logger, servo_generic_pulse_settings.check());
        }
    };

private:
    const Config &_config;               ///< Reference to configuration
    drivers::zms::PwmPositionServo _arm; ///< Arm axis servo driver
    drivers::zms::PwmPositionServo _claw;///< Claw axis servo driver

public:
    /// @brief Construct manipulator instance
    explicit Manipulator2DOF(const Config &config) noexcept :
        _config{config},
        _arm{config.servo_pwm, config.arm_axis, config.servo_generic_pulse_settings},
        _claw{config.servo_pwm, config.claw_axis, config.servo_generic_pulse_settings} {}

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
    void arm(math::Degrees angle) noexcept { _arm.set(angle); }

    /// @brief Set claw axis angle
    void claw(math::Degrees angle) noexcept { _claw.set(angle); }

    /// @brief Disable arm axis servo (stop PWM)
    void disableArm() noexcept { _arm.disable(); }

    /// @brief Disable claw axis servo (stop PWM)
    void disableClaw() noexcept { _claw.disable(); }
};

}// namespace kf::controllers