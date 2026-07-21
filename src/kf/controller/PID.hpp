// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Option.hpp"
#include "kf/filter/LowFrequencyFilter.hpp"
#include "kf/math.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Resettable.hpp"
#include "kf/primitives.hpp"

namespace kf::internal {

using PidFilterImpl = filter::LowFrequencyFilter<f32>;

struct PidConfig final {
    f32 proportional_gain;///< Proportional gain coefficient
    f32 integral_gain;    ///< Integral gain coefficient
    f32 derivative_gain;  ///< Derivative gain coefficient
    f32 integral_limit;   ///< Integral term saturation limit
    f32 output_limit;     ///< Controller output saturation limit
    f32 max_dt;           ///< Max valid dt

    typename PidFilterImpl::Config derivative_filter;///< Derivative filter config

    [[nodiscard]] constexpr f32 calc(f32 x, f32 ix, f32 dx) const noexcept {
        return math::clamp(proportional_gain * x + integral_gain * ix + derivative_gain * dx, -output_limit, output_limit);
    }
};

}// namespace kf::internal

namespace kf::controller {

/// @brief PID controller implementation
/// @note Includes derivative filtering and integral anti-windup
struct PID final :

    mixin::NonCopyable,
    mixin::Resettable<PID>,
    mixin::Configured<internal::PidConfig>

{

    using FilterImpl = internal::PidFilterImpl;

    /// @brief PID controller tuning parameters
    using Config = internal::PidConfig;

    /// @brief Construct PID controller instance
    /// @param PID tuning parameters
    /// @param dx_filter_alpha Derivative filter smoothing factor (default: 1.0 = no filtering)
    explicit PID(Config const &config) noexcept :
        mixin::Configured<Config>{config}, _derivative_filter{config.derivative_filter} {}

    /// @brief Calculate PID controller output
    /// @param error Current control error (setpoint - measurement)
    /// @param dt Time step in seconds since last calculation
    /// @return Controller output (saturated to output_limit)
    /// @note Skips calculation for invalid dt values (<= 0 or >0.1s)
    [[nodiscard]] f32 calc(f32 error, f32 dt) noexcept {
        if (dt <= 0.0f or dt > this->config().max_dt) {
            return 0.0f;
        }

        if (this->config().integral_gain != 0.0f) {
            _current_integral += error * dt;
            _current_integral = math::clamp(_current_integral, -this->config().integral_limit, this->config().integral_limit);
        }

        if (this->config().derivative_gain != 0.0f and _last_error.isSome()) {
            _current_derivative = _derivative_filter.calc((error - _last_error.unwrap()) / dt);
        } else {
            _current_derivative = 0.0f;
        }
        _last_error = someTrivial(error);

        return this->config().calc(error, _current_integral, _current_derivative);
    }

private:
    FilterImpl _derivative_filter;       ///< Low-pass filter for derivative term
    f32 _current_derivative{0};          ///< Current derivative value
    f32 _current_integral{0};            ///< Current integral value
    TrivialOption<f32> _last_error{none};///< Previous error value

    KF_IMPL_RESETTABLE(PID);
    constexpr void resetImpl() noexcept {
        _current_derivative = 0.0f;
        _current_integral = 0.0f;
        _last_error.reset();
    }
};

}// namespace kf::controller