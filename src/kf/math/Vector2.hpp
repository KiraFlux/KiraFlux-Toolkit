// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>

#include "kf/Option.hpp"
#include "kf/primitives.hpp"

namespace kf::math {

/// @brief 2D vector template for graphics and calculations
/// @tparam T Component type (float, integer, etc.)
template<typename T> struct Vector2 final {

    using Scalar = T;///< Vector component type

    Scalar x, y;

    [[nodiscard]] static constexpr Vector2 zero() noexcept {
        return {
            .x = 0,
            .y = 0,
        };
    }

    /// @brief Vector addition
    /// @param other Vector to add
    /// @return Sum vector
    [[nodiscard]] constexpr Vector2 operator+(const Vector2 &other) const noexcept {
        return {
            .x = static_cast<Scalar>(x + other.x),
            .y = static_cast<Scalar>(y + other.y),
        };
    }

    /// @brief Vector subtraction
    /// @param other Vector to subtract
    /// @return Difference vector
    [[nodiscard]] constexpr Vector2 operator-(const Vector2 &other) const noexcept {
        return {
            .x = static_cast<Scalar>(x - other.x),
            .y = static_cast<Scalar>(y - other.y),
        };
    }

    /// @brief Scalar multiplication
    /// @param scalar Multiplication factor
    /// @return Scaled vector
    [[nodiscard]] constexpr Vector2 operator*(Scalar scalar) const noexcept {
        return {
            .x = static_cast<Scalar>(x * scalar),
            .y = static_cast<Scalar>(y * scalar),
        };
    }

    /// @brief Safe scalar division with zero-check
    /// @param scalar Division factor
    /// @return Option containing divided vector or empty if divisor is zero
    [[nodiscard]] TrivialOption<Vector2> divChecked(Scalar scalar) const noexcept {
        if (scalar == 0) { return none; }

        return someTrivial(Vector2{
            .x = static_cast<Scalar>(x / scalar),
            .y = static_cast<Scalar>(y / scalar),
        });
    }

    /// @brief Scalar division
    /// @param scalar Division factor
    /// @return Divided vector
    /// @warning No zero-check (use divChecked for safe division)
    [[nodiscard]] constexpr Vector2 operator/(Scalar scalar) const noexcept {
        return {
            .x = static_cast<Scalar>(x / scalar),
            .y = static_cast<Scalar>(y / scalar),
        };
    }

    /// @brief Vector addition assignment
    /// @param other Vector to add
    /// @return Reference to modified vector
    Vector2 &operator+=(const Vector2 &other) noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }

    /// @brief Vector subtraction assignment
    /// @param other Vector to subtract
    /// @return Reference to modified vector
    Vector2 &operator-=(const Vector2 &other) noexcept {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    /// @brief Calculate vector length (magnitude)
    /// @return Euclidean length
    [[nodiscard]] constexpr Scalar length() const noexcept {
        return static_cast<Scalar>(std::hypot(x, y));
    }

    /// @brief Get normalized (unit) vector
    /// @return Option containing unit vector or empty if vector is zero-length
    [[nodiscard]] TrivialOption<Vector2> normalized() const noexcept {
        const auto len = length();

        if (len == 0) { return none; }

        return someTrivial(Vector2{
            .x = static_cast<Scalar>(x / len),
            .y = static_cast<Scalar>(y / len),
        });
    }

    /// @brief Calculate dot product with another vector
    /// @param other Second vector
    /// @return Dot product value
    [[nodiscard]] constexpr Scalar dot(const Vector2 &other) const noexcept {
        return static_cast<Scalar>(x * other.x + y * other.y);
    }

    /// @brief Check if vector is zero (both components zero)
    /// @return true if both components are zero
    [[nodiscard]] constexpr bool isZero() const noexcept {
        return x == 0 and y == 0;
    }
};

using Vector2f = Vector2<f32>;///< Float precision 2D vector
using Vector2i = Vector2<i32>;///< Integer precision 2D vector

}// namespace kf::math