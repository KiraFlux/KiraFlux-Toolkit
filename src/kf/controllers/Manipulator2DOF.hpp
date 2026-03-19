// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/math/units.hpp"

namespace kf::controllers {

/// @brief Two-degree-of-freedom robotic manipulator with servo control
/// @note Controls arm and claw axes using PWM-position servo drivers
template<typename I> struct Manipulator2DOF final : mixin::Initable<Manipulator2DOF, bool> {
    using ActuatorImpl = I;

    /// @brief Construct manipulator instance
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

    KF_IMPL_INITABLE(Manipulator2DOF, bool);
    bool initImpl() noexcept {
        if (not _arm.init()) { return false; }
        if (not _claw.init()) { return false; }
        return true;
    }
};

}// namespace kf::controllers