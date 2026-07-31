// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/gpio.hpp"
#include "kf/primitives.hpp"

#include "kf/mixin/Configured.hpp"
#include "kf/mixin/Resettable.hpp"

#include "kf/driver/sensor/SensorDriver.hpp"

namespace kf::internal {

/// @brief Configuration for a QuadratureEncoder
/// @tparam T Physical unit type representing linear distance per tick
template<typename T> struct QuadratureEncoderConfig final {

    using TickType = kf::isize;    ///< Integral tick counter type
    using UnitType = T;            ///< Physical unit type
    using StepType = kf::i8;       ///< Direction step type
    using PhaseStateType = kf::u32;///< Packed two-bit phase state (AB)

    /// @brief Rotation direction that increments the tick counter
    /// @note Follows the right-hand grip rule
    enum class Direction : StepType {
        CW = -1,///< Clockwise decrements (negative increment)
        CCW = +1///< Counter-clockwise increments
    };

    UnitType units_per_tick;      ///< Conversion factor: physical units per encoder tick
    Direction positive_direction; ///< Desired positive rotation direction
    gpio::DigitalInput::Pull pull;///< Phase Pull type

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
    using Self = QuadratureEncoder<T>;

    using Config = internal::QuadratureEncoderConfig<T>;

    explicit constexpr QuadratureEncoder(Config const &config, gpio::GpioNumber gpio_num_phase_a, gpio::GpioNumber gpio_num_phase_b) noexcept :
        mixin::Configured<Config>::Configured{config}, _gpio_phase_a{gpio_num_phase_a, config.pull}, _gpio_phase_b{gpio_num_phase_b, config.pull} {}

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
    gpio::DigitalInput _gpio_phase_a, _gpio_phase_b;
    typename Config::TickType volatile _position_ticks{0};  ///< Accumulated step count
    typename Config::PhaseStateType volatile _last_state{0};///< Previous AB phase state

    /// @brief ISR triggered on any edge of either phase
    static void KF_PLACE_IRAM onAnyPhaseChange(void *arg) noexcept {
        auto &self = *static_cast<Self *>(arg);
        auto const positive_step = static_cast<typename Config::StepType>(self.config().positive_direction);

        auto const current_state = self.read();

        // Index formed by concatenating last and current states (4 bits)
        // 4X decoding lookup table (index = (last_A << 3 | last_B << 2 | cur_A << 1 | cur_B))
        switch ((self._last_state << 2) | current_state) {
            case 0b00'01:
            case 0b01'11:
            case 0b10'00:
            case 0b11'10:
                self._position_ticks += positive_step;
                break;

            case 0b00'10:
            case 0b01'00:
            case 0b10'11:
            case 0b11'01:
                self._position_ticks -= positive_step;
                break;
        }

        self._last_state = current_state;
    }

    KF_IMPL_SENSOR_DRIVER(Self, typename Config::PhaseStateType, void());

    void initImpl() noexcept {
        _gpio_phase_a.init();
        _gpio_phase_a.attachInterrupt(onAnyPhaseChange, static_cast<void *>(this), gpio::DigitalInput::Interrupt::OnChange);

        _gpio_phase_b.init();
        _gpio_phase_b.attachInterrupt(onAnyPhaseChange, static_cast<void *>(this), gpio::DigitalInput::Interrupt::OnChange);

        this->reset();
    }

    typename Config::PhaseStateType readImpl() const noexcept {
        auto const state_a = static_cast<typename Config::PhaseStateType>(_gpio_phase_a.level());
        auto const state_b = static_cast<typename Config::PhaseStateType>(_gpio_phase_b.level());
        return (state_a << 1) | state_b;// pack as AB
    }

    KF_IMPL_RESETTABLE(Self);
    void resetImpl() noexcept {
        _position_ticks = 0;
        _last_state = this->read();
    }
};

}// namespace kf::driver::sensor