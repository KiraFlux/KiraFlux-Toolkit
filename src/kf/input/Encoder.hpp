// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/math/units.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/validation.hpp"

namespace kf::input {

/// @brief Two-phase incremental rotary encoder with position tracking
/// @note Uses interrupt on phase A for accurate position counting
struct Encoder : mixin::Initable<Encoder, void>, mixin::NonCopyable {

    /// @brief Alias for encoder position in ticks
    using Ticks = i32;

    static constexpr auto logger{Logger::create("Encoder")};

    /// @brief Conversion configuration between ticks and physical units
    struct ConversionConfig final : Validatable<ConversionConfig>, mixin::NonCopyable {
        f32 ticks_in_one_mm;///< Ticks per millimeter (must be positive)

        /// @brief Convert ticks to millimeters
        [[nodiscard]] constexpr math::Millimeters toMillimeters(Ticks ticks) const noexcept {
            return math::Millimeters{ticks} / ticks_in_one_mm;
        }

        /// @brief Convert millimeters to ticks
        [[nodiscard]] constexpr Ticks toTicks(math::Millimeters mm) const noexcept {
            return Ticks{mm * ticks_in_one_mm};
        }

        /// @brief Validate conversion config
        void checkImpl(Validator &validator) const noexcept {
            kf_Validator_check(validator, logger, ticks_in_one_mm > 0);
        }
    };

    /// @brief GPIO pin configuration for encoder
    struct PinsConfig final : mixin::NonCopyable {
        /// @brief Interrupt trigger edge
        enum class Edge : u8 {
            Rising = RISING, ///< Trigger on rising edge (LOW to HIGH)
            Falling = FALLING///< Trigger on falling edge (HIGH to LOW)
        };

        u8 phase_a;///< Primary signal pin (interrupt source)
        u8 phase_b;///< Secondary phase pin (direction detection)
        Edge edge; ///< Interrupt trigger edge
    };

    const PinsConfig &pins;
    const ConversionConfig &conversion;

    explicit Encoder(
        const PinsConfig &pins_settings,
        const ConversionConfig &conversion_settings) noexcept :
        pins{pins_settings}, conversion{conversion_settings} {}

    /// @brief Enable interrupt handling for encoder
    void enable() {
        attachInterruptArg(
            pins.phase_a,
            interruptHandler,
            static_cast<void *>(this),
            static_cast<int>(pins.edge));
    }

    /// @brief Disable encoder interrupts
    void disable() const {
        detachInterrupt(pins.phase_a);
    }

    /// @brief Get current position in ticks
    [[nodiscard]] Ticks position() const noexcept {
        return _position;
    }

    /// @brief Set position in ticks
    void position(Ticks new_position) noexcept {
        _position = new_position;
    }

    /// @brief Get current position in millimeters
    [[nodiscard]] math::Millimeters positionMillimeters() const noexcept {
        return conversion.toMillimeters(_position);
    }

    /// @brief Set position in millimeters
    void positionMillimeters(math::Millimeters new_position) noexcept {
        _position = conversion.toTicks(new_position);
    }

private:
    volatile Ticks _position{0};///< Current position in ticks

    /// @brief Primary phase interrupt handler for rotary encoder
    IRAM_ATTR static void interruptHandler(void *instance) noexcept {
        auto &encoder = *static_cast<Encoder *>(instance);

        if (digitalRead(encoder.pins.phase_b)) {
            encoder._position += 1;
        } else {
            encoder._position -= 1;
        }
    }

    // impl
    using This = Encoder;

    KF_IMPL_INITABLE(This, void);
    void initImpl() noexcept {
        pinMode(pins.phase_a, INPUT);
        pinMode(pins.phase_b, INPUT);
    }
};

}// namespace kf::input
