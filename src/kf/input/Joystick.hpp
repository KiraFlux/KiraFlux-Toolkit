// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "AnalogAxis.hpp"
#include "kf/Tuner.hpp"


namespace kf {

/// @brief Two-axis joystick with calibration support
/// @note Uses filtered analog inputs and includes dead-zone compensation
struct Joystick final {

    struct Config {
        AnalogAxis::Config x;
        AnalogAxis::Config y;
    };

    AnalogAxis axis_x;///< X-axis input (horizontal movement)
    AnalogAxis axis_y;///< Y-axis input (vertical movement)

    explicit Joystick(const Config &config, f32 filter_k) noexcept:
        axis_x{config.x, filter_k},
        axis_y{config.y, filter_k} {}

    inline void init() const noexcept {
        axis_x.init();
        axis_y.init();
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