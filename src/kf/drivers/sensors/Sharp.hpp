// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/gpio/GPIO.hpp"
#include "kf/math/units.hpp"
#include "kf/meta/type_check.hpp"
#include "kf/mixin/Initable.hpp"

namespace kf::drivers::sensors {

/// @brief Sharp infrared distance sensor driver
/// @note Provides distance measurements in millimeters using analog voltage output
template<typename I> struct Sharp : kf::mixin::Initable<Sharp<I>, void> {
    kf_crtp_check(I, kf::gpio::AdcInputTag);
    using PinImpl = I;

    explicit Sharp(PinImpl &&pin) noexcept : _pin{pin} {}

    /// @brief Read distance in millimeters
    /// @return Distance in millimeters (inverse of voltage)
    /// @note Takes multiple samples for noise reduction
    /// @warning correct calculus only with 10-bit ADC resolution
    [[nodiscard]] math::Millimeters read() const noexcept {
        // todo generalize formula
        return 65535.0F / math::Millimeters(_pin.read());
    }

private:
    PinImpl _pin;

    // initable impl

    friend struct kf::mixin::Initable<Sharp<I>, void>;
    void initImpl() noexcept { _pin.init(); }
};

}// namespace kf::drivers::sensors