// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/meta/CRTP.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf::drivers::sensors {

struct SensorTag {};

/// @brief Sensor CRTP interface
/// @tparam Impl Sensor implementation
/// @tparam Measurement Result of `read()`
/// @tparam InitResult Reslt of `init()`
template<typename Impl, typename Measurement, typename InitResult>
struct Sensor : SensorTag, meta::CRTP<Impl>, mixin::Initable<Impl, InitResult>, mixin::NonCopyable {

    [[nodiscard]] Measurement read() noexcept { return this->impl().readImpl(); }
};

}// namespace kf::drivers::sensors
