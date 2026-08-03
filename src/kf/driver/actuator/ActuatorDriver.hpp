// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    driver/actuator/ActuatorDriver.hpp
/// @brief   CRTP base for actuators (set, stop, init).

#pragma once

#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"

#include "kf/driver/Driver.hpp"

namespace kf::driver::actuator {

struct ActuatorDriverTag {};

/// @brief Actuator Driver CRTP interface
/// @tparam Impl Actuator Driver implementation
/// @tparam InitResult `init()`
template<typename Impl, typename Unit, typename InitResult> struct ActuatorDriver :

    ActuatorDriverTag,
    Driver<Impl, InitResult>

{

    /// @brief Set Control value
    void set(Unit value) noexcept {
        static_cast<Impl *>(this)->setImpl(value);
    }

    /// @brief Release actuator (disable output)
    void stop() noexcept {
        static_cast<Impl *>(this)->stopImpl();
    }
};

}// namespace kf::driver::actuator

#define KF_IMPL_ACTUATOR_DRIVER(__impl__, __unit_type__, ...)                                   \
    friend struct ::kf::driver::actuator::ActuatorDriver<__impl__, __unit_type__, __VA_ARGS__>; \
    KF_IMPL_DRIVER(__impl__, __VA_ARGS__)
