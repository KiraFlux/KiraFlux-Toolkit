// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/gpio.hpp"
#include "kf/units.hpp"

#include "kf/driver/sensor/SensorDriver.hpp"

namespace kf::driver::sensor {

/// @brief Sharp infrared distance sensor driver
/// @note Provides distance measurements in millimeters using analog voltage output
struct Sharp final : SensorDriver<Sharp, units::Millimeters, void()> {
    using Self = Sharp;

    explicit Sharp(gpio::GpioNumber gpio_num) noexcept :
        _adc_input{gpio_num} {}

private:
    gpio::AdcInput _adc_input;

    KF_IMPL_SENSOR_DRIVER(Self, units::Millimeters, void());

    void initImpl() noexcept {
        _adc_input.init();
    }

    units::Millimeters readImpl() noexcept {
        // correct calculus only with 10-bit ADC resolution
        // TODO: generalize formula
        return 65535.0F / units::Millimeters(_adc_input.read());
    }
};

}// namespace kf::driver::sensor