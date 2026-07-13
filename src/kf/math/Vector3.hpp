// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>

#include "kf/Option.hpp"
#include "kf/mixin/Length.hpp"
#include "kf/primitives.hpp"

namespace kf::math {

/// @brief 3D vector template for graphics and calculations
template<typename T> struct Vector3 final : mixin::Length<Vector3<T>, T> {

    /// @brief Vector component type
    using Scalar = T;

    Scalar x, y, z;

    /// @brief Create 3D vector with auto-deduced types
    /// @tparam X x component type (likely auto deduced)
    /// @tparam Y y component type (likely auto deduced)
    /// @tparam Z z component type (likely auto deduced)
    template<typename X, typename Y, typename Z> [[nodiscard]] static constexpr Vector3 create(X x, Y y, Z z) noexcept {
        return {
            .x = static_cast<Scalar>(x),
            .y = static_cast<Scalar>(y),
            .z = static_cast<Scalar>(z),
        };
    }

    [[nodiscard]] static constexpr Vector3 zero() noexcept {
        return create(0, 0, 0);
    }

    /// @brief Vector addition
    /// @param other Vector to add
    /// @return Sum vector
    [[nodiscard]] constexpr Vector3 operator+(const Vector3 &other) const noexcept {
        return create(x + other.x, y + other.y, z + other.z);
    }

    /// @brief Vector subtraction
    /// @param other Vector to subtract
    /// @return Difference vector
    [[nodiscard]] constexpr Vector3 operator-(const Vector3 &other) const noexcept {
        return create(x - other.x, y - other.y, z - other.z);
    }

    /// @brief Scalar multiplication
    /// @param scalar Multiplication factor
    /// @return Scaled vector
    [[nodiscard]] constexpr Vector3 operator*(Scalar scalar) const noexcept {
        return create(x * scalar, y * scalar, z * scalar);
    }

    /// @brief Safe scalar division with zero-check
    /// @param scalar Division factor
    /// @return Option containing divided vector or empty if divisor is zero
    [[nodiscard]] constexpr auto divChecked(Scalar scalar) const noexcept -> TrivialOption<Vector3> {
        return (scalar == 0) ? none : someTrivial((*this) / scalar);
    }

    /// @brief Scalar division
    /// @param scalar Division factor
    /// @return Divided vector
    /// @warning No zero-check (use divChecked for safe division)
    [[nodiscard]] constexpr Vector3 operator/(Scalar scalar) const noexcept {
        return create(x / scalar, y / scalar, z / scalar);
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

    /// @brief Get normalized (unit) vector
    /// @return Option containing unit vector or empty if vector is zero-length
    [[nodiscard]] auto normalized() const noexcept -> TrivialOption<Vector3> {
        return divChecked(this->length());
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
        return create(
            (y * other.z - z * other.y),
            (z * other.x - x * other.z),
            (x * other.y - y * other.x));
    }

    /// @brief Check if vector is zero (all components zero)
    /// @return true if all components are zero
    [[nodiscard]] constexpr bool isZero() const noexcept {
        return x == 0 and y == 0 and z == 0;
    }

private:
    using This = Vector3<Scalar>;

    KF_IMPL_LENGTH(This, Scalar);
    constexpr Scalar lengthImpl() const noexcept {
        return static_cast<Scalar>(std::sqrt(x * x + y * y + z * z));
    }
};

using Vector3f = Vector3<f32>;///< Float precision 3D vector
using Vector3i = Vector3<i32>;///< Integer precision 3D vector

}// namespace kf::math