// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/core/attributes.hpp"
#include "kf/math/units.hpp"
#include "kf/validation.hpp"

namespace kf {

/// @brief PWM-controlled position servo driver for ESP32 LEDC hardware
/// @note Converts angular positions to PWM pulse widths for standard RC servos
struct PwmPositionServo {

    /// @brief PWM signal configuration for LEDC hardware
    struct PwmSettings : Validable<PwmSettings> {
        u32 ledc_frequency_hz;  ///< PWM frequency in Hz
        u8 ledc_resolution_bits;///< PWM resolution (8-16 bits)

        /// @brief Calculate maximum duty cycle value
        /// @return Maximum duty cycle value (2^resolution - 1)
        kf_nodiscard constexpr u32 maxDuty() const noexcept {
            return (1u << ledc_resolution_bits) - 1u;
        }

        /// @brief Convert pulse width to duty cycle value
        /// @param pulse_width Pulse width in microseconds
        /// @return Duty cycle value for LEDC hardware
        kf_nodiscard u16 dutyFromPulseWidth(Milliseconds pulse_width) const noexcept {
            const auto t = u64(pulse_width) * ledc_frequency_hz * maxDuty();
            return u16(t / 1000000u);
        }

        /// @brief Validate PWM configuration parameters
        /// @param validator Validation context
        void check(Validator &validator) const noexcept {
            kf_Validator_check(validator, ledc_frequency_hz > 0);
            kf_Validator_check(validator, ledc_resolution_bits >= 8);
            kf_Validator_check(validator, ledc_resolution_bits <= 16);
        }
    };

    /// @brief Servo driver hardware configuration
    struct DriverSettings : Validable<DriverSettings> {
        u8 signal_pin;    ///< GPIO pin for PWM signal output
        u8 ledc_channel;  ///< LEDC channel (0-15) for ESP32 PWM
        Degrees min_angle;///< Minimum servo rotation angle
        Degrees max_angle;///< Maximum servo rotation angle

        /// @brief Validate driver configuration parameters
        /// @param validator Validation context
        void check(Validator &validator) const noexcept {
            kf_Validator_check(validator, ledc_channel <= 15);
            kf_Validator_check(validator, min_angle < max_angle);
        }
    };

    /// @brief Pulse width mapping configuration for servo angles
    struct PulseSettings : Validable<PulseSettings> {
        /// @brief Angle-to-pulse width mapping point
        struct Pulse {
            Microseconds pulse;///< Pulse width in microseconds
            Degrees angle;     ///< Corresponding servo angle
        };

        Pulse min_position;///< Minimum position mapping (angle ↔ pulse width)
        Pulse max_position;///< Maximum position mapping (angle ↔ pulse width)

        /// @brief Convert angle to pulse width using linear interpolation
        /// @param angle Target servo angle
        /// @return Required pulse width in microseconds
        kf_nodiscard Microseconds pulseWidthFromAngle(Degrees angle) const noexcept {
            return map(
                constrain(angle, min_position.angle, max_position.angle),
                min_position.angle,
                max_position.angle,
                static_cast<long>(min_position.pulse),
                static_cast<long>(max_position.pulse));
        }

        /// @brief Validate pulse mapping configuration
        /// @param validator Validation context
        void check(Validator &validator) const noexcept {
            kf_Validator_check(validator, min_position.pulse < max_position.pulse);
            kf_Validator_check(validator, min_position.angle < max_position.angle);
        }
    };

private:
    const PwmSettings &pwm_settings;      ///< PWM signal configuration
    const DriverSettings &driver_settings;///< Servo hardware configuration
    const PulseSettings &pulse_settings;  ///< Angle-pulse mapping configuration

public:
    /// @brief Construct servo driver instance
    /// @param pwm_settings PWM signal configuration
    /// @param driver_settings Servo hardware configuration
    /// @param pulse_settings Angle-pulse mapping configuration
    explicit constexpr PwmPositionServo(
        const PwmSettings &pwm_settings,
        const DriverSettings &driver_settings,
        const PulseSettings &pulse_settings) noexcept:
        driver_settings{driver_settings}, pwm_settings(pwm_settings), pulse_settings(pulse_settings) {}

    /// @brief Initialize servo driver hardware
    /// @return true if PWM channel setup successful
    /// @note Configures ESP32 LEDC hardware for PWM generation
    kf_nodiscard bool init() const noexcept {
        const auto freq = ledcSetup(
            driver_settings.ledc_channel,
            pwm_settings.ledc_frequency_hz,
            pwm_settings.ledc_resolution_bits);

        if (freq == 0) {
            kf_Logger_error("LEDC setup failed");
            return false;
        }

        ledcAttachPin(driver_settings.signal_pin, driver_settings.ledc_channel);

        return true;
    }

    /// @brief Set servo to target angle
    /// @param angle Target angle in degrees
    /// @note Automatically converts angle to PWM duty cycle
    void set(Degrees angle) noexcept{
        write(pwm_settings.dutyFromPulseWidth(pulse_settings.pulseWidthFromAngle(angle)));
    }

    /// @brief Disable servo (stop PWM signal)
    void disable() noexcept{
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