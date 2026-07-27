// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>

#include "kf/TrivialOption.hpp"
#include "kf/concepts.hpp"
#include "kf/mixin/Length.hpp"
#include "kf/primitives.hpp"

namespace kf::math {

inline namespace buildin_constants {

constexpr f64 pi{M_PI};

constexpr f64 half_pi{M_PI_2};

constexpr f64 euler{M_E};

}// namespace buildin_constants

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
[[nodiscard]] constexpr auto abs(auto const &x) noexcept {
    return (x > 0) ? x : static_cast<decltype(x)>(-x);
}

/// @brief Min
[[nodiscard]] constexpr auto min(auto const &a, auto const &b) noexcept {
    return (a < b) ? a : b;
}

/// @brief Max
[[nodiscard]] constexpr auto max(auto const &a, auto const &b) noexcept {
    return (a > b) ? a : b;
}

/// @brief Constrain value between lower and upper bounds
[[nodiscard]] constexpr auto clamp(auto const &value, auto const &low, auto const &high) noexcept {
    return min(high, max(value, low));
}

/// @brief Linear interpolate value from input (low..high) to output (log..high)
[[nodiscard]] constexpr auto linearMap(
    auto const &value,
    auto const &in_low, auto const &in_high,
    auto const &out_low, auto const &out_high) noexcept {
    return (value - in_low) * (out_high - out_low) / (in_high - in_low) + out_low;
}

}// namespace custom_functions

inline namespace objects {

struct Vector2Tag {};

/// @brief 2D vector template for graphics and calculations
template<arithmetic T> struct Vector2 :

    Vector2Tag,
    mixin::Length<Vector2<T>, T>

{

    /// @brief Vector component type
    using Scalar = T;

    using Self = Vector2<Scalar>;

    Scalar x, y;

    /// @brief Create 2D vector with auto-deduced types
    [[nodiscard]] static constexpr Self create(auto x, auto y) noexcept {
        return Self{
            .x = static_cast<Scalar>(x),
            .y = static_cast<Scalar>(y),
        };
    }

    [[nodiscard]] static constexpr Self zero() noexcept {
        return Self::create(0, 0);
    }

    /// @brief Vector addition
    /// @param other Vector to add
    /// @return Sum vector
    [[nodiscard]] constexpr auto operator+(implements<Vector2Tag> auto const &other) const noexcept {
        return Self::create(x + other.x, y + other.y);
    }

    /// @brief Vector subtraction
    /// @param other Vector to subtract
    /// @return Difference vector
    [[nodiscard]] constexpr auto operator-(implements<Vector2Tag> auto const &other) const noexcept {
        return Self::create(x - other.x, y - other.y);
    }

    /// @brief Scalar multiplication
    /// @param scalar Multiplication factor
    /// @return Scaled vector
    [[nodiscard]] constexpr auto operator*(Scalar scalar) const noexcept {
        return Self::create(x * scalar, y * scalar);
    }

    /// @brief Safe scalar division with zero-check
    /// @param scalar Division factor
    /// @return TrivialOption containing divided vector or empty if divisor is zero
    [[nodiscard]] constexpr auto divChecked(Scalar scalar) const noexcept;

    /// @brief Scalar division
    /// @param scalar Division factor
    /// @return Divided vector
    /// @warning No zero-check (use divChecked for safe division)
    [[nodiscard]] constexpr auto operator/(Scalar scalar) const noexcept {
        return Self::create(x / scalar, y / scalar);
    }

    /// @brief Vector addition assignment
    /// @param other Vector to add
    /// @return Reference to modified vector
    Self &operator+=(implements<Vector2Tag> auto const &other) noexcept {
        x += other.x;
        y += other.y;
        return *this;
    }

    /// @brief Vector subtraction assignment
    /// @param other Vector to subtract
    /// @return Reference to modified vector
    Self &operator-=(implements<Vector2Tag> auto const &other) noexcept {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    /// @brief Get normalized (unit) vector
    /// @return TrivialOption containing unit vector or empty if vector is zero-length
    [[nodiscard]] constexpr auto normalized() const noexcept;

    /// @brief Calculate dot product with another vector
    /// @param other Second vector
    /// @return Dot product value
    [[nodiscard]] constexpr Scalar dot(implements<Vector2Tag> auto const &other) const noexcept {
        return static_cast<Scalar>(x * other.x + y * other.y);
    }

    /// @brief Check if vector is zero (both components zero)
    /// @return true if both components are zero
    [[nodiscard]] constexpr bool isZero() const noexcept {
        return x == 0 and y == 0;
    }

private:
    KF_IMPL_LENGTH(Self, Scalar);
    constexpr Scalar lengthImpl() const noexcept {
        return static_cast<Scalar>(math::hypot(x, y));
    }
};

template<arithmetic T> constexpr auto Vector2<T>::divChecked(Scalar scalar) const noexcept {
    return (scalar == 0) ? TrivialOption<Self>{none} : someTrivial((*this) / scalar);
}

template<arithmetic T> constexpr auto Vector2<T>::normalized() const noexcept {
    return divChecked(this->length());
}

using Vector2f = Vector2<f32>;///< Float precision 2D vector
using Vector2i = Vector2<i32>;///< Integer precision 2D vector

struct Vector3Tag {};

/// @brief 3D vector template for graphics and calculations
template<arithmetic T> struct Vector3 :

    Vector3Tag,
    mixin::Length<Vector3<T>, T>

{

    /// @brief Vector component type
    using Scalar = T;

    using Self = Vector3<Scalar>;

    Scalar x, y, z;

    /// @brief Create 3D vector with auto-deduced types
    [[nodiscard]] static constexpr Self create(auto x, auto y, auto z) noexcept {
        return Self{
            .x = static_cast<Scalar>(x),
            .y = static_cast<Scalar>(y),
            .z = static_cast<Scalar>(z),
        };
    }

    [[nodiscard]] static constexpr Self zero() noexcept {
        return Self::create(0, 0, 0);
    }

    /// @brief Vector addition
    /// @param other Vector to add
    /// @return Sum vector
    [[nodiscard]] constexpr Self operator+(implements<Vector3Tag> auto const &other) const noexcept {
        return Self::create(x + other.x, y + other.y, z + other.z);
    }

    /// @brief Vector subtraction
    /// @param other Vector to subtract
    /// @return Difference vector
    [[nodiscard]] constexpr Self operator-(implements<Vector3Tag> auto const &other) const noexcept {
        return Self::create(x - other.x, y - other.y, z - other.z);
    }

    /// @brief Scalar multiplication
    /// @param scalar Multiplication factor
    /// @return Scaled vector
    [[nodiscard]] constexpr Self operator*(Scalar scalar) const noexcept {
        return Self::create(x * scalar, y * scalar, z * scalar);
    }

    /// @brief Safe scalar division with zero-check
    /// @param scalar Division factor
    /// @return TrivialOption containing divided vector or empty if divisor is zero
    [[nodiscard]] constexpr auto divChecked(Scalar scalar) const noexcept;

    /// @brief Scalar division
    /// @param scalar Division factor
    /// @return Divided vector
    /// @warning No zero-check (use divChecked for safe division)
    [[nodiscard]] constexpr Self operator/(Scalar scalar) const noexcept {
        return Self::create(x / scalar, y / scalar, z / scalar);
    }

    /// @brief Vector addition assignment
    /// @param other Vector to add
    /// @return Reference to modified vector
    Self &operator+=(implements<Vector3Tag> auto const &other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    /// @brief Vector subtraction assignment
    /// @param other Vector to subtract
    /// @return Reference to modified vector
    Self &operator-=(implements<Vector3Tag> auto const &other) noexcept {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    /// @brief Get normalized (unit) vector
    /// @return TrivialOption containing unit vector or empty if vector is zero-length
    [[nodiscard]] constexpr auto normalized() const noexcept;

    /// @brief Calculate dot product with another vector
    /// @param other Second vector
    /// @return Dot product value
    [[nodiscard]] constexpr Scalar dot(implements<Vector3Tag> auto const &other) const noexcept {
        return static_cast<Scalar>(x * other.x + y * other.y + z * other.z);
    }

    /// @brief Calculate cross product with another vector
    /// @param other Second vector
    /// @return Cross product vector (perpendicular to both inputs)
    [[nodiscard]] constexpr Self cross(implements<Vector3Tag> auto const &other) const noexcept {
        return Self::create(
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
    KF_IMPL_LENGTH(Self, Scalar);
    constexpr Scalar lengthImpl() const noexcept {
        return static_cast<Scalar>(math::sqrt(x * x + y * y + z * z));
    }
};

template<arithmetic T> constexpr auto Vector3<T>::divChecked(Scalar scalar) const noexcept {
    return (scalar == 0) ? TrivialOption<Self>{none} : someTrivial((*this) / scalar);
}

template<arithmetic T> constexpr auto Vector3<T>::normalized() const noexcept {
    return divChecked(this->length());
}

using Vector3f = Vector3<f32>;///< Float precision 3D vector
using Vector3i = Vector3<i32>;///< Integer precision 3D vector

struct QuaternionTag {};

/// @brief Quaternion for 3D rotations (trivially copyable aggregate)
/// @tparam T Floating-point component type
template<arithmetic T> struct Quaternion :

    QuaternionTag,
    mixin::Length<Quaternion<T>, T>

{

    /// @brief Component type
    using Scalar = T;

    using Self = Quaternion<Scalar>;

    Scalar x, y, z, w;

    /// @brief Create Quaternion with auto-deduced types
    [[nodiscard]] static constexpr Self create(auto x, auto y, auto z, auto w) noexcept {
        return Self{
            .x = static_cast<Scalar>(x),
            .y = static_cast<Scalar>(y),
            .z = static_cast<Scalar>(z),
            .w = static_cast<Scalar>(w),
        };
    }

    /// @brief Identity quaternion (no rotation)
    [[nodiscard]] static constexpr Self identity() noexcept {
        return Self::create(0, 0, 0, 1);
    }

    /// @brief Create quaternion from axis‑angle representation
    /// @param axis Rotation axis (must be unit length)
    /// @param angle Rotation angle in radians
    /// @return Quaternion representing the rotation
    [[nodiscard]] static Self fromAxisAngle(implements<Vector3Tag> auto const &axis, Scalar angle) noexcept {
        auto const
            s = math::sin(angle * 0.5),
            c = math::cos(angle * 0.5);

        return Self::create(axis.x * s, axis.y * s, axis.z * s, c);
    }

    /// @brief Create quaternion from Euler angles (ZYX order) using any numeric vector type
    /// @param eulers Vector containing (roll, pitch, yaw) in radians
    /// @return Quaternion equivalent to the Euler rotation
    [[nodiscard]] static Self fromEulers(implements<Vector3Tag> auto const &eulers) noexcept {
        constexpr auto half = static_cast<Scalar>(0.5);

        auto const
            cr = math::cos(eulers.x * half),
            sr = math::sin(eulers.x * half),
            cp = math::cos(eulers.y * half),
            sp = math::sin(eulers.y * half),
            cy = math::cos(eulers.z * half),
            sy = math::sin(eulers.z * half);

        return Self::create(
            (sr * cp * cy - cr * sp * sy),
            (cr * sp * cy + sr * cp * sy),
            (cr * cp * sy - sr * sp * cy),
            (cr * cp * cy + sr * sp * sy));
    }

    /// @brief Conjugate of the quaternion
    [[nodiscard]] constexpr Self conjugate() const noexcept {
        return Self::create(-x, -y, -z, w);
    }

    /// @brief Convert quaternion to Euler angles (roll, pitch, yaw) in radians
    /// @return Vector3<T> containing (roll, pitch, yaw)
    /// @note Uses ZYX extrinsic rotation order (roll around X, pitch around Y, yaw around Z).
    ///       Handles gimbal lock (pitch = +-90 deg) by setting yaw = 0 and adjusting roll.
    template<typename U = T> [[nodiscard]] Vector3<U> toEulers() const noexcept {
        // sin(pitch) term (sarg)
        auto const sarg = -2 * (x * z - w * y) / lengthSquared();

        constexpr auto lim = T{0.99999};

        // North pole (pitch = +90 deg)
        if (sarg >= lim) {
            return Vector3<U>::create(0, half_pi, 2 * math::atan2(y, x));// roll, pitch, yaw
        }

        // South pole (pitch = -90 deg)
        if (sarg <= -lim) {
            return Vector3<U>::create(0, -half_pi, -2 * math::atan2(y, x));// roll, pitch, yaw
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
    constexpr void normalize() noexcept {
        auto n = this->length();

        if (n > 0) {
            x = static_cast<Scalar>(x / n);
            y = static_cast<Scalar>(y / n);
            z = static_cast<Scalar>(z / n);
            w = static_cast<Scalar>(w / n);
        }
    }

    /// @brief Get normalized (unit) quaternion
    /// @return TrivialOption containing unit quaternion, or empty if zero length
    [[nodiscard]] constexpr auto normalized() const noexcept;

    /// @brief Inverse of the quaternion
    /// @return TrivialOption containing inverse, or empty if zero length (non‑invertible)
    [[nodiscard]] constexpr auto inverse() const noexcept;

    /// @brief Rotate a 3D vector by this quaternion (assumes unit quaternion)
    /// @param v Vector to rotate
    /// @return Rotated vector
    [[nodiscard]] constexpr auto rotate(implements<Vector3Tag> auto const &v) const noexcept {
        auto const
            xx = x * x,
            yy = y * y,
            zz = z * z,
            xy = x * y,
            xz = x * z,
            xw = x * w,
            yz = y * z,
            yw = y * w,
            zw = z * w;

        return std::decay_t<decltype(v)>::create(
            ((1 - 2 * (yy + zz)) * v.x + 2 * (xy - zw) * v.y + 2 * (xz + yw) * v.z),
            (2 * (xy + zw) * v.x + (1 - 2 * (xx + zz)) * v.y + 2 * (yz - xw) * v.z),
            (2 * (xz - yw) * v.x + 2 * (yz + xw) * v.y + (1 - 2 * (xx + yy)) * v.z));
    }

    /// @brief Quaternion multiplication (composition of rotations)
    [[nodiscard]] constexpr Self operator*(implements<Self> auto const &other) const noexcept {
        return Self::create(
            (w * other.x + x * other.w + y * other.z - z * other.y),
            (w * other.y - x * other.z + y * other.w + z * other.x),
            (w * other.z + x * other.y - y * other.x + z * other.w),
            (w * other.w - x * other.x - y * other.y - z * other.z));
    }

    /// @brief Multiply‑assign
    Quaternion &operator*=(implements<Self> auto const &other) noexcept {
        *this = *this * other;
        return *this;
    }

private:
    KF_IMPL_LENGTH(Self, Scalar);
    constexpr Scalar lengthImpl() const noexcept {
        return static_cast<Scalar>(math::sqrt(lengthSquared()));
    }
};

template<arithmetic T> constexpr auto Quaternion<T>::normalized() const noexcept {
    auto const n = this->length();

    if (n == 0) { return TrivialOption<Self>{none}; }

    return someTrivial(Self::create(x / n, y / n, z / n, w / n));
}

template<arithmetic T> constexpr auto Quaternion<T>::inverse() const noexcept {
    auto const n2 = lengthSquared();

    if (n2 == 0) { return TrivialOption<Self>{none}; }

    auto const c = conjugate();

    return someTrivial(Self::create(c.x / n2, c.y / n2, c.z / n2, c.w / n2));
}

using Quaternionf = Quaternion<f32>;

}// namespace objects

}// namespace kf::math