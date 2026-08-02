// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    Range.hpp

#pragma once

#include "kf/math.hpp"
#include "kf/mixin/Length.hpp"

namespace kf {

/// @brief   Closed interval [start, end] with clamping and length calculation.
/// @tparam T Numeric type (must support comparison and arithmetic)
/// @note The range is valid when `start <= end`
template<typename T> struct Range final : mixin::Length<Range<T>, T> {

    using Self = Range<T>;

    T start;///< Lower bound (inclusive)
    T end;  ///< Upper bound (inclusive)

    /// @brief Constrains a value to the interval [start, end]
    /// @param value The value to math::clamp
    [[nodiscard]] constexpr T clamped(T value) const noexcept {
        return math::clamp(value, start, end);
    }

private:
    KF_IMPL_LENGTH(Self, T);
    constexpr T lengthImpl() const noexcept {
        return end - start;
    }
};

}// namespace kf