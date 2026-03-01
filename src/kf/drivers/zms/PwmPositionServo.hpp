// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/math/units.hpp"
#include "kf/validation.hpp"

namespace kf {

/// @brief PWM-controlled position servo driver for ESP32 LEDC hardware
/// @note Converts angular positions to PWM pulse widths for standard RC servos
struct PwmPositionServo {

    static constexpr auto logger{Logger::create("PwmPositionServo")};

    /// @brief PWM signal configuration for LEDC hardware
    struct PwmConfig : Validatable<PwmConfig> {
        u32 ledc_frequency_hz;  ///< PWM frequency in Hz
        u8 ledc_resolution_bits;///< PWM resolution (8-16 bits)

        /// @brief Calculate maximum duty cycle value
        /// @return Maximum duty cycle value (2^resolution - 1)
        [[nodiscard]] constexpr u32 maxDuty() const noexcept {
            return (1u << ledc_resolution_bits) - 1u;
        }

        /// @brief Convert pulse width to duty cycle value
        /// @param pulse_width Pulse width in microseconds
        /// @return Duty cycle value for LEDC hardware
        [[nodiscard]] u16 dutyFromPulseWidth(Microseconds pulse_width) const noexcept {
            const auto t = u64{pulse_width} * ledc_frequency_hz * maxDuty();
            return u16{t / 1'000'000u};
        }

        /// @brief Validate PWM configuration parameters
        void checkImpl(Validator &validator) const noexcept {
            kf_Validator_check(validator, logger, ledc_frequency_hz > 0);
            kf_Validator_check(validator, logger, ledc_resolution_bits >= 8);
            kf_Validator_check(validator, logger, ledc_resolution_bits <= 16);
        }
    };

    /// @brief Servo driver hardware configuration
    struct DriverConfig : Validatable<DriverConfig> {
        u8 signal_pin;    ///< GPIO pin for PWM signal output
        u8 ledc_channel;  ///< LEDC channel (0-15) for ESP32 PWM
        Degrees min_angle;///< Minimum servo rotation angle
        Degrees max_angle;///< Maximum servo rotation angle

        /// @brief Validate driver configuration parameters
        void checkImpl(Validator &validator) const noexcept {
            kf_Validator_check(validator, logger, ledc_channel <= 15);
            kf_Validator_check(validator, logger, min_angle < max_angle);
        }
    };

    /// @brief Pulse width mapping configuration for servo angles
    struct PulseConfig : Validatable<PulseConfig> {
        /// @brief Angle-to-pulse width mapping point
        struct Pulse {
            Microseconds pulse;///< Pulse width in microseconds
            Degrees angle;     ///< Corresponding servo angle
        };

        Pulse min_pulse, max_pulse;///< Position mapping (angle <-> pulse width)

        /// @brief Convert angle to pulse width using linear interpolation
        /// @param angle Target servo angle
        /// @return Required pulse width in microseconds
        [[nodiscard]] Microseconds pulseWidthFromAngle(Degrees angle) const noexcept {
            return map(
                constrain(angle, min_pulse.angle, max_pulse.angle),
                min_pulse.angle,
                max_pulse.angle,
                static_cast<long>(min_pulse.pulse),
                static_cast<long>(max_pulse.pulse));
        }

        /// @brief Validate pulse mapping configuration
        void checkImpl(Validator &validator) const noexcept {
            kf_Validator_check(validator, logger, min_pulse.pulse < max_pulse.pulse);
            kf_Validator_check(validator, logger, min_pulse.angle < max_pulse.angle);
        }
    };

private:
    const PwmConfig &pwm_settings;      ///< PWM signal configuration
    const DriverConfig &driver_settings;///< Servo hardware configuration
    const PulseConfig &pulse_settings;  ///< Angle-pulse mapping configuration

public:
    /// @brief Construct servo driver instance
    /// @param pwm_settings PWM signal configuration
    /// @param driver_settings Servo hardware configuration
    /// @param pulse_settings Angle-pulse mapping configuration
    explicit constexpr PwmPositionServo(
        const PwmConfig &pwm_settings,
        const DriverConfig &driver_settings,
        const PulseConfig &pulse_settings) noexcept :
        driver_settings{driver_settings}, pwm_settings(pwm_settings), pulse_settings(pulse_settings) {}

    /// @brief Initialize servo driver hardware
    /// @return true if PWM channel setup successful
    /// @note Configures ESP32 LEDC hardware for PWM generation
    [[nodiscard]] bool init() const noexcept {
        const auto freq = ledcSetup(
            driver_settings.ledc_channel,
            pwm_settings.ledc_frequency_hz,
            pwm_settings.ledc_resolution_bits);

        if (0 == freq) {
            logger.error("LEDC setup failed");
            return false;
        }

        ledcAttachPin(driver_settings.signal_pin, driver_settings.ledc_channel);

        return true;
    }

    /// @brief Set servo to target angle
    /// @param angle Target angle in degrees
    /// @note Automatically converts angle to PWM duty cycle
    void set(Degrees angle) noexcept {
        write(pwm_settings.dutyFromPulseWidth(pulse_settings.pulseWidthFromAngle(angle)));
    }

    /// @brief Disable servo (stop PWM signal)
    void disable() noexcept {
        write(0);
    }

private:
    /// @brief Write duty cycle value to LEDC hardware
    /// @param duty Duty cycle value (0 to maxDuty)
    void write(u16 duty) const noexcept {
        ledcWrite(driver_settings.ledc_channel, duty);
    }
};

}// namespace kf