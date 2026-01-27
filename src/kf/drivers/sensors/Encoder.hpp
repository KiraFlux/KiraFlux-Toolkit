// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/core/attributes.hpp"
#include "kf/math/units.hpp"
#include "kf/validation.hpp"


/// @brief Primary phase interrupt handler for rotary encoder
static void IRAM_ATTR encoderInterruptHandler(void *);

namespace kf {

/// @brief Two-phase incremental rotary encoder with position tracking
/// @note Uses interrupt on phase A for accurate position counting
struct Encoder {

    /// @brief Alias for encoder position in ticks
    using Ticks = i32;

    /// @brief Conversion settings between ticks and physical units
    struct ConversionSettings : Validable<ConversionSettings> {
        f32 ticks_in_one_mm;///< Ticks per millimeter (must be positive)

        /// @brief Convert ticks to millimeters
        kf_nodiscard constexpr Millimeters toMillimeters(Ticks ticks) const noexcept {
            return Millimeters(ticks) / ticks_in_one_mm;
        }

        /// @brief Convert millimeters to ticks
        kf_nodiscard constexpr Ticks toTicks(Millimeters mm) const noexcept {
            return Ticks(mm * ticks_in_one_mm);
        }

        /// @brief Validate conversion settings
        /// @param validator Validation context
        void check(Validator &validator) const noexcept {
            kf_Validator_check(validator, ticks_in_one_mm > 0);
        }
    };

    /// @brief GPIO pin configuration for encoder
    struct PinsSettings {
        /// @brief Interrupt trigger edge
        enum class Edge : u8 {
            Rising = RISING, ///< Trigger on rising edge (LOW to HIGH)
            Falling = FALLING///< Trigger on falling edge (HIGH to LOW)
        };

        u8 phase_a;///< Primary signal pin (interrupt source)
        u8 phase_b;///< Secondary phase pin (direction detection)
        Edge edge; ///< Interrupt trigger edge
    };

    const PinsSettings &pins;            ///< Pin configuration reference
    const ConversionSettings &conversion;///< Unit conversion settings
    Ticks position{0};                   ///< Current position in ticks

    explicit Encoder(
        const PinsSettings &pins_settings,
        const ConversionSettings &conversion_settings
    ) noexcept:
        pins{pins_settings}, conversion{conversion_settings} {}

    /// @brief Initialize encoder GPIO pins
    /// @note Must be called before enabling interrupts
    void init() noexcept {
        pinMode(pins.phase_a, INPUT);
        pinMode(pins.phase_b, INPUT);
        enable();
    }

    /// @brief Enable interrupt handling for encoder
    void enable() {
        attachInterruptArg(
            pins.phase_a,
            encoderInterruptHandler,
            static_cast<void *>(this),
            static_cast<int>(pins.edge));
    }

    /// @brief Disable encoder interrupts
    void disable() const {
        detachInterrupt(pins.phase_a);
    }

    /// @brief Get current position in ticks
    /// @return Encoder position in ticks
    kf_nodiscard inline Ticks getPositionTicks() const noexcept {
        return position;
    }

    /// @brief Set position in ticks
    /// @param new_position New tick count
    void setPositionTicks(Ticks new_position) noexcept {
        position = new_position;
    }

    /// @brief Get current position in millimeters
    /// @return Encoder position in millimeters
    kf_nodiscard inline Millimeters getPositionMillimeters() const noexcept {
        return conversion.toMillimeters(position);
    }

    /// @brief Set position in millimeters
    /// @param new_position New position in millimeters
    void setPositionMillimeters(Millimeters new_position) noexcept {
        position = conversion.toTicks(new_position);
    }
};

}// namespace kf

/// @brief Interrupt handler for rotary encoder
void encoderInterruptHandler(void *instance) noexcept {
    auto &encoder = *static_cast<kf::Encoder *>(instance);

    if (digitalRead(encoder.pins.phase_b)) {
        encoder.position += 1;
    } else {
        encoder.position -= 1;
    }
}
