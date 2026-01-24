// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>


namespace kf {

using std::find;    ///< Find element in range (std implementation)
using std::for_each;///< Apply function to each element in range (std implementation)
using std::max;     ///< Return larger of two values (std implementation)
using std::min;     ///< Return smaller of two values (std implementation)

/// Constrain value between lower and upper bounds
template<typename T> constexpr T clamp(const T &value, const T &low, const T &high) {
    return (value < low) ? low : (value > high) ? high : value;
}

}