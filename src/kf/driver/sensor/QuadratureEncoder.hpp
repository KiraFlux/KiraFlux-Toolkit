// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/GPIO.hpp"
#include "kf/concepts.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/mixin/Resettable.hpp"
#include "kf/primitives.hpp"

#include "kf/driver/sensor/SensorDriver.hpp"

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

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

    UnitType units_per_tick;     ///< Conversion factor: physical units per encoder tick
    Direction positive_direction;///< Desired positive rotation direction

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
template<implements<GPIO::DigitalInputTag> G, typename T> struct QuadratureEncoder final :

    driver::sensor::SensorDriver<QuadratureEncoder<G, T>, typename internal::QuadratureEncoderConfig<T>::PhaseStateType, void()>,
    mixin::Resettable<QuadratureEncoder<G, T>>,
    mixin::Configured<internal::QuadratureEncoderConfig<T>>

{
    using DigitalInputImpl = G;

    using Config = internal::QuadratureEncoderConfig<T>;

    explicit constexpr QuadratureEncoder(const Config &config, DigitalInputImpl &&gpio_phase_a, DigitalInputImpl &&gpio_phase_b) noexcept :
        mixin::Configured<Config>::Configured{config}, _gpio_phase_a{std::move(gpio_phase_a)}, _gpio_phase_b{std::move(gpio_phase_b)} {}

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
    DigitalInputImpl _gpio_phase_a, _gpio_phase_b;
    volatile typename Config::TickType _position_ticks{0};  ///< Accumulated step count
    volatile typename Config::PhaseStateType _last_state{0};///< Previous AB phase state

    using This = QuadratureEncoder<G, T>;

    /// @brief ISR triggered on any edge of either phase
    static void IRAM_ATTR onAnyPhaseChange(void *arg) noexcept {
        auto &self = *static_cast<This *>(arg);
        const auto positive_step = static_cast<typename Config::StepType>(self.config().positive_direction);

        const auto current_state = self.read();

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

    KF_IMPL_SENSOR_DRIVER(This, typename Config::PhaseStateType, void());

    void initImpl() noexcept {
        _gpio_phase_a.init();
        _gpio_phase_a.attachInterrupt(onAnyPhaseChange, static_cast<void *>(this), DigitalInputImpl::Interrupt::OnChange);

        _gpio_phase_b.init();
        _gpio_phase_b.attachInterrupt(onAnyPhaseChange, static_cast<void *>(this), DigitalInputImpl::Interrupt::OnChange);

        this->reset();
    }

    typename Config::PhaseStateType readImpl() const noexcept {
        const auto state_a = static_cast<typename Config::PhaseStateType>(_gpio_phase_a.read());
        const auto state_b = static_cast<typename Config::PhaseStateType>(_gpio_phase_b.read());
        return (state_a << 1) | state_b;// pack as AB
    }

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        _position_ticks = 0;
        _last_state = this->read();
    }
};

}// namespace kf::driver::sensor