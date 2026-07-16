// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "kf/concepts.hpp"
#include "kf/gpio.hpp"
#include "kf/units.hpp"

#include "kf/driver/sensor/SensorDriver.hpp"

namespace kf::driver::sensor {

/// @brief Sharp infrared distance sensor driver
/// @note Provides distance measurements in millimeters using analog voltage output
/// @tparam G Implementation of GPIO with ADC input support
template<implements<gpio::AdcInputTag> G> struct Sharp : SensorDriver<Sharp<G>, units::Millimeters, void()> {

    using AdcInputImpl = G;

    explicit Sharp(AdcInputImpl &&gpio) noexcept :
        _gpio{std::move(gpio)} {}

private:
    AdcInputImpl _gpio;

    KF_IMPL_SENSOR_DRIVER(Sharp<G>, units::Millimeters, void());

    void initImpl() noexcept {
        _gpio.init();
    }

    [[nodiscard]] units::Millimeters readImpl() noexcept {
        // correct calculus only with 10-bit ADC resolution
        // TODO: generalize formula
        return 65535.0F / units::Millimeters(_gpio.read());
    }
};

}// namespace kf::driver::sensor