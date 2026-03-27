// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/math/units.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf::controllers {

/// @brief Two-degree-of-freedom robotic manipulator
/// @tparam I Actuator implementation type (must satisfy Actuator interface).
/// @note Controls arm and claw axes servo drivers
template<typename I> struct Manipulator2DOF final : mixin::NonCopyable, mixin::Initable<Manipulator2DOF, bool> {
    using ActuatorImpl = I;

    /// @brief Construct manipulator instance.
    /// @param arm  Actuator for the arm axis (will be moved).
    /// @param claw Actuator for the claw axis (will be moved).
    explicit Manipulator2DOF(ActuatorImpl &&arm, ActuatorImpl &&claw) noexcept : _arm{arm}, _claw{claw} {}

    /// @brief Set arm axis angle
    void arm(math::Degrees angle) noexcept { _arm.write(angle); }

    /// @brief Set claw axis angle
    void claw(math::Degrees angle) noexcept { _claw.write(angle); }

    /// @brief Disable arm axis servo (stop PWM)
    void disableArm() noexcept { _arm.disable(); }

    /// @brief Disable claw axis servo (stop PWM)
    void disableClaw() noexcept { _claw.disable(); }

private:
    ActuatorImpl _arm, _claw;

    // impl
    using This = Manipulator2DOF;

    KF_IMPL_INITABLE(This, bool);
    bool initImpl() noexcept {
        if (not _arm.init()) { return false; }
        if (not _claw.init()) { return false; }
        return true;
    }
};

}// namespace kf::controllers