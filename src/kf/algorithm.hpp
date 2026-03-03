// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>

namespace kf {

using std::abs;
using std::find;
using std::for_each;
using std::max;
using std::min;

/// @brief Constrain value between lower and upper bounds
template<typename T> constexpr T clamp(const T &value, const T &low, const T &high) noexcept {
    return (value < low) ? low : ((value > high) ? high : value);
}

/// @brief Linear interpolate value from input (low..high) to output (log..high)
template<typename T> constexpr T linearMap(
    const T &value,
    const T &in_low, const T &in_high,
    const T &out_low, const T &out_high) noexcept {
    return (value - in_low) * (out_high - out_low) / (in_max - in_min);
}

}// namespace kf