// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"

namespace kf {

/// @brief Closed interval [start, end]
/// @tparam T Numeric type (must support comparison and arithmetic)
/// @note The range is valid when `start <= end`
template<typename T> struct Range final {

    T start;///< Lower bound (inclusive)
    T end;  ///< Upper bound (inclusive)

    /// @brief Get interval length
    [[nodiscard]] constexpr T length() const noexcept {
        return end - start;
    }

    /// @brief Constrains a value to the interval [start, end]
    /// @param value The value to clamp
    [[nodiscard]] constexpr T clamped(T value) const noexcept {
        return clamp(value, start, end);
    }
};

}// namespace kf