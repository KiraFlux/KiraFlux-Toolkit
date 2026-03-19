// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "kf/gpio/GPIO.hpp"
#include "kf/math/units.hpp"
#include "kf/meta/type_check.hpp"

#include "kf/drivers/sensors/Sensor.hpp"

namespace kf::drivers::sensors {

/// @brief Sharp infrared distance sensor driver
/// @note Provides distance measurements in millimeters using analog voltage output
template<typename I> struct Sharp : Sensor<Sharp<I>, math::Millimeters, void> {
    kf_crtp_check(I, kf::gpio::AdcInputTag);
    using PinImpl = I;

    explicit Sharp(PinImpl &&pin) noexcept : _pin{std::move(pin)} {}

private:
    PinImpl _pin;

    // impl
    using This = Sharp<I>;

    KF_IMPL_INITABLE(This, void);
    void initImpl() noexcept { _pin.init(); }

    friend struct Sensor<This, math::Millimeters, void>;
    /// @brief Read distance in millimeters
    /// @return Distance in millimeters (inverse of voltage)
    /// @note Takes multiple samples for noise reduction
    /// @warning correct calculus only with 10-bit ADC resolution
    [[nodiscard]] math::Millimeters readImpl() noexcept {
        // todo generalize formula
        return 65535.0F / math::Millimeters(_pin.read());
    }
};

}// namespace kf::drivers::sensors