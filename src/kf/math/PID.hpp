// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>
#include <limits>

#include "kf/algorithm.hpp"
#include "kf/aliases.hpp"
#include "kf/math/filters/LowFrequencyFilter.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf::math {

/// @brief PID controller implementation
/// @note Includes derivative filtering and integral anti-windup
struct PID final : kf::mixin::Resettable<PID> {

    using FilterImpl = filters::LowFrequencyFilter<f32>;

    /// @brief PID controller tuning parameters
    struct Config {
        f32 p;           ///< Proportional gain coefficient
        f32 i;           ///< Integral gain coefficient
        f32 d;           ///< Derivative gain coefficient
        f32 i_limit;     ///< Integral term saturation limit
        f32 output_limit;///< Controller output saturation limit

        typename FilterImpl::Config dx_filter;///< Derivative filter
    };

    /// @brief Construct PID controller instance
    /// @param PID tuning parameters
    /// @param dx_filter_alpha Derivative filter smoothing factor (default: 1.0 = no filtering)
    explicit PID(const Config &config) noexcept :
        _config{config}, _dx_filter{config.dx_filter} {}

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
            _ix += error * dt;
            _ix = kf::clamp(_ix, -_config.i_limit, _config.i_limit);
        }

        if (_config.d != 0.0f and not std::isnan(_last_error)) {
            _dx = _dx_filter.calc((error - _last_error) / dt);
        } else {
            _dx = 0.0f;
        }
        _last_error = error;

        return kf::clamp(
            _config.p * error + _config.i * _ix + _config.d * _dx,
            -_config.output_limit,
            _config.output_limit);
    }

private:
    static constexpr auto nan = std::numeric_limits<f32>::quiet_NaN();

    const Config &_config;///< Reference to tuning parameters
    FilterImpl _dx_filter;///< Low-pass filter for derivative term
    f32 _dx{0};           ///< Current derivative value
    f32 _ix{0};           ///< Current integral value
    f32 _last_error{nan}; ///< Previous error value

    // impl
    using This = PID;

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        _dx = 0.0f;
        _ix = 0.0f;
        _last_error = nan;
    }
};

}// namespace kf::math