// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/meta/CRTP.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf::driver::sensor {

struct SensorTag {};
// TODO: rename to SensorDriver

/// @brief Sensor CRTP interface
/// @tparam Impl Sensor implementation
/// @tparam Measurement Result of `read()`
/// @tparam InitSignature signature of `init`
template<typename Impl, typename Measurement, typename InitSignature> struct Sensor :

    SensorTag,
    meta::CRTP<Impl>,
    mixin::NonCopyable,
    mixin::Initable<Impl, InitSignature>

{

    [[nodiscard]] Measurement read() noexcept {
        return this->impl().readImpl();
    }
};

}// namespace kf::driver::sensor

#define KF_IMPL_SENSOR(__impl__, __measurement_type__, ...)                                  \
    friend struct ::kf::driver::sensor::Sensor<__impl__, __measurement_type__, __VA_ARGS__>; \
    KF_IMPL_INITABLE(__impl__, __VA_ARGS__)
