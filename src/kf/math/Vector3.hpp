// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>

#include "kf/Option.hpp"
#include "kf/primitives.hpp"

namespace kf::math {

/// @brief 3D vector template for graphics and calculations
template<typename T> struct Vector3 final {

    using Scalar = T;///< Vector component type

    Scalar x, y, z;

    [[nodiscard]] static constexpr Vector3 zero() noexcept {
        return {
            .x = 0,
            .y = 0,
            .z = 0,
        };
    }

    /// @brief Vector addition
    /// @param other Vector to add
    /// @return Sum vector
    [[nodiscard]] constexpr Vector3 operator+(const Vector3 &other) const noexcept {
        return {
            .x = static_cast<Scalar>(x + other.x),
            .y = static_cast<Scalar>(y + other.y),
            .z = static_cast<Scalar>(z + other.z),
        };
    }

    /// @brief Vector subtraction
    /// @param other Vector to subtract
    /// @return Difference vector
    [[nodiscard]] constexpr Vector3 operator-(const Vector3 &other) const noexcept {
        return {
            .x = static_cast<Scalar>(x - other.x),
            .y = static_cast<Scalar>(y - other.y),
            .z = static_cast<Scalar>(z - other.z),
        };
    }

    /// @brief Scalar multiplication
    /// @param scalar Multiplication factor
    /// @return Scaled vector
    [[nodiscard]] constexpr Vector3 operator*(Scalar scalar) const noexcept {
        return {
            .x = static_cast<Scalar>(x * scalar),
            .y = static_cast<Scalar>(y * scalar),
            .z = static_cast<Scalar>(z * scalar),
        };
    }

    /// @brief Safe scalar division with zero-check
    /// @param scalar Division factor
    /// @return Option containing divided vector or empty if divisor is zero
    [[nodiscard]] Option<Vector3> divChecked(Scalar scalar) const noexcept {
        if (scalar == 0) { return none; }

        return some(Vector3{
            .x = static_cast<Scalar>(x / scalar),
            .y = static_cast<Scalar>(y / scalar),
            .z = static_cast<Scalar>(z / scalar),
        });
    }

    /// @brief Scalar division
    /// @param scalar Division factor
    /// @return Divided vector
    /// @warning No zero-check (use divChecked for safe division)
    [[nodiscard]] constexpr Vector3 operator/(Scalar scalar) const noexcept {
        return {
            .x = static_cast<Scalar>(x / scalar),
            .y = static_cast<Scalar>(y / scalar),
            .z = static_cast<Scalar>(z / scalar),
        };
    }

    /// @brief Vector addition assignment
    /// @param other Vector to add
    /// @return Reference to modified vector
    Vector3 &operator+=(const Vector3 &other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    /// @brief Vector subtraction assignment
    /// @param other Vector to subtract
    /// @return Reference to modified vector
    Vector3 &operator-=(const Vector3 &other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    /// @brief Calculate vector length (magnitude)
    /// @return Euclidean length
    [[nodiscard]] constexpr Scalar length() const noexcept {
        return static_cast<Scalar>(std::sqrt(x * x + y * y + z * z));
    }

    /// @brief Get normalized (unit) vector
    /// @return Option containing unit vector or empty if vector is zero-length
    [[nodiscard]] Option<Vector3> normalized() const noexcept {
        const auto len = length();

        if (len == 0) { return none; }

        return some(Vector3{
            .x = static_cast<Scalar>(x / len),
            .y = static_cast<Scalar>(y / len),
            .z = static_cast<Scalar>(z / len),
        });
    }

    /// @brief Calculate dot product with another vector
    /// @param other Second vector
    /// @return Dot product value
    [[nodiscard]] constexpr Scalar dot(const Vector3 &other) const noexcept {
        return static_cast<Scalar>(x * other.x + y * other.y + z * other.z);
    }

    /// @brief Calculate cross product with another vector
    /// @param other Second vector
    /// @return Cross product vector (perpendicular to both inputs)
    [[nodiscard]] constexpr Vector3 cross(const Vector3 &other) const noexcept {
        return {
            .x = static_cast<Scalar>(y * other.z - z * other.y),
            .y = static_cast<Scalar>(z * other.x - x * other.z),
            .z = static_cast<Scalar>(x * other.y - y * other.x),
        };
    }

    /// @brief Check if vector is zero (all components zero)
    /// @return true if all components are zero
    [[nodiscard]] constexpr bool isZero() const noexcept {
        return x == 0 and y == 0 and z == 0;
    }
};

using Vector3f = Vector3<f32>;///< Float precision 3D vector
using Vector3i = Vector3<i32>;///< Integer precision 3D vector

}// namespace kf::math