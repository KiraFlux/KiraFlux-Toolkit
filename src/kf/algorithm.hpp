// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>


namespace kf {

using std::find;
using std::for_each;
using std::max;
using std::min;
using std::abs;

/// Constrain value between lower and upper bounds
template<typename T> constexpr T clamp(const T &value, const T &low, const T &high) noexcept {
    return (value < low) ? low : (value > high) ? high : value;
}

}