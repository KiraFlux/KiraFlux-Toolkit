// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    driver/actuator/DRV8871.hpp
/// @brief   DRV8871 motor driver (H‑bridge) with dead‑zone compensation.

#pragma once

#include "kf/gpio.hpp"
#include "kf/math.hpp"
#include "kf/mixin/Configured.hpp"

#include "kf/driver/actuator/ActuatorDriver.hpp"

namespace kf::internal {

/// @brief Configuration for the DRV8871 motor driver
struct DRV8871Config {
    using InputType = kf::i16;///< Signed input command type

    gpio::PwmOutput::Config pwm;         ///< PWM configuration (shared with both GPIO objects)
    InputType max_input;                 ///< Maximum absolute input value (e.g., 1000)
    gpio::PwmOutput::Duty duty_dead_zone;///< Minimum PWM duty to start moving forward

    /// @brief Map speed command to PWM duty cycle respecting dead zone
    [[nodiscard]] constexpr gpio::PwmOutput::Duty calcDuty(InputType value) const noexcept {
        auto const clamped_input = math::clamp(value, 0, max_input);
        return static_cast<gpio::PwmOutput::Duty>(math::linearMap(clamped_input, 0, max_input, duty_dead_zone, pwm.maxDuty()));
    }
};

}// namespace kf::internal

namespace kf::driver::actuator {

/// @brief DRV8871 H-bridge motor driver abstraction
struct DRV8871 final :

    driver::actuator::ActuatorDriver<DRV8871, internal::DRV8871Config::InputType, bool()>,
    mixin::Configured<internal::DRV8871Config>

{
    using Config = internal::DRV8871Config;

    /// @param config Driver configuration (dead zones, max input)
    /// @param gpio_num_forward PWM gpio output for forward rotation
    /// @param gpio_num_backward PWM gpio output for backward rotation
    explicit DRV8871(Config const &config, gpio::GpioNumber gpio_num_forward, gpio::GpioNumber gpio_num_backward) noexcept :
        mixin::Configured<Config>(config),
        _pwm_gpio_forward{config.pwm, gpio_num_forward},
        _pwm_gpio_backward{config.pwm, gpio_num_backward} {}

private:
    gpio::PwmOutput _pwm_gpio_forward, _pwm_gpio_backward;

    KF_IMPL_ACTUATOR_DRIVER(DRV8871, Config::InputType, bool());

    bool initImpl() noexcept {
        if (not _pwm_gpio_forward.init()) { return false; }
        if (not _pwm_gpio_backward.init()) { return false; }
        return true;
    }

    void setImpl(Config::InputType value) noexcept {
        if (value < 0) {
            _pwm_gpio_forward.write(0);
            _pwm_gpio_backward.write(this->config().calcDuty(-value));
        } else {
            _pwm_gpio_forward.write(this->config().calcDuty(value));
            _pwm_gpio_backward.write(0);
        }
    }

    void stopImpl() noexcept {
        _pwm_gpio_forward.write(0);
        _pwm_gpio_backward.write(0);
    }
};

}// namespace kf::driver::actuator