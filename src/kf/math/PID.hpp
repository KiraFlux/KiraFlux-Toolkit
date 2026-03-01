// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/aliases.hpp"
#include "kf/math/filters/LowFrequencyFilter.hpp"

namespace kf {

/// @brief PID controller implementation
/// @note Includes derivative filtering and integral anti-windup
struct PID {

public:
    /// @brief PID controller tuning parameters
    struct Config {
        f32 p;           ///< Proportional gain coefficient
        f32 i;           ///< Integral gain coefficient
        f32 d;           ///< Derivative gain coefficient
        f32 i_limit;     ///< Integral term saturation limit
        f32 output_limit;///< Controller output saturation limit
    };

private:
    const Config &_config;         ///< Reference to tuning parameters
    LowFrequencyFilter<f32> dx_filter;///< Low-pass filter for derivative term
    f32 dx{0};                        ///< Current derivative value
    f32 ix{0};                        ///< Current integral value
    f32 last_error{NAN};              ///< Previous error value

public:
    /// @brief Construct PID controller instance
    /// @param PID tuning parameters
    /// @param dx_filter_alpha Derivative filter smoothing factor (default: 1.0 = no filtering)
    explicit PID(const Config &config, f32 dx_filter_alpha = 1.0f) noexcept :
        _config{config}, dx_filter{dx_filter_alpha} {}

    /// @brief Calculate PID controller output
    /// @param error Current control error (setpoint - measurement)
    /// @param dt Time step in seconds since last calculation
    /// @return Controller output (saturated to output_limit)
    /// @note Skips calculation for invalid dt values (≤0 or >0.1s)
    [[nodiscard]] f32 calc(f32 error, f32 dt) noexcept {
        constexpr auto max_dt = 0.1f;

        if (dt <= 0.0f or dt > max_dt) {
            return 0.0f;
        }

        if (_config.i != 0.0f) {
            ix += error * dt;
            ix = constrain(ix, -_config.i_limit, _config.i_limit);
        }

        if (_config.d != 0.0f and not isnan(last_error)) {
            dx = dx_filter.calc((error - last_error) / dt);
        } else {
            dx = 0.0f;
        }
        last_error = error;

        const auto output = _config.p * error + _config.i * ix + _config.d * dx;
        return constrain(output, -_config.output_limit, _config.output_limit);
    }

    /// @brief Reset controller internal state (integral and derivative terms)
    void reset() noexcept {
        dx = 0.0f;
        ix = 0.0f;
        last_error = NAN;
    }
};

}// namespace kf