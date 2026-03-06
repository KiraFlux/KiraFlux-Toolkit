// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>

#include "kf/Option.hpp"
#include "kf/aliases.hpp"

namespace kf::math {

/// @brief 2D vector template for graphics and calculations
/// @tparam T Component type (float, integer, etc.)
template<typename T> struct Vector final {

    using Scalar = T;///< Vector component type

    T x, y;

    /// @brief Default constructor (initializes to zero)
    Vector() noexcept :
        x{0}, y{0} {}

    /// @brief Construct from x and y components
    Vector(T x, T y) noexcept :
        x{x}, y{y} {}

    /// @brief Vector addition
    /// @param other Vector to add
    /// @return Sum vector
    [[nodiscard]] Vector operator+(const Vector &other) const noexcept {
        return {x + other.x, y + other.y};
    }

    /// @brief Vector subtraction
    /// @param other Vector to subtract
    /// @return Difference vector
    [[nodiscard]] Vector operator-(const Vector &other) const noexcept {
        return {x - other.x, y - other.y};
    }

    /// @brief Scalar multiplication
    /// @param scalar Multiplication factor
    /// @return Scaled vector
    [[nodiscard]] Vector operator*(T scalar) const noexcept {
        return {x * scalar, y * scalar};
    }

    /// @brief Safe scalar division with zero-check
    /// @param scalar Division factor
    /// @return Option containing divided vector or empty if divisor is zero
    [[nodiscard]] Option<Vector> divChecked(T scalar) const noexcept {
        if (scalar == 0) {
            return {};
        }

        return {Vector{x / scalar, y / scalar}};
    }

    /// @brief Scalar division
    /// @param scalar Division factor
    /// @return Divided vector
    /// @warning No zero-check (use divChecked for safe division)
    [[nodiscard]] Vector operator/(T scalar) const noexcept {
        return Vector{x / scalar, y / scalar};
    }

    /// @brief Vector addition assignment
    /// @param other Vector to add
    /// @return Reference to modified vector
    Vector &operator+=(const Vector &other) noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }

    /// @brief Vector subtraction assignment
    /// @param other Vector to subtract
    /// @return Reference to modified vector
    Vector &operator-=(const Vector &other) noexcept {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    /// @brief Calculate vector length (magnitude)
    /// @return Euclidean length
    [[nodiscard]] T length() const noexcept {
        return std::hypot(x, y);
    }

    /// @brief Get normalized (unit) vector
    /// @return Option containing unit vector or empty if vector is zero-length
    [[nodiscard]] Option<Vector> normalized() const noexcept {
        const T len = length();

        if (len == 0) {
            return {};
        }

        return {Vector{x / len, y / len}};
    }

    /// @brief Calculate dot product with another vector
    /// @param other Second vector
    /// @return Dot product value
    [[nodiscard]] T dot(const Vector &other) const noexcept {
        return x * other.x + y * other.y;
    }

    /// @brief Check if vector is zero (both components zero)
    /// @return true if both components are zero
    [[nodiscard]] inline bool isZero() const noexcept {
        return x == 0 and y == 0;
    }
};

using Vector2f = Vector<f32>;///< Float precision 2D vector
using Vector2i = Vector<i32>;///< Integer precision 2D vector

}// namespace kf::math