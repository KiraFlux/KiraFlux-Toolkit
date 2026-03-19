// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/meta/CRTP.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf::drivers::actuators {

struct ActuatorTag {};

/// @brief Actuator CRTP interface
/// @tparam Impl Actuator implementation
/// @tparam InitResult Reslt of `init()`
template<typename Impl, typename InitResult>
struct Actuator : ActuatorTag, meta::CRTP<Impl>, mixin::Initable<Impl, InitResult>, mixin::NonCopyable {};

}// namespace kf::drivers::actuators
