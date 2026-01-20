// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"

namespace kf {

/// @brief Exponential moving average filter (EMA)
/// @tparam T Data type (typically float or integer)
/// @note Simple first-order IIR filter for smoothing noisy signals
template<typename T> struct ExponentialFilter {
    f32 k;     ///< Smoothing factor (0.0 to 1.0, higher = faster response)
    T filtered;///< Current filtered value

    /// @brief Construct exponential filter instance
    /// @param k Smoothing factor (0.0 to 1.0)
    /// @param init_value Initial filter state (default: zero-initialized)
    constexpr explicit ExponentialFilter(f32 k, T init_value = T{}) :
        k{k}, filtered{init_value} {}

    /// @brief Update filter with new sample
    /// @param value New input value
    /// @return Current filtered value after update
    kf_nodiscard const T &calc(const T &value) {
        filtered += (value - filtered) * k;
        return filtered;
    }
};

}// namespace kf