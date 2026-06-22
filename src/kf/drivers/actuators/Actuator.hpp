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
/// @tparam InitSignature `init()`
template<typename Impl, typename InitSignature> struct Actuator :

    ActuatorTag,
    meta::CRTP<Impl>,
    mixin::NonCopyable,
    mixin::Initable<Impl, InitSignature>

{};

}// namespace kf::drivers::actuators