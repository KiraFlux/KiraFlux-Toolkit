// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/GPIO.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/primitives.hpp"

namespace kf::internal {

struct ArduinoPwmOutputConfig final {
    u32 frequency_hz;  ///< PWM frequency (Hz)
    u8 resolution_bits;///< resolution in bits (1..16)
    u8 gpio_num;       ///< GPIO pin number

    /// @brief Maximum PWM value for given resolution (2^bits - 1)
    [[nodiscard]] constexpr u16 maxDuty() const noexcept {
        return static_cast<u16>((1u << (resolution_bits)) - 1u);
    }
};

}// namespace kf::internal

namespace kf::arduino {

/// @brief Arduino Core GPIO backend
struct ArduinoGPIO : GpioTag {

    /// @brief Arduino digital input with configurable pull-up/down
    /// @note Pull mode is set during `init()`; after that the pin stays configured
    struct DigitalInput : GPIO::DigitalInput<DigitalInput, void()> {

        /// @param pin       GPIO number
        /// @param pull_type Pull configuration (internal/external, up/down)
        explicit constexpr DigitalInput(gpio_num_t gpio_num, Pull pull_type) noexcept :
            _gpio_num{static_cast<u8>(gpio_num)}, _state{static_cast<u8>(pull_type)} {}

    private:
        const u8 _gpio_num;
        u8 _state;

        [[nodiscard]] u8 matchMode(bool inverted_reading) const noexcept {
            if (static_cast<bool>(_state & external_pull_bit)) {
                return INPUT;
            } else {
                return inverted_reading ? INPUT_PULLDOWN : INPUT_PULLUP;
            }
        }

        KF_IMPL_GPIO_DIGITAL_INPUT(DigitalInput, void());

        void initImpl() noexcept {
            const bool inverted_reading = ((_state & pull_up_bit) == 0);
            pinMode(_gpio_num, matchMode(inverted_reading));
            _state = static_cast<u8>(inverted_reading);
        }

        bool readImpl() const noexcept {
            const auto level = static_cast<bool>(digitalRead(_gpio_num));

            // if inverted
            if (static_cast<bool>(_state)) {
                return not level;
            } else {
                return level;
            }
        }
    };

    /// @brief Arduino ADC input (typically 12‑bit on ESP32)
    /// @note Global resolution is set via `resolution()`
    struct AdcInput : GPIO::AdcInput<AdcInput, void()> {

        /// @param pin GPIO number with ADC capability (e.g., `GPIO_NUM_34`)
        explicit constexpr AdcInput(gpio_num_t gpio_num) noexcept :
            _gpio_num{static_cast<u8>(gpio_num)} {}

    private:
        inline static usize resolution_bits{12};// Arduino default resolution on ESP32 is 12-bits

        const u8 _gpio_num;

        KF_IMPL_GPIO_ADC_INPUT(AdcInput, void());

        void initImpl() noexcept {
            pinMode(_gpio_num, INPUT);
        }

        [[nodiscard]] u16 readImpl() const noexcept {
            return analogRead(_gpio_num);
        }

        static void setResolutionImpl(usize new_resolution_bits) noexcept {
            if (resolution_bits != new_resolution_bits) {
                resolution_bits = new_resolution_bits;
                analogReadResolution(resolution_bits);
            }
        }

        static usize getResolutionImpl() noexcept {
            return resolution_bits;
        }
    };

    /// @brief Arduino digital output
    struct DigitalOutput : GPIO::DigitalOutput<DigitalOutput, void()> {

        /// @param pin GPIO number
        explicit constexpr DigitalOutput(gpio_num_t gpio_num) noexcept :
            _gpio_num{static_cast<u8>(gpio_num)} {}

    private:
        const u8 _gpio_num;

        KF_IMPL_GPIO_DIGITAL_OUTPUT(DigitalOutput, void());

        void initImpl() noexcept {
            pinMode(_gpio_num, OUTPUT);
        }

        void writeImpl(bool level) const noexcept {
            digitalWrite(_gpio_num, level);
        }
    };

    /// @brief Arduino PWM output using ESP32 LEDC hardware
    struct PwmOutput :

        GPIO::PwmOutput<PwmOutput, bool()>,
        mixin::Configured<internal::ArduinoPwmOutputConfig>

    {

        /// @brief Configuration for an ESP32 LEDC PWM channel
        using Config = internal::ArduinoPwmOutputConfig;

        using mixin::Configured<Config>::Configured;

    private:
        KF_IMPL_GPIO_PWM_OUTPUT(PwmOutput, bool());

        bool initImpl() noexcept {
            return ledcAttach(this->config().gpio_num, this->config().frequency_hz, this->config().resolution_bits);
        }

        void writeImpl(u16 duty) const noexcept {
            ledcWrite(this->config().gpio_num, duty);
        }

        u32 getFrequencyImpl() const noexcept {
            return this->config().frequency_hz;
        }

        usize getResolutionImpl() const noexcept {
            return this->config().resolution_bits;
        }
    };
};

}// namespace kf::arduino