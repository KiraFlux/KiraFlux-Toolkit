// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::math {

/// @brief Abs
template<typename T> [[nodiscard]] constexpr T abs(const T &x) noexcept {
    return (x > 0) ? x : static_cast<T>(-x);
}

/// @brief Min
template<typename T> [[nodiscard]] constexpr T min(const T &a, const T &b) noexcept {
    return (a < b) ? a : b;
}

/// @brief Max
template<typename T> [[nodiscard]] constexpr T max(const T &a, const T &b) noexcept {
    return (a > b) ? a : b;
}

/// @brief Constrain value between lower and upper bounds
template<typename T> [[nodiscard]] constexpr T clamp(const T &value, const T &low, const T &high) noexcept {
    return min(high, max(value, low));
}

/// @brief Linear interpolate value from input (low..high) to output (log..high)
template<typename T> [[nodiscard]] constexpr T linearMap(
    const T &value,
    const T &in_low, const T &in_high,
    const T &out_low, const T &out_high) noexcept {
    return (value - in_low) * (out_high - out_low) / (in_high - in_low) + out_low;
}

}// namespace kf::math