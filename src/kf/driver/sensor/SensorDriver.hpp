// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/CRTP.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf::driver::sensor {

struct SensorDriverTag {};

/// @brief SensorDriver CRTP interface
/// @tparam Impl SensorDriver implementation
/// @tparam Measurement Result of `read()`
/// @tparam InitSignature signature of `init`
template<typename Impl, typename Measurement, typename InitSignature> struct SensorDriver :

    SensorDriverTag,
    mixin::CRTP<Impl>,
    mixin::NonCopyable,
    mixin::Initable<Impl, InitSignature>

{

    [[nodiscard]] Measurement read() noexcept {
        return this->impl().readImpl();
    }
};

}// namespace kf::driver::sensor

#define KF_IMPL_SENSOR_DRIVER(__impl__, __measurement_type__, ...)                                 \
    friend struct ::kf::driver::sensor::SensorDriver<__impl__, __measurement_type__, __VA_ARGS__>; \
    KF_IMPL_INITABLE(__impl__, __VA_ARGS__)
