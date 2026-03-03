// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/Logger.hpp"
#include "kf/math/units.hpp"
#include "kf/validation.hpp"
#include "kf/algorithm.hpp"

namespace kf {

/// @brief Motor driver supporting IArduino Motor Shield and L298N/L293D H-bridge
/// @note Provides bidirectional PWM control with configurable dead zone and direction
struct Motor {

    /// @brief Signed PWM value type for bidirectional control
    using SignedPwm = i16;

    /// @brief Motor driver hardware implementation type
    enum class DriverImpl : u8 {
        IArduino = 0x00,   ///< IArduino Motor Shield (L298P)
        L298nModule = 0x01,///< Generic L298N/L293D H-bridge module
    };

    /// @brief Positive rotation direction definition
    enum class Direction : u8 {
        CW = 0x00,///< Clockwise rotation is positive
        CCW = 0x01///< Counter-clockwise rotation is positive
    };

    static constexpr auto logger{Logger::create("Motor")};

    /// @brief Motor driver hardware configuration
    struct DriverConfig : Validatable<DriverConfig> {
        DriverImpl impl;    ///< Driver hardware type
        Direction direction;///< Positive rotation direction
        u8 pin_a;           ///< Direction pin A (IArduino) / IN1/IN3 (L298N)
        u8 pin_b;           ///< PWM pin (IArduino) / IN2/IN4 (L298N)
        u8 ledc_channel;    ///< LEDC channel (0-15) for ESP32 PWM

        /// @brief Validate driver configuration parameters
        /// @param validator Validation context
        void checkImpl(Validator &validator) const noexcept {
            kf_Validator_check(validator, logger, ledc_channel <= 15);
        }
    };

    /// @brief PWM signal configuration
    struct PwmConfig : Validatable<PwmConfig> {
        using FrequencyScalar = u16;///< PWM frequency in Hz

        FrequencyScalar ledc_frequency_hz;///< PWM frequency (Hz)
        SignedPwm dead_zone;              ///< PWM dead zone value
        u8 ledc_resolution_bits;          ///< PWM resolution (8-12 bits)

        /// @brief Calculate maximum PWM value based on resolution
        /// @return Maximum PWM value (2^resolution - 1)
        [[nodiscard]] constexpr SignedPwm maxPwm() const noexcept {
            return static_cast<SignedPwm>((1u << ledc_resolution_bits) - 1u);
        }

        /// @brief Validate PWM configuration parameters
        /// @param validator Validation context
        void checkImpl(Validator &validator) const noexcept {
            kf_Validator_check(validator, logger, dead_zone >= 0);
            kf_Validator_check(validator, logger, ledc_resolution_bits >= 8);
            kf_Validator_check(validator, logger, ledc_resolution_bits <= 12);
        }
    };

    const DriverConfig &driver_settings;///< Driver hardware configuration
    const PwmConfig &pwm_settings;      ///< PWM signal configuration

private:
    SignedPwm max_pwm{0};///< Cached maximum PWM value

public:
    explicit constexpr Motor(const DriverConfig &driver_settings,
                             const PwmConfig &pwm_settings) noexcept :
        driver_settings{driver_settings}, pwm_settings{pwm_settings} {}

    /// @brief Initialize motor driver hardware
    /// @return true if initialization successful
    /// @note Configures GPIO pins and PWM channels based on driver type
    [[nodiscard]] bool init() noexcept {
        max_pwm = pwm_settings.maxPwm();

        pinMode(driver_settings.pin_a, OUTPUT);
        pinMode(driver_settings.pin_b, OUTPUT);

        switch (driver_settings.impl) {
            case DriverImpl::IArduino: {
                logger.debug("IArduino mode");

                const auto current_frequency = ledcSetup(
                    driver_settings.ledc_channel,
                    pwm_settings.ledc_frequency_hz,
                    pwm_settings.ledc_resolution_bits);

                if (current_frequency == 0) {
                    logger.error("LEDC setup failed!");
                    return false;
                }

                ledcAttachPin(driver_settings.pin_b, driver_settings.ledc_channel);
            } break;

            case DriverImpl::L298nModule: {
                logger.debug("L293n mode");

                analogWriteFrequency(pwm_settings.ledc_frequency_hz);
                analogWriteResolution(pwm_settings.ledc_resolution_bits);
            } break;
        }

        stop();

        logger.debug("isOk");
        return true;
    }

    /// @brief Set motor speed from normalized value
    /// @param value Normalized speed (-1.0 to 1.0)
    /// @note Applies dead zone and converts to PWM with direction
    void set(float value) const noexcept {
        write(fromNormalized(value));
    }

    /// @brief Stop motor (set PWM to zero)
    inline void stop() const noexcept {
        write(0);
    }

    /// @brief Write signed PWM value with direction control
    /// @param pwm Signed PWM value (-max to +max)
    /// @note Positive values rotate in positive direction (as configured)
    void write(SignedPwm pwm) const noexcept {
        pwm = kf::clamp(pwm, static_cast<SignedPwm>(-max_pwm), max_pwm);

        switch (driver_settings.impl) {
            case DriverImpl::IArduino: {
                digitalWrite(driver_settings.pin_a, matchDirection(pwm));
                ledcWrite(driver_settings.ledc_channel, std::abs(pwm));
            }
                return;

            case DriverImpl::L298nModule: {
                const bool positive_direction = matchDirection(pwm);
                if (positive_direction) {
                    analogWrite(driver_settings.pin_a, std::abs(pwm));
                    analogWrite(driver_settings.pin_b, 0);
                } else {
                    analogWrite(driver_settings.pin_a, 0);
                    analogWrite(driver_settings.pin_b, std::abs(pwm));
                }
            }
                return;
        }
    }

private:
    /// @brief Map signed PWM to direction signal
    /// @param pwm Signed PWM value
    /// @return true for positive direction, false for negative
    [[nodiscard]] inline bool matchDirection(SignedPwm pwm) const noexcept {
        const bool positive = pwm > 0;
        return driver_settings.direction == Direction::CW == positive;
    }

    /// @brief Convert normalized value to signed PWM
    /// @param value Normalized speed (-1.0 to 1.0)
    /// @return Signed PWM value with dead zone applied
    [[nodiscard]] SignedPwm fromNormalized(float value) const noexcept {
        constexpr auto normalized_dead_zone = 1e-2f;

        if (std::isnan(value)) { return 0; }

        const auto abs_value = std::abs(kf::clamp(value, -1.0f, +1.0f));
        if (abs_value < normalized_dead_zone) { return 0; }

        const auto ret = int(abs_value * float(max_pwm - pwm_settings.dead_zone)) + pwm_settings.dead_zone;
        return static_cast<SignedPwm>((value > 0.0f) ? ret : -ret);
    }
};

}// namespace kf