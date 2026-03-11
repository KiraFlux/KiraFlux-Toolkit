// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/mixin/Initable.hpp"

namespace kf::gpio {

/// @brief CRTP base for inputs.
/// @note Requires `T readImpl() const` in derived class.
template<typename Impl, typename LevelType, typename InitResultType> struct Input : meta::CRTP<Impl>, mixin::Initable<Impl, InitResultType> {
    LevelType read() const noexcept { return this->impl().readImpl(); }
};

struct DigitalInputTag {};

/// @brief Digital input specialization.
template<typename Impl, typename InitResultType> struct DigitalInput : Input<Impl, bool, InitResultType>, DigitalInputTag {};

struct AdcInputTag {};

/// @brief Analog input specialization.
template<typename Impl, typename InitResultType> struct AdcInput : Input<Impl, u16, InitResultType>, AdcInputTag {};

/// @brief CRTP base for outputs.
/// @note Requires `void writeImpl(T) const` in derived class.
template<typename Impl, typename LevelType, typename InitResultType> struct Output : meta::CRTP<Impl>, mixin::Initable<Impl, InitResultType> {
    void write(LevelType level) const noexcept { this->impl().writeImpl(level); }
};

struct DigitalOutputTag {};

/// @brief Digital output specialization.
template<typename Impl, typename InitResultType> struct DigitalOutput : Output<Impl, bool, InitResultType>, DigitalOutputTag {};

struct PwmOutputTag {};

/// @brief Analog output specialization.
template<typename Impl, typename InitResultType> struct PwmOutput : Output<Impl, u16, InitResultType>, PwmOutputTag {};

}// namespace kf::gpio
