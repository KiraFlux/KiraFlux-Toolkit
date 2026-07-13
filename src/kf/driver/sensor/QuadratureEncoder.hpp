// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/driver/sensor/SensorDriver.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/mixin/Resettable.hpp"
#include "kf/primitives.hpp"

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

namespace kf::internal {

/// @brief Configuration for a QuadratureEncoder
/// @tparam T Physical unit type representing linear distance per tick
template<typename T> struct QuadratureEncoderConfig final {

    using TickType = kf::i32;      ///< Integral tick counter type
    using UnitType = T;            ///< Physical unit type
    using StepType = kf::i8;       ///< Direction step type
    using PhaseStateType = kf::u32;///< Packed two-bit phase state (AB)
    using GpioNumType = kf::u8;    ///< GPIO number type

    /// @brief Rotation direction that increments the tick counter
    /// @note Follows the right-hand grip rule
    enum class Direction : StepType {
        CW = -1,///< Clockwise decrements (negative increment)
        CCW = +1///< Counter-clockwise increments
    };

    UnitType units_per_tick;                       ///< Conversion factor: physical units per encoder tick
    Direction positive_direction;                  ///< Desired positive rotation direction
    GpioNumType gpio_num_phase_a, gpio_num_phase_b;///< GPIO pins for phase A and phase B

    /// @brief Converts ticks to physical units
    [[nodiscard]] constexpr UnitType unitsFromTicks(TickType value) const noexcept {
        return static_cast<UnitType>(value) * units_per_tick;
    }

    /// @brief Converts physical units to ticks (truncated)
    /// @note For integral unit types, precision loss may occur due to integer division
    [[nodiscard]] constexpr TickType ticksFromUnits(UnitType value) const noexcept {
        return static_cast<TickType>(value / units_per_tick);
    }
};

}// namespace kf::internal

namespace kf::driver::sensor {

/// @brief Quadrature encoder sensor with 4X decoding
/// @note Arduino-Only
/// @tparam T Physical linear unit
template<typename T> struct QuadratureEncoder final :

    driver::sensor::SensorDriver<QuadratureEncoder<T>, typename internal::QuadratureEncoderConfig<T>::PhaseStateType, void()>,
    mixin::Resettable<QuadratureEncoder<T>>,
    mixin::Configured<internal::QuadratureEncoderConfig<T>>

{
    using Config = internal::QuadratureEncoderConfig<T>;

    using mixin::Configured<Config>::Configured;

    /// @brief Current accumulated position in ticks
    [[nodiscard]] auto positionTicks() const noexcept -> typename Config::TickType {
        return _position_ticks;
    }

    /// @brief Overwrite the current tick count
    void positionTicks(typename Config::TickType position) noexcept {
        _position_ticks = position;
    }

    /// @brief Current position converted to physical units
    [[nodiscard]] auto positionUnits() const noexcept -> typename Config::UnitType {
        return this->config().unitsFromTicks(_position_ticks);
    }

    /// @brief Set position in physical units (converted to ticks)
    void positionUnits(typename Config::UnitType position) noexcept {
        _position_ticks = this->config().ticksFromUnits(position);
    }

private:
    volatile typename Config::TickType _position_ticks{0};  ///< Accumulated step count
    volatile typename Config::PhaseStateType _last_state{0};///< Previous AB phase state

    /// @brief ISR triggered on any edge of either phase
    static void IRAM_ATTR onAnyPhaseChange(void *arg) {
        // 4X decoding lookup table (index = (prev_A << 3 | prev_B << 2 | cur_A << 1 | cur_B))
        static const typename Config::StepType increment_table[16] = {
            0, // 00 -> 00 : invalid
            +1,// 00 -> 01 : CW step
            -1,// 00 -> 10 : CCW step
            0, // 00 -> 11 : invalid
            -1,// 01 -> 00 : CCW step
            0, // 01 -> 01 : invalid
            0, // 01 -> 10 : invalid
            +1,// 01 -> 11 : CW step
            +1,// 10 -> 00 : CW step
            0, // 10 -> 01 : invalid
            0, // 10 -> 10 : invalid
            -1,// 10 -> 11 : CCW step
            0, // 11 -> 00 : invalid
            -1,// 11 -> 01 : CCW step
            +1,// 11 -> 10 : CW step
            0  // 11 -> 11 : invalid
        };

        auto &self = *static_cast<QuadratureEncoder *>(arg);
        const auto current_state = self.read();

        // Index formed by concatenating previous and current states (4 bits)
        self._position_ticks += increment_table[(current_state | (self._last_state << 2))] * static_cast<typename Config::StepType>(self.config().positive_direction);
        self._last_state = current_state;
    }

    // Implementation details
    KF_IMPL_SENSOR_DRIVER(QuadratureEncoder<T>, typename Config::PhaseStateType, void());

    void initImpl() noexcept {
        pinMode(this->config().gpio_num_phase_a, INPUT);
        attachInterruptArg(digitalPinToInterrupt(this->config().gpio_num_phase_a), onAnyPhaseChange, static_cast<void *>(this), CHANGE);

        pinMode(this->config().gpio_num_phase_b, INPUT);
        attachInterruptArg(digitalPinToInterrupt(this->config().gpio_num_phase_b), onAnyPhaseChange, static_cast<void *>(this), CHANGE);

        this->reset();
    }

    typename Config::PhaseStateType readImpl() const noexcept {
        const auto state_a = static_cast<typename Config::PhaseStateType>(digitalRead(this->config().gpio_num_phase_a));
        const auto state_b = static_cast<typename Config::PhaseStateType>(digitalRead(this->config().gpio_num_phase_b));
        return (state_a << 1) | state_b;// pack as AB
    }

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        _position_ticks = 0;
        _last_state = this->read();
    }
};

}// namespace kf::driver::sensor