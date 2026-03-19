// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
#include "kf/math/units.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf::gpio {

/// @brief CRTP base for inputs.
/// @note Requires `T readImpl() const` in derived class.
template<typename Impl, typename LevelType, typename InitResultType> struct Input : mixin::Initable<Impl, InitResultType>, mixin::NonCopyable {
    LevelType read() const noexcept { return static_cast<const Impl *>(this)->readImpl(); }
};

struct DigitalInputTag {};

/// @brief Digital input specialization.
template<typename Impl, typename InitResultType> struct DigitalInput : Input<Impl, bool, InitResultType>, DigitalInputTag {
    static constexpr u8 external_pull_bit{0b01};
    static constexpr u8 pull_up_bit{0b10};

    /// @brief Pull configuration
    enum class Pull : u8 {
        InternalDown = 0,
        ExternalDown = external_pull_bit,
        InternalUp = pull_up_bit,
        ExternalUp = external_pull_bit | pull_up_bit,
    };
};

struct AdcInputTag {};

/// @brief Analog input specialization.
template<typename Impl, typename InitResultType> struct AdcInput : Input<Impl, u16, InitResultType>, AdcInputTag {
    static void resolution(u8 resolution_bits) noexcept { Impl::setResolutionImpl(resolution_bits); }

    [[nodiscard]] static u8 resolution() noexcept { return Impl::getResolutionImpl(); }

    [[nodiscard]] static u16 maxValue() noexcept { return static_cast<u16>((1u << resolution()) - 1u); }
};

/// @brief CRTP base for outputs.
/// @note Requires `void writeImpl(T) const` in derived class.
template<typename Impl, typename LevelType, typename InitResultType> struct Output : mixin::Initable<Impl, InitResultType>, mixin::NonCopyable {
    void write(LevelType level) const noexcept { static_cast<const Impl *>(this)->writeImpl(level); }
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
