// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/meta/CRTP.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf::driver::sensor {

struct SensorTag {};

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
