// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"

namespace kf::math::filters {

/// @brief Exponential moving average filter (EMA)
/// @tparam T Data type (typically float or integer)
/// @note Simple first-order IIR filter for smoothing noisy signals
template<typename T> struct ExponentialFilter {
    struct Config {
        f32 factor;///< Smoothing factor (0.0 to 1.0, higher = faster response)
    };

    constexpr explicit ExponentialFilter(const Config &config) noexcept : _config{config} {}

    /// @brief Update filter with new sample
    /// @param value New input value
    /// @return Current filtered value after update
    [[nodiscard]] const T &calc(const T &value) noexcept {
        current_filtered += (value - current_filtered) * _config.factor;
        return current_filtered;
    }

private:
    const Config &_config;
    T current_filtered{};
};

}// namespace kf::math::filters