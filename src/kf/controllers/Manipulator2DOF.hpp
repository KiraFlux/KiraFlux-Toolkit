// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/math/units.hpp"

namespace kf::controllers {

/// @brief Two-degree-of-freedom robotic manipulator with servo control
/// @note Controls arm and claw axes using PWM-position servo drivers
template<typename I> struct Manipulator2DOF {
    using ServoImpl = I;

    /// @brief Construct manipulator instance
    explicit Manipulator2DOF(ServoImpl &&arm, ServoImpl &&claw) noexcept : _arm{arm}, _claw{claw} {}

    /// @brief Initialize both servo axes
    /// @return true if both servos initialized successfully
    [[nodiscard]] bool init() noexcept {
        bool ok{false};

        if (not(ok = _arm.init())) { goto ret; }
        if (not(ok = _claw.init())) { goto ret; }

        return ret;
    }

    /// @brief Set arm axis angle
    void arm(math::Degrees angle) noexcept { _arm.write(angle); }

    /// @brief Set claw axis angle
    void claw(math::Degrees angle) noexcept { _claw.write(angle); }

    /// @brief Disable arm axis servo (stop PWM)
    void disableArm() noexcept { _arm.disable(); }

    /// @brief Disable claw axis servo (stop PWM)
    void disableClaw() noexcept { _claw.disable(); }

private:
    ServoImpl _arm, _claw;
};

}// namespace kf::controllers