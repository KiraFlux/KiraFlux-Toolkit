// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/math/units.hpp"
#include "kf/validation.hpp"

namespace kf::drivers::sensors {

/// @brief Sharp infrared distance sensor driver
/// @note Provides distance measurements in millimeters using analog voltage output
struct Sharp {

    /// @brief Raw ADC value type
    using AnalogValue = u16;

    static constexpr auto logger{Logger::create("Encoder")};

    /// @brief Sensor configuration
    struct Config : Validatable<Config> {
        u8 pin;       ///< Analog input GPIO pin
        u8 resolution;///< ADC resolution in bits (1-16)

        /// @brief Calculate maximum ADC value based on resolution
        /// @return Maximum possible ADC value (2^resolution - 1)
        [[nodiscard]] constexpr AnalogValue maxValue() const noexcept {
            return static_cast<AnalogValue>((1u << resolution) - 1u);
        }

        /// @brief Validate sensor configuration parameters
        /// @param validator Validation context
        void checkImpl(Validator &validator) const noexcept {
            kf_Validator_check(validator, logger, resolution > 0);
            kf_Validator_check(validator, logger, resolution <= 16);
        }
    };

    const Config &_config;

private:
    AnalogValue _max_value{0};///< Cached maximum ADC value

public:
    explicit Sharp(const Config &config) noexcept :
        _config{config} {}

    /// @brief Initialize sensor hardware
    /// @return Always returns true (initialization cannot fail)
    /// @note Sets pin mode and configures ADC resolution
    [[nodiscard]] bool init() noexcept {
        _max_value = _config.maxValue();

        pinMode(_config.pin, INPUT);
        analogReadResolution(_config.resolution);

        return true;
    }

    /// @brief Read raw ADC value from sensor
    /// @return Raw ADC reading (0 to maxValue)
    [[nodiscard]] inline AnalogValue readRaw() const noexcept {
        return analogRead(_config.pin);
    }

    /// @brief Read distance in millimeters
    /// @return Distance in millimeters (inverse of voltage)
    /// @note Takes multiple samples for noise reduction
    [[nodiscard]] math::Millimeters read() const noexcept {
        // 65535 / analogRead(a)
        long sum = 0;

        const auto n = 4;

        for (int i = 0; i < n; i += 1) {
            sum += readRaw();
            delay(1);
        }

        return (65535.0F * n) / math::Millimeters(sum);
    }
};

}// namespace kf