// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "kf/gpio/GPIO.hpp"
#include "kf/math/units.hpp"

#include "kf/drivers/sensors/Sensor.hpp"

namespace kf::drivers::sensors {

/// @brief Sharp infrared distance sensor driver
/// @note Provides distance measurements in millimeters using analog voltage output
/// @tparam G Implementation of GPIO with ADC input support
template<typename G> struct Sharp : Sensor<Sharp<G>, math::Millimeters, void> {
    KF_CHECK_IMPL(G, ::kf::gpio::GPIO::AdcInputTag);
    using PinImpl = G;

    explicit Sharp(PinImpl &&pin) noexcept : _pin{std::move(pin)} {}

private:
    PinImpl _pin;

    // impl
    using This = Sharp<G>;

    KF_IMPL_INITABLE(This, void);
    void initImpl() noexcept { _pin.init(); }

    KF_IMPL(Sensor<This, math::Millimeters, void>);
    [[nodiscard]] math::Millimeters readImpl() noexcept {
        // correct calculus only with 10-bit ADC resolution
        // todo generalize formula
        return 65535.0F / math::Millimeters(_pin.read());
    }
};

}// namespace kf::drivers::sensors