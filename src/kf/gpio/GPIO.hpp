// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/math/units.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/primitives.hpp"

namespace kf::gpio {

struct GpioTag {};

struct GPIO final : GpioTag {

    /// @brief CRTP base for input peripherals.
    /// @tparam Impl         The derived class (must implement `readImpl()`).
    /// @tparam LevelType    Type of the input value (e.g., bool, u16).
    /// @tparam InitResult   Result type of the initialization (e.g., void, bool).
    template<typename Impl, typename LevelType, typename InitResultType> struct Input : mixin::Initable<Impl, InitResultType>, mixin::NonCopyable {

        /// @brief Reads the current input value.
        /// @return The value read from the hardware .
        LevelType read() const noexcept {
            return static_cast<const Impl *>(this)->readImpl();
        }
    };

    struct DigitalInputTag {};

    /// @brief Specialization for digital inputs.
    /// @tparam Impl         Concrete implementation class.
    /// @tparam InitResult   Return type of the initialization.
    /// @note Derived classes must define `Pull` enum and implement `readImpl()`.
    template<typename Impl, typename InitResultType> struct DigitalInput : Input<Impl, bool, InitResultType>, DigitalInputTag {
        static constexpr u8 external_pull_bit{0b01};
        static constexpr u8 pull_up_bit{0b10};

        /// @brief Pull configuration options.
        enum class Pull : u8 {
            InternalDown = 0,                            ///< No pull (internal high‑impedance).
            ExternalDown = external_pull_bit,            ///< External pull-down resistor.
            InternalUp = pull_up_bit,                    ///< Internal pull-up resistor.
            ExternalUp = external_pull_bit | pull_up_bit,///< External pull-up resistor.
        };
    };

    struct AdcInputTag {};

    /// @brief Specialization for analog inputs (ADC).
    /// @tparam Impl         Concrete implementation class.
    /// @tparam InitResult   Return type of the initialization.
    /// @note Requires static methods `setResolutionImpl(u8)` and `getResolutionImpl()`.
    template<typename Impl, typename InitResultType> struct AdcInput : Input<Impl, u16, InitResultType>, AdcInputTag {

        /// @brief Sets the ADC resolution globally.
        /// @param resolution_bits Number of bits (e.g., 10, 12).
        static void resolution(u8 resolution_bits) noexcept {
            Impl::setResolutionImpl(resolution_bits);
        }

        /// @brief Returns the current ADC resolution in bits.
        /// @return Number of bits used for analog readings.
        [[nodiscard]] static u8 resolution() noexcept {
            return Impl::getResolutionImpl();
        }

        /// @brief Returns the maximum possible ADC value (2^resolution - 1).
        /// @return Maximum value for the current resolution.
        [[nodiscard]] static u16 maxValue() noexcept {
            return static_cast<u16>((1u << resolution()) - 1u);
        }
    };

    /// @brief CRTP base for output peripherals.
    /// @tparam Impl         Concrete implementation class (must provide `writeImpl()`).
    /// @tparam LevelType    Type of the output value (e.g., `bool`, `u16`).
    /// @tparam InitResult   Return type of the initialization.
    template<typename Impl, typename LevelType, typename InitResultType> struct Output : mixin::Initable<Impl, InitResultType>, mixin::NonCopyable {
        void write(LevelType level) const noexcept {
            static_cast<const Impl *>(this)->writeImpl(level);
        }
    };

    struct DigitalOutputTag {};

    /// @brief Specialization for digital outputs.
    /// @tparam Impl         Concrete implementation class.
    /// @tparam InitResult   Return type of the initialization.
    template<typename Impl, typename InitResultType> struct DigitalOutput : Output<Impl, bool, InitResultType>, DigitalOutputTag {};

    struct PwmOutputTag {};

    /// @brief Specialization for PWM outputs.
    /// @tparam Impl         Concrete implementation class.
    /// @tparam InitResult   Return type of the initialization.
    /// @note Requires methods `getFrequencyImpl()`, `getResolutionImpl()`.
    template<typename Impl, typename InitResultType> struct PwmOutput : Output<Impl, u16, InitResultType>, PwmOutputTag {

        /// @brief Returns the PWM frequency in Hz.
        [[nodiscard]] u32 frequency() const noexcept {
            return static_cast<const Impl *>(this)->getFrequencyImpl();
        }

        /// @brief Returns the PWM resolution in bits.
        [[nodiscard]] u8 resolution() const noexcept {
            return static_cast<const Impl *>(this)->getResolutionImpl();
        }

        /// @brief Returns the maximum duty cycle value (2^resolution - 1).
        [[nodiscard]] u16 maxDuty() const noexcept {
            return static_cast<u16>((1u << resolution()) - 1u);
        }

        /// @brief Converts a pulse width (microseconds) to a duty cycle value.
        /// @param pulse_width Pulse width in microseconds.
        /// @return Duty cycle value suitable for `write()`.
        [[nodiscard]] u16 dutyFromPulseWidth(kf::math::Microseconds pulse_width) const noexcept {
            const auto t = u64{pulse_width} * frequency() * maxDuty();
            return static_cast<u16>(t / 1'000'000u);
        }
    };
};

}// namespace kf::gpio
