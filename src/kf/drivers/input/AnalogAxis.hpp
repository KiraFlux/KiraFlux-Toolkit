// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"
#include "kf/math/filters/ExponentialFilter.hpp"

namespace kf {

/// @brief Single analog joystick axis with filtering and dead-zone compensation
/// @note Uses ESP32's 12-bit ADC (0-4095 range) with configurable filtering
struct AnalogAxis final {

private:
    /// @brief Maximum raw analog value (12-bit ADC)
    static constexpr auto max_analog_value = 4095;

    /// @brief Default center position (half of ADC range)
    static constexpr auto default_analog_center = max_analog_value / 2;

public:
    bool inverted{false};///< Invert axis direction (swap positive/negative)

private:
    const u8 pin;                                                ///< GPIO pin for analog input
    ExponentialFilter<f32> filter;                               ///< Exponential smoothing filter
    i16 range_negative{default_analog_center};                   ///< Negative range from center
    i16 range_positive{max_analog_value - default_analog_center};///< Positive range from center

public:
    i16 dead_zone{0};///< Raw value dead zone (absolute units)

    /// @brief Construct analog axis instance
    /// @param pin GPIO pin for analog input
    /// @param k Filter coefficient (0.0 to 1.0, higher = more smoothing)
    explicit AnalogAxis(gpio_num_t pin, f32 k) noexcept :
        pin{static_cast<u8>(pin)}, filter{k} {}

    /// @brief Initialize axis hardware (set pin mode)
    inline void init() const noexcept {
        pinMode(pin, INPUT);
    }

    /// @brief Update calibration center point
    /// @param new_center New center position in raw ADC units
    /// @note Adjusts range calculations for normalization
    void updateCenter(i16 new_center) noexcept {
        range_negative = new_center;
        range_positive = static_cast<i16>(max_analog_value - new_center);
    }

    /// @brief Read raw ADC value without processing
    /// @return Raw 12-bit ADC reading (0-4095)
    kf_nodiscard inline int readRaw() const noexcept {
        return analogRead(pin);
    }

    /// @brief Read normalized axis position
    /// @return Filtered value normalized to [-1.0, 1.0] range
    /// @note Applies dead zone, filtering, and optional inversion
    kf_nodiscard f32 read() noexcept {
        return inverted ? -pureRead() : pureRead();
    }

private:
    /// @brief Internal normalized reading without inversion
    /// @return Filtered normalized value [0.0, 1.0] without direction
    kf_nodiscard f32 pureRead() noexcept {
        const auto deviation = readRaw() - range_negative;

        if (std::abs(deviation) < dead_zone) {
            return 0.0f;
        }

        const auto filtered = filter.calc(static_cast<f32>(deviation));

        if (filtered < 0.0f) {
            return filtered / static_cast<f32>(range_negative);
        } else {
            return filtered / static_cast<f32>(range_positive);
        }
    }
};
}// namespace kf