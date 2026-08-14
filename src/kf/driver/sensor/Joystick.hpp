// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    driver/sensor/Joystick.hpp
/// @brief   Two‑axis joystick with calibration and unit‑circle output.

#pragma once

#include "kf/core.hpp"
#include "kf/gpio.hpp"
#include "kf/math.hpp"
#include "kf/tuner/Tuner.hpp"
#include "kf/units.hpp"

#include "kf/driver/sensor/SensorDriver.hpp"

namespace kf::internal {

struct JoystickValue {
    f32 x;        ///< Normalized X-axis value (-1.0 to 1.0)
    f32 y;        ///< Normalized Y-axis value (-1.0 to 1.0)
    f32 magnitude;///< Combined vector magnitude (0.0 to 1.0)
};

}// namespace kf::internal

namespace kf::driver::sensor {

/// @brief Two-axis joystick with calibration support
/// @note Uses filtered analog inputs and includes dead-zone compensation
template<implements<SensorDriverTag> I> struct Joystick final : SensorDriver<Joystick<I>, internal::JoystickValue, void> {
    using Self = Joystick<I>;
    using InputImpl = I;

    /// @brief Normalized joystick reading value
    using Value = internal::JoystickValue;

    struct Config final {
        typename InputImpl::Config x, y;
        typename InputImpl::FilterImpl::Config filter;
    };

    /// @brief Tuner for a complete two‑axis joystick.
    /// @note Aggregates two InputImpl::Tuner instances (X and Y).
    ///       After reset(), call poll() repeatedly until running() returns false.
    ///       The tuner reads raw values from the joystick axes internally.
    struct Tuner : tuner::Tuner<Tuner> {
        explicit constexpr Tuner(Config &config, Joystick &joystick, usize samples) noexcept :
            _tuner_x{config.x, joystick.axis_x, samples},
            _tuner_y{config.y, joystick.axis_y, samples} {}

    private:
        typename InputImpl::Tuner _tuner_x, _tuner_y;

        KF_IMPL_TUNER(Tuner);

        constexpr void resetImpl() noexcept {
            _tuner_x.reset();
            _tuner_y.reset();
        }

        void pollImpl(units::Milliseconds now) noexcept {
            _tuner_x.poll(now);
            _tuner_y.poll(now);
        }

        bool runningImpl() const noexcept {
            return _tuner_x.running() or _tuner_y.running();
        }
    };

    InputImpl axis_x, axis_y;

    explicit constexpr Joystick(Config const &config, gpio::GpioNumber gpio_num_axis_x, gpio::GpioNumber gpio_num_axis_y) noexcept :
        axis_x{config.x, config.filter, gpio_num_axis_x},
        axis_y{config.y, config.filter, gpio_num_axis_y} {}

private:
    KF_IMPL_SENSOR_DRIVER(Self, Value, void);

    void initImpl() noexcept {
        axis_x.init();
        axis_y.init();
    }

    [[nodiscard]] Value readImpl() noexcept {
        // Output is normalized to unit circle (clamped at magnitude 1.0)

        auto const x = axis_x.read();
        auto const y = axis_y.read();
        auto const h = math::hypot(x, y);

        if (h < 1e-3) { return {0, 0, 0}; }
        if (h > 1) { return {x / h, y / h, 1}; }
        return {x, y, h};
    }
};

}// namespace kf::driver::sensor