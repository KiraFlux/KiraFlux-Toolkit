// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
#include "kf/math/units.hpp"
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
template<typename Impl, typename InitResultType> struct AdcInput : Input<Impl, u16, InitResultType>, AdcInputTag {
    static void resolution(u8 resolution_bits) noexcept { Impl::setResolutionImpl(resolution_bits); }

    [[nodiscard]] static u8 resolution() noexcept { return Impl::getResolutionImpl(); }

    [[nodiscard]] static u16 maxValue() noexcept { return static_cast<u16>((1u << resolution()) - 1u); }
};

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
template<typename Impl, typename InitResultType> struct PwmOutput : Output<Impl, u16, InitResultType>, PwmOutputTag {
    [[nodiscard]] u32 frequency() const noexcept { return static_cast<const Impl *>(this)->getFrequencyImpl(); }

    [[nodiscard]] u8 resolution() const noexcept { return static_cast<const Impl *>(this)->getResolutionImpl(); }

    [[nodiscard]] u16 maxDuty() const noexcept { return static_cast<u16>((1u << resolution()) - 1u); }

    [[nodiscard]] u16 dutyFromPulseWidth(kf::math::Microseconds pulse_width) const noexcept {
        const auto t = u64{pulse_width} * frequency() * maxDuty();
        return static_cast<u16>(t / 1'000'000u);
    }
};

}// namespace kf::gpio
