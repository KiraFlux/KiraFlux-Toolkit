// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>

#include "kf/TrivialOption.hpp"
#include "kf/mixin/Length.hpp"
#include "kf/primitives.hpp"

namespace kf::math {

inline namespace builtin_functions {

using std::isinf;
using std::isnan;

using std::hypot;
using std::sqrt;

using std::asin;
using std::sin;

using std::acos;
using std::cos;

using std::atan;
using std::atan2;
using std::tan;

}// namespace builtin_functions

inline namespace custom_functions {

/// @brief Abs
template<typename T> [[nodiscard]] constexpr T abs(const T &x) noexcept {
    return (x > 0) ? x : static_cast<T>(-x);
}

/// @brief Min
template<typename T> [[nodiscard]] constexpr T min(const T &a, const T &b) noexcept {
    return (a < b) ? a : b;
}

/// @brief Max
template<typename T> [[nodiscard]] constexpr T max(const T &a, const T &b) noexcept {
    return (a > b) ? a : b;
}

/// @brief Constrain value between lower and upper bounds
template<typename T> [[nodiscard]] constexpr T clamp(const T &value, const T &low, const T &high) noexcept {
    return min(high, max(value, low));
}

/// @brief Linear interpolate value from input (low..high) to output (log..high)
template<typename T> [[nodiscard]] constexpr T linearMap(
    const T &value,
    const T &in_low, const T &in_high,
    const T &out_low, const T &out_high) noexcept {
    return (value - in_low) * (out_high - out_low) / (in_high - in_low) + out_low;
}

}// namespace custom_functions

inline namespace objects {

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
        return static_cast<Scalar>(math::sqrt(x * x + y * y + z * z));
    }
};

using Vector3f = Vector3<f32>;///< Float precision 3D vector
using Vector3i = Vector3<i32>;///< Integer precision 3D vector

/// @brief Quaternion for 3D rotations (trivially copyable aggregate)
/// @tparam T Floating-point component type
template<typename T> struct Quaternion final : mixin::Length<Quaternion<T>, T> {

    /// @brief Component type
    using Scalar = T;

    Scalar x, y, z, w;

    /// @brief Create Quaternion with auto-deduced types
    /// @tparam X x component type (likely auto deduced)
    /// @tparam Y y component type (likely auto deduced)
    /// @tparam Z z component type (likely auto deduced)
    /// @tparam W w component type (likely auto deduced)
    template<typename X, typename Y, typename Z, typename W> [[nodiscard]] static constexpr Quaternion create(X x, Y y, Z z, W w) noexcept {
        return Quaternion{
            .x = static_cast<Scalar>(x),
            .y = static_cast<Scalar>(y),
            .z = static_cast<Scalar>(z),
            .w = static_cast<Scalar>(w),
        };
    }

    /// @brief Identity quaternion (no rotation)
    [[nodiscard]] static constexpr Quaternion identity() noexcept {
        return create(0, 0, 0, 1);
    }

    /// @brief Create quaternion from axis‑angle representation
    /// @param axis Rotation axis (must be unit length)
    /// @param angle Rotation angle in radians
    /// @return Quaternion representing the rotation
    [[nodiscard]] static Quaternion fromAxisAngle(const Vector3<Scalar> &axis, Scalar angle) noexcept {
        const auto
            s = math::sin(angle * 0.5),
            c = math::cos(angle * 0.5);

        return create(axis.x * s, axis.y * s, axis.z * s, c);
    }

    /// @brief Create quaternion from Euler angles (ZYX order) using any numeric vector type
    /// @tparam U Arithmetic type of input vector components
    /// @param eulers Vector containing (roll, pitch, yaw) in radians
    /// @return Quaternion equivalent to the Euler rotation
    template<typename U> [[nodiscard]] static Quaternion fromEulers(const Vector3<U> &eulers) noexcept {
        constexpr auto half = static_cast<Scalar>(0.5);

        const auto
            cr = math::cos(eulers.x * half),
            sr = math::sin(eulers.x * half),
            cp = math::cos(eulers.y * half),
            sp = math::sin(eulers.y * half),
            cy = math::cos(eulers.z * half),
            sy = math::sin(eulers.z * half);

        return create(
            (sr * cp * cy - cr * sp * sy),
            (cr * sp * cy + sr * cp * sy),
            (cr * cp * sy - sr * sp * cy),
            (cr * cp * cy + sr * sp * sy));
    }

    /// @brief Conjugate of the quaternion
    [[nodiscard]] constexpr Quaternion conjugate() const noexcept {
        return create(-x, -y, -z, w);
    }

    /// @brief Convert quaternion to Euler angles (roll, pitch, yaw) in radians
    /// @return Vector3<T> containing (roll, pitch, yaw)
    /// @note Uses ZYX extrinsic rotation order (roll around X, pitch around Y, yaw around Z).
    ///       Handles gimbal lock (pitch = +-90 deg) by setting yaw = 0 and adjusting roll.
    template<typename U = T> [[nodiscard]] Vector3<U> toEulers() const noexcept {
        // sin(pitch) term (sarg)
        const auto sarg = -2 * (x * z - w * y) / lengthSquared();

        constexpr auto lim = T{0.99999};

        // North pole (pitch = +90 deg)
        if (sarg >= lim) {
            return Vector3<U>::create(0, M_PI_2, 2 * math::atan2(y, x));// roll, pitch, yaw
        }

        // South pole (pitch = -90 deg)
        if (sarg <= -lim) {
            return Vector3<U>::create(0, -M_PI_2, -2 * math::atan2(y, x));// roll, pitch, yaw
        }

        // Regular case
        return Vector3<U>::create(
            (math::atan2(2 * (w * x + y * z), 1 - 2 * (x * x + y * y))),// roll
            (math::asin(sarg)),                                         // pitch
            (math::atan2(2 * (x * y + w * z), 1 - 2 * (y * y + z * z))) // yaw
        );
    }

    /// @brief Squared length (magnitude squared)
    [[nodiscard]] constexpr Scalar lengthSquared() const noexcept {
        return static_cast<Scalar>(x * x + y * y + z * z + w * w);
    }

    /// @brief Normalize the quaternion in‑place (does nothing if zero)
    void normalize() noexcept {
        auto n = this->length();

        if (n > 0) {
            x = static_cast<Scalar>(x / n);
            y = static_cast<Scalar>(y / n);
            z = static_cast<Scalar>(z / n);
            w = static_cast<Scalar>(w / n);
        }
    }

    /// @brief Get normalized (unit) quaternion
    /// @return Option containing unit quaternion, or empty if zero length
    [[nodiscard]] auto normalized() const noexcept -> TrivialOption<Quaternion> {
        auto n = this->length();

        if (n == 0) { return none; }

        return someTrivial(create(x / n, y / n, z / n, w / n));
    }

    /// @brief Inverse of the quaternion
    /// @return Option containing inverse, or empty if zero length (non‑invertible)
    [[nodiscard]] auto inverse() const noexcept -> TrivialOption<Quaternion> {
        const auto n2 = lengthSquared();

        if (n2 == 0) { return none; }

        const auto c = conjugate();

        return someTrivial(create(c.x / n2, c.y / n2, c.z / n2, c.w / n2));
    }

    /// @brief Rotate a 3D vector by this quaternion (assumes unit quaternion)
    /// @tparam U Arithmetic type of input vector components
    /// @param v Vector to rotate
    /// @return Rotated vector (component type Scalar)
    template<typename U> [[nodiscard]] auto rotate(const Vector3<U> &v) const noexcept -> Vector3<Scalar> {
        const auto
            vx = static_cast<Scalar>(v.x),
            vy = static_cast<Scalar>(v.y),
            vz = static_cast<Scalar>(v.z);

        const auto
            xx = x * x,
            yy = y * y,
            zz = z * z,
            xy = x * y,
            xz = x * z,
            xw = x * w,
            yz = y * z,
            yw = y * w,
            zw = z * w;

        return Vector3<Scalar>::create(
            ((1 - 2 * (yy + zz)) * vx + 2 * (xy - zw) * vy + 2 * (xz + yw) * vz),
            (2 * (xy + zw) * vx + (1 - 2 * (xx + zz)) * vy + 2 * (yz - xw) * vz),
            (2 * (xz - yw) * vx + 2 * (yz + xw) * vy + (1 - 2 * (xx + yy)) * vz));
    }

    /// @brief Quaternion multiplication (composition of rotations)
    [[nodiscard]] constexpr Quaternion operator*(const Quaternion &other) const noexcept {
        return Quaternion::create(
            (w * other.x + x * other.w + y * other.z - z * other.y),
            (w * other.y - x * other.z + y * other.w + z * other.x),
            (w * other.z + x * other.y - y * other.x + z * other.w),
            (w * other.w - x * other.x - y * other.y - z * other.z));
    }

    /// @brief Multiply‑assign
    Quaternion &operator*=(const Quaternion &other) noexcept {
        *this = *this * other;
        return *this;
    }

private:
    using This = Quaternion<Scalar>;

    KF_IMPL_LENGTH(This, Scalar);
    constexpr Scalar lengthImpl() const noexcept {
        return static_cast<Scalar>(math::sqrt(lengthSquared()));
    }
};

}// namespace objects

inline namespace unit_type_aliases {

/// @brief Angle in degrees
using Degrees = u16;

/// @brief Pixel position coordinate
using Pixels = i16;

/// @brief Physical distance in millimeters
using Millimeters = f64;

/// @brief Time duration in seconds
using Seconds = f32;

/// @brief Frequency in Hertz (Hz = 1/s)
using Hertz = u16;

/// @brief Time duration in milliseconds
using Milliseconds = u32;

/// @brief Time duration in microseconds
using Microseconds = u32;

}// namespace unit_type_aliases

}// namespace kf::math