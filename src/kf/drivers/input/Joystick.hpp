// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/drivers/input/AnalogAxis.hpp"

namespace kf {

/// @brief Two-axis joystick with calibration support
/// @note Uses filtered analog inputs and includes dead-zone compensation
struct Joystick final {
    AnalogAxis axis_x;///< X-axis input (horizontal movement)
    AnalogAxis axis_y;///< Y-axis input (vertical movement)

    /// @brief Construct joystick instance
    /// @param pin_x GPIO pin for X-axis analog input
    /// @param pin_y GPIO pin for Y-axis analog input
    /// @param filter_k Filter coefficient for axis smoothing (0.0 to 1.0)
    explicit Joystick(gpio_num_t pin_x, gpio_num_t pin_y, f32 filter_k) noexcept :
        axis_x{pin_x, filter_k},
        axis_y{pin_y, filter_k} {}

    /// @brief Initialize joystick hardware (ADC channels)
    inline void init() const noexcept {
        axis_x.init();
        axis_y.init();
    }

    /// @brief Calibrate joystick center position and dead zones
    /// @param samples Number of samples to collect for calibration
    /// @note Performs two-pass calibration: center position then maximum deviation
    /// @warning Must be called with joystick in neutral position for accurate calibration
    void calibrate(int samples) noexcept {
        constexpr auto period_ms = 1;

        i64 sum_x = 0;
        i64 sum_y = 0;

        // First pass: calculate precise center
        for (int i = 0; i < samples; i++) {
            sum_x += axis_x.readRaw();
            sum_y += axis_y.readRaw();
            delay(period_ms);
        }

        const auto center_x = static_cast<int>(sum_x / samples);
        const auto center_y = static_cast<int>(sum_y / samples);

        // Second pass: calculate maximum deviation
        int max_dev_x = 0;
        int max_dev_y = 0;

        for (int i = 0; i < samples; i++) {
            const int x = axis_x.readRaw();
            const int y = axis_y.readRaw();

            max_dev_x = std::max(max_dev_x, std::abs(x - center_x));
            max_dev_y = std::max(max_dev_y, std::abs(y - center_y));

            delay(period_ms);
        }

        // Set center and dead zone
        axis_x.updateCenter(center_x);
        axis_y.updateCenter(center_y);

        // Dead zone = 150% of maximum deviation + small margin
        axis_x.dead_zone = max_dev_x * 3 / 2 + 10;
        axis_y.dead_zone = max_dev_y * 3 / 2 + 10;
    }

    /// @brief Normalized joystick reading data
    struct Data {
        f32 x;        ///< Normalized X-axis value (-1.0 to 1.0)
        f32 y;        ///< Normalized Y-axis value (-1.0 to 1.0)
        f32 magnitude;///< Combined vector magnitude (0.0 to 1.0)
    };

    /// @brief Read current joystick position with normalization
    /// @return Normalized X, Y coordinates and magnitude
    /// @note Output is normalized to unit circle (clamped at magnitude 1.0)
    Data read() noexcept {
        const auto x = axis_x.read();
        const auto y = axis_y.read();
        const auto h = std::hypot(x, y);

        if (h < 1e-3) { return {0, 0, 0}; }
        if (h > 1) { return {x / h, y / h, 1}; }
        return {x, y, h};
    }
};
}// namespace kf