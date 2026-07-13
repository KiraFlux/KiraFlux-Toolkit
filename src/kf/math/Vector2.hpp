// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Option.hpp"
#include "kf/math.hpp"
#include "kf/mixin/Length.hpp"
#include "kf/primitives.hpp"

namespace kf::math {

/// @brief 2D vector template for graphics and calculations
/// @tparam T Component type (float, integer, etc.)
template<typename T> struct Vector2 final : mixin::Length<Vector2<T>, T> {

    /// @brief Vector component type
    using Scalar = T;

    Scalar x, y;

    /// @brief Create 2D vector with auto-deduced types
    /// @tparam X x component type (likely auto deduced)
    /// @tparam Y y component type (likely auto deduced)
    template<typename X, typename Y> [[nodiscard]] static constexpr Vector2 create(X x, Y y) noexcept {
        return {
            .x = static_cast<Scalar>(x),
            .y = static_cast<Scalar>(y),
        };
    }

    [[nodiscard]] static constexpr Vector2 zero() noexcept {
        return create(0, 0);
    }

    /// @brief Vector addition
    /// @param other Vector to add
    /// @return Sum vector
    [[nodiscard]] constexpr Vector2 operator+(const Vector2 &other) const noexcept {
        return create(x + other.x, y + other.y);
    }

    /// @brief Vector subtraction
    /// @param other Vector to subtract
    /// @return Difference vector
    [[nodiscard]] constexpr Vector2 operator-(const Vector2 &other) const noexcept {
        return create(x - other.x, y - other.y);
    }

    /// @brief Scalar multiplication
    /// @param scalar Multiplication factor
    /// @return Scaled vector
    [[nodiscard]] constexpr Vector2 operator*(Scalar scalar) const noexcept {
        return create(x * scalar, y * scalar);
    }

    /// @brief Safe scalar division with zero-check
    /// @param scalar Division factor
    /// @return Option containing divided vector or empty if divisor is zero
    [[nodiscard]] constexpr auto divChecked(Scalar scalar) const noexcept -> TrivialOption<Vector2> {
        return (scalar == 0) ? none : someTrivial((*this) / scalar);
    }

    /// @brief Scalar division
    /// @param scalar Division factor
    /// @return Divided vector
    /// @warning No zero-check (use divChecked for safe division)
    [[nodiscard]] constexpr Vector2 operator/(Scalar scalar) const noexcept {
        return create(x / scalar, y / scalar);
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

    /// @brief Get normalized (unit) vector
    /// @return Option containing unit vector or empty if vector is zero-length
    [[nodiscard]] auto normalized() const noexcept -> TrivialOption<Vector2> {
        return divChecked(this->length());
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

private:
    using This = Vector2<Scalar>;

    KF_IMPL_LENGTH(This, Scalar);
    constexpr Scalar lengthImpl() const noexcept {
        return static_cast<Scalar>(math::hypot(x, y));
    }
};

using Vector2f = Vector2<f32>;///< Float precision 2D vector
using Vector2i = Vector2<i32>;///< Integer precision 2D vector

}// namespace kf::math