// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/validation.hpp"

namespace kf {

/// @brief Closed interval [start, end] with validation and clamping.
/// @tparam T Numeric type (must support comparison and arithmetic).
/// @note The range is valid when `start <= end`. Invalid ranges fail validation.
template<typename T> struct Range final : Validatable<Range<T>> {

    T start;///< Lower bound (inclusive).
    T end;  ///< Upper bound (inclusive).

    /// @brief Constrains a value to the interval [start, end].
    /// @param value The value to clamp.
    [[nodiscard]] constexpr T clamped(T value) const noexcept {
        return clamp(value, start, end);
    }

private:
    static constexpr auto logger{Logger::create("Range")};

    KF_IMPL_VALIDATABLE(Validatable<Range<T>>);
    void checkImpl(Validator &validator) const noexcept {
        KF_VALIDATOR_CHECK(validator, logger, start <= end);
    }
};

}// namespace kf