// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include <kf/drivers/actuators/Actuator.hpp>
#include <kf/gpio/GPIO.hpp>
#include <kf/math.hpp>
#include <kf/mixin/Configurable.hpp>

namespace kf::internal {

/// @brief Configuration for the DRV8871 motor driver
struct DRV8871Config {
    using DutyType = kf::u16; ///< PWM duty cycle type (0..maxDuty)
    using InputType = kf::i16;///< Signed input command type

    InputType max_input;        ///< Maximum absolute input value (e.g., 1000)
    DutyType forward_dead_zone; ///< Minimum PWM duty to start moving forward
    DutyType backward_dead_zone;///< Minimum PWM duty to start moving backward
};

}// namespace kf::internal

namespace kf::drivers::actuators {

/// @brief DRV8871 H-bridge motor driver abstraction
/// @tparam G Implementation of GPIO with PWM output support
template<typename G> struct DRV8871 final :

    drivers::actuators::Actuator<DRV8871<G>, bool()>,
    mixin::Configurable<internal::DRV8871Config>

{
    KF_CHECK_IMPL(G, ::kf::gpio::GPIO::PwmOutputTag);

    using PwmOutputImpl = G;
    using Config = internal::DRV8871Config;

    /// @param config Driver configuration (dead zones, max input)
    /// @param forward PWM output for forward rotation
    /// @param backward PWM output for backward rotation
    explicit DRV8871(const Config &config, PwmOutputImpl &&forward, PwmOutputImpl &&backward) noexcept :
        mixin::Configurable<Config>(config),
        _pwm_gpio_forward{std::move(forward)}, _pwm_gpio_backward{std::move(backward)} {}

    /// @brief Set motor speed and direction
    /// @param value Signed input command (-max_input .. +max_input)
    void set(Config::InputType value) noexcept {
        if (value < 0) {
            _pwm_gpio_forward.write(0);
            _pwm_gpio_backward.write(calcDuty(-value, this->config().backward_dead_zone, _pwm_gpio_backward));
        } else {
            _pwm_gpio_forward.write(calcDuty(value, this->config().forward_dead_zone, _pwm_gpio_forward));
            _pwm_gpio_backward.write(0);
        }
    }

    /// @brief Stop the motor (both outputs driven low)
    void stop() noexcept {
        _pwm_gpio_forward.write(0);
        _pwm_gpio_backward.write(0);
    }

private:
    PwmOutputImpl _pwm_gpio_forward, _pwm_gpio_backward;

    /// @brief Map speed command to PWM duty cycle respecting dead zone
    Config::DutyType calcDuty(Config::InputType value, Config::DutyType dead_zone, const PwmOutputImpl &pwm_output) const noexcept {
        return math::linearMap<Config::DutyType>(
            math::clamp(value, static_cast<Config::InputType>(0), this->config().max_input),
            0, this->config().max_input,
            dead_zone, pwm_output.config().maxDuty());
    }

    // impl
    using This = DRV8871<G>;

    KF_IMPL_INITABLE(This, bool());
    bool initImpl() noexcept {
        if (not _pwm_gpio_forward.init()) { return false; }
        if (not _pwm_gpio_backward.init()) { return false; }
        return true;
    }
};

}// namespace kf::drivers::actuators