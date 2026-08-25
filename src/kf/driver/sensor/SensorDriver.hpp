// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    driver/sensor/SensorDriver.hpp
/// @brief   CRTP base for sensors (read, init).

#pragma once

#include "kf/driver/Driver.hpp"

namespace kf::driver::sensor {

struct SensorDriverTag {};

/// @brief SensorDriver CRTP interface
/// @tparam Impl SensorDriver implementation
/// @tparam Measurement Result of `read()`
/// @tparam InitResult return type of `init`
template<typename Impl, typename Measurement, typename InitResult> struct SensorDriver :

    SensorDriverTag,
    Driver<Impl, InitResult>

{

    /// @brief Read sensor value
    [[nodiscard]] Measurement read() noexcept {
        return static_cast<Impl *>(this)->readImpl();
    }
};

}// namespace kf::driver::sensor

#define KF_IMPL_SENSOR_DRIVER(__impl__, __measurement_type__, ...)                                 \
    friend struct ::kf::driver::sensor::SensorDriver<__impl__, __measurement_type__, __VA_ARGS__>; \
    KF_IMPL_DRIVER(__impl__, __VA_ARGS__)
