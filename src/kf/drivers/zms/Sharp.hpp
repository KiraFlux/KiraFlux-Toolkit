// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/core/attributes.hpp"
#include "kf/math/units.hpp"
#include "kf/validation.hpp"

namespace kf {

/// @brief Sharp infrared distance sensor driver
/// @note Provides distance measurements in millimeters using analog voltage output
struct Sharp {

    /// @brief Raw ADC value type
    using AnalogValue = u16;

    /// @brief Sensor configuration settings
    struct Settings : Validable<Settings> {
        u8 pin;       ///< Analog input GPIO pin
        u8 resolution;///< ADC resolution in bits (1-16)

        /// @brief Calculate maximum ADC value based on resolution
        /// @return Maximum possible ADC value (2^resolution - 1)
        kf_nodiscard inline AnalogValue maxValue() const {
            return static_cast<AnalogValue>((1u << resolution) - 1u);
        }

        /// @brief Validate sensor configuration parameters
        /// @param validator Validation context
        void check(Validator &validator) const {
            kf_Validator_check(validator, resolution > 0);
            kf_Validator_check(validator, resolution <= 16);
        }
    };

    const Settings &settings;///< Reference to configuration settings

private:
    AnalogValue max_value{0};///< Cached maximum ADC value

public:
    explicit Sharp(const Settings &settings) :
        settings{settings} {}

    /// @brief Initialize sensor hardware
    /// @return Always returns true (initialization cannot fail)
    /// @note Sets pin mode and configures ADC resolution
    kf_nodiscard bool init() {
        max_value = settings.maxValue();

        pinMode(settings.pin, INPUT);
        analogReadResolution(settings.resolution);

        return true;
    }

    /// @brief Read raw ADC value from sensor
    /// @return Raw ADC reading (0 to maxValue)
    kf_nodiscard inline AnalogValue readRaw() const {
        return analogRead(settings.pin);
    }

    /// @brief Read distance in millimeters
    /// @return Distance in millimeters (inverse of voltage)
    /// @note Takes multiple samples for noise reduction
    kf_nodiscard Millimeters read() const {
        // 65535 / analogRead(a)
        long sum = 0;

        const auto n = 4;

        for (int i = 0; i < n; i += 1) {
            sum += readRaw();
            delay(1);
        }

        return (65535.0F * n) / Millimeters(sum);
    }
};

}// namespace kf