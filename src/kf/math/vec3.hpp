// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>

#include "kf/Option.hpp"
#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"

namespace kf {

/// @brief 3D vector template for graphics and calculations
template<typename T> struct vec3 final {

    using Scalar = T;///< Vector component type

    T x;///< X component
    T y;///< Y component
    T z;///< Z component

    /// @brief Default constructor (initializes to zero)
    vec3() :
        x{0}, y{0}, z{0} {}

    /// @brief Construct from x, y, z components
    /// @param x X component value
    /// @param y Y component value
    /// @param z Z component value
    vec3(T x, T y, T z) :
        x{x}, y{y}, z{z} {}

    /// @brief Vector addition
    /// @param other Vector to add
    /// @return Sum vector
    kf_nodiscard vec3 operator+(const vec3 &other) const noexcept {
        return {x + other.x, y + other.y, z + other.z};
    }

    /// @brief Vector subtraction
    /// @param other Vector to subtract
    /// @return Difference vector
    kf_nodiscard vec3 operator-(const vec3 &other) const noexcept {
        return {x - other.x, y - other.y, z - other.z};
    }

    /// @brief Scalar multiplication
    /// @param scalar Multiplication factor
    /// @return Scaled vector
    kf_nodiscard vec3 operator*(T scalar) const noexcept {
        return {x * scalar, y * scalar, z * scalar};
    }

    /// @brief Safe scalar division with zero-check
    /// @param scalar Division factor
    /// @return Option containing divided vector or empty if divisor is zero
    kf_nodiscard Option<vec3> divChecked(T scalar) const noexcept {
        if (scalar == 0) {
            return {};
        }

        return {vec3{x / scalar, y / scalar, z / scalar}};
    }

    /// @brief Scalar division
    /// @param scalar Division factor
    /// @return Divided vector
    /// @warning No zero-check (use divChecked for safe division)
    kf_nodiscard vec3 operator/(T scalar) const noexcept {
        return vec3{x / scalar, y / scalar, z / scalar};
    }

    /// @brief Vector addition assignment
    /// @param other Vector to add
    /// @return Reference to modified vector
    vec3 &operator+=(const vec3 &other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    /// @brief Vector subtraction assignment
    /// @param other Vector to subtract
    /// @return Reference to modified vector
    vec3 &operator-=(const vec3 &other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    /// @brief Calculate vector length (magnitude)
    /// @return Euclidean length
    kf_nodiscard T length() const noexcept {
        return std::sqrt(x * x + y * y + z * z);
    }

    /// @brief Get normalized (unit) vector
    /// @return Option containing unit vector or empty if vector is zero-length
    kf_nodiscard Option<vec3> normalized() const noexcept {
        const T len = length();

        if (len == 0) {
            return {};
        }

        return {vec3{x / len, y / len, z / len}};
    }

    /// @brief Calculate dot product with another vector
    /// @param other Second vector
    /// @return Dot product value
    kf_nodiscard T dot(const vec3 &other) const noexcept {
        return x * other.x + y * other.y + z * other.z;
    }

    /// @brief Calculate cross product with another vector
    /// @param other Second vector
    /// @return Cross product vector (perpendicular to both inputs)
    kf_nodiscard vec3 cross(const vec3 &other) const noexcept {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x};
    }

    /// @brief Check if vector is zero (all components zero)
    /// @return true if all components are zero
    kf_nodiscard inline bool isZero() const noexcept {
        return x == 0 and y == 0 and z == 0;
    }
};

using vec3f = vec3<f32>;///< Float precision 3D vector
using vec3i = vec3<i32>;///< Integer precision 3D vector

}// namespace kf