// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>

#include "kf/primitives.hpp"

namespace kf::math {

// functions from builtins

using std::isinf;
using std::isnan;

using std::hypot;
using std::sqrt;

using std::asin;
using std::sin;

using std::acos;
using std::cos;

using std::atan;
using std::atan2;
using std::tan;

// custom functions

/// @brief Get corresponding NaN value
template<typename T> [[nodiscard]] constexpr T nan() noexcept {
    return std::numeric_limits<T>::quiet_NaN();
}

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

// units

/// @brief Angle in degrees
using Degrees = u16;

/// @brief Pixel position coordinate
using Pixels = i16;

/// @brief Physical distance in millimeters
using Millimeters = f64;

/// @brief Time duration in seconds
using Seconds = f32;

/// @brief Frequency in Hertz (Hz = 1/s)
using Hertz = u16;

/// @brief Time duration in milliseconds
using Milliseconds = u32;

/// @brief Time duration in microseconds
using Microseconds = u32;

}// namespace kf::math