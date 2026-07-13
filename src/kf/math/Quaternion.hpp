// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/math.hpp"

#include "kf/Option.hpp"
#include "kf/math/Vector3.hpp"
#include "kf/mixin/Length.hpp"
#include "kf/primitives.hpp"

namespace kf::math {

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

}// namespace kf::math