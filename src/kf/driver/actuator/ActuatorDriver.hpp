// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/meta/CRTP.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf::driver::actuator {

struct ActuatorDriverTag {};

/// @brief Actuator Driver CRTP interface
/// @tparam Impl Actuator Driver implementation
/// @tparam InitSignature `init()`
template<typename Impl, typename Unit, typename InitSignature> struct ActuatorDriver :

    ActuatorDriverTag,
    meta::CRTP<Impl>,
    mixin::NonCopyable,
    mixin::Initable<Impl, InitSignature>

{
    void set(Unit value) noexcept {
        this->impl().setImpl(value);
    }

    void stop() noexcept {
        this->impl().stopImpl();
    }
};

}// namespace kf::driver::actuator

#define KF_IMPL_ACTUATOR_DRIVER(__impl__, __unit_type__, ...)                                          \
    friend struct ::kf::driver::actuator::ActuatorDriver<__impl__, __unit_type__, __VA_ARGS__>; \
    KF_IMPL_INITABLE(__impl__, __VA_ARGS__)
