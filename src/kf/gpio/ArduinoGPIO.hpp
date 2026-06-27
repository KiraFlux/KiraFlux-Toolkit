// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/gpio/GPIO.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/primitives.hpp"

namespace kf::internal {

struct ArduinoPwmOutputConfig final {
    u32 frequency_hz;  ///< PWM frequency (Hz)
    u8 resolution_bits;///< resolution in bits (1..16)
    u8 pin;            ///< GPIO pin number
    u8 channel;        ///< LEDC channel (0..15)

    /// @brief Maximum PWM value for given resolution (2^bits - 1).
    [[nodiscard]] constexpr u16 maxDuty() const noexcept {
        return static_cast<u16>((1u << (resolution_bits)) - 1u);
    }
};

}// namespace kf::internal

namespace kf::gpio {

/// @brief Arduino Core GPIO backend
struct ArduinoGPIO : GpioTag {

    /// @brief Arduino digital input with configurable pull-up/down.
    /// @note Pull mode is set during `init()`; after that the pin stays configured.
    struct DigitalInput : GPIO::DigitalInput<DigitalInput, void()> {

        /// @brief Constructor.
        /// @param pin       GPIO number (e.g., `GPIO_NUM_4`).
        /// @param pull_type Pull configuration (internal/external, up/down).
        explicit constexpr DigitalInput(gpio_num_t pin, Pull pull_type) noexcept :
            _pin{static_cast<u8>(pin)}, _state{static_cast<u8>(pull_type)} {}

    private:
        const u8 _pin;
        u8 _state;

        [[nodiscard]] u8 matchMode(bool inverted_reading) const noexcept {
            if (static_cast<bool>(_state & external_pull_bit)) {
                return INPUT;
            } else {
                return inverted_reading ? INPUT_PULLDOWN : INPUT_PULLUP;
            }
        }

        KF_IMPL_INITABLE(DigitalInput, void());
        void initImpl() noexcept {
            const bool inverted_reading = ((_state & pull_up_bit) == 0);
            pinMode(_pin, matchMode(inverted_reading));
            _state = static_cast<u8>(inverted_reading);
        }

        KF_IMPL(::kf::gpio::GPIO::Input<DigitalInput, bool, void()>);
        bool readImpl() const noexcept {
            const auto level = static_cast<bool>(digitalRead(_pin));

            // if inverted
            if (static_cast<bool>(_state)) {
                return not level;
            } else {
                return level;
            }
        }
    };

    /// @brief Arduino ADC input (typically 12‑bit on ESP32).
    /// @note Global resolution is set via `resolution()`.
    struct AdcInput : GPIO::AdcInput<AdcInput, void()> {

        /// @brief Constructor.
        /// @param pin GPIO number with ADC capability (e.g., `GPIO_NUM_34`)
        explicit constexpr AdcInput(gpio_num_t pin) noexcept : _pin{static_cast<u8>(pin)} {}

    private:
        inline static u8 resolution_bits{12};// Arduino default resolution on ESP32 is 12-bits

        const u8 _pin;

        KF_IMPL_INITABLE(AdcInput, void());
        void initImpl() noexcept {
            pinMode(_pin, INPUT);
        }

        KF_IMPL(::kf::gpio::GPIO::Input<AdcInput, u16, void()>);
        [[nodiscard]] u16 readImpl() const noexcept {
            return analogRead(_pin);
        }

        KF_IMPL(::kf::gpio::GPIO::AdcInput<AdcInput, void()>);
        static void setResolutionImpl(u8 new_resolution_bits) noexcept {
            if (resolution_bits != new_resolution_bits) {
                resolution_bits = new_resolution_bits;
                analogReadResolution(resolution_bits);
            }
        }

        static u8 getResolutionImpl() noexcept {
            return resolution_bits;
        }
    };

    /// @brief Arduino digital output.
    struct DigitalOutput : GPIO::DigitalOutput<DigitalOutput, void()> {

        /// @brief Constructor.
        /// @param pin GPIO number (e.g., `GPIO_NUM_2`).
        explicit constexpr DigitalOutput(gpio_num_t pin) noexcept :
            _pin{static_cast<u8>(pin)} {}

    private:
        const u8 _pin;

        KF_IMPL_INITABLE(DigitalOutput, void());
        void initImpl() noexcept {
            pinMode(_pin, OUTPUT);
        }

        KF_IMPL(::kf::gpio::GPIO::Output<DigitalOutput, bool, void()>);
        void writeImpl(bool level) const noexcept {
            digitalWrite(_pin, level);
        }
    };

    /// @brief Arduino PWM output using ESP32 LEDC hardware
    struct PwmOutput :

        GPIO::PwmOutput<PwmOutput, bool()>,
        mixin::Configurable<internal::ArduinoPwmOutputConfig>

    {

        /// @brief Configuration for an ESP32 LEDC PWM channel
        using Config = internal::ArduinoPwmOutputConfig;

        using mixin::Configurable<Config>::Configurable;

    private:
        KF_IMPL_INITABLE(PwmOutput, bool());
        bool initImpl() noexcept {
            if (ledcSetup(this->config().channel, this->config().frequency_hz, this->config().resolution_bits) == 0) {
                return false;
            }

            ledcAttachPin(this->config().pin, this->config().channel);

            return true;
        }

        KF_IMPL(::kf::gpio::GPIO::Output<PwmOutput, u16, bool()>);
        void writeImpl(u16 level) const noexcept {
            ledcWrite(this->config().channel, level);
        }

        KF_IMPL(::kf::gpio::GPIO::PwmOutput<PwmOutput, bool()>);
        u32 getFrequencyImpl() const noexcept { return this->config().frequency_hz; }
        u8 getResolutionImpl() const noexcept { return this->config().resolution_bits; }
    };
};

}// namespace kf::gpio