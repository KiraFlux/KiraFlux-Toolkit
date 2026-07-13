// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/math.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/tuner/Tuner.hpp"

#include "kf/driver/sensor/Sensor.hpp"

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
template<typename I> struct Joystick final : Sensor<Joystick<I>, internal::JoystickValue, void()> {
    KF_CHECK_IMPL(I, Sensor<I, f32, void()>);

    using InputImpl = I;

    /// @brief Normalized joystick reading value
    using Value = internal::JoystickValue;

    struct Config final {
        typename InputImpl::Config x, y;
    };

    /// @brief Tuner for a complete two‑axis joystick.
    /// @note Aggregates two InputImpl::Tuner instances (X and Y).
    ///       After reset(), call poll() repeatedly until running() returns false.
    ///       The tuner reads raw values from the joystick axes internally.
    struct Tuner : tuner::Tuner<Tuner> {
        explicit Tuner(Config &config, Joystick &joystick, u16 samples) noexcept :
            _tuner_x{config.x, joystick.axis_x, samples},
            _tuner_y{config.y, joystick.axis_y, samples} {}

    private:
        typename InputImpl::Tuner _tuner_x, _tuner_y;

        KF_IMPL_TUNER(Tuner);

        constexpr void resetImpl() noexcept {
            _tuner_x.reset();
            _tuner_y.reset();
        }

        void pollImpl() noexcept {
            _tuner_x.poll();
            _tuner_y.poll();
        }

        bool runningImpl() const noexcept {
            return _tuner_x.running() or _tuner_y.running();
        }
    };

    InputImpl axis_x, axis_y;

    explicit Joystick(
        const Config &config,
        const typename InputImpl::FilterImpl::Config &filter_config,
        typename InputImpl::AdcInputImpl &&pin_x,
        typename InputImpl::AdcInputImpl &&pin_y) noexcept :
        axis_x{config.x, filter_config, std::move(pin_x)},
        axis_y{config.y, filter_config, std::move(pin_y)} {}

private:
    KF_IMPL_SENSOR(Joystick<I>, Value, void());

    void initImpl() noexcept {
        axis_x.init();
        axis_y.init();
    }

    [[nodiscard]] Value readImpl() noexcept {
        // Output is normalized to unit circle (clamped at magnitude 1.0)

        const auto x = axis_x.read();
        const auto y = axis_y.read();
        const auto h = math::hypot(x, y);

        if (h < 1e-3) { return {0, 0, 0}; }
        if (h > 1) { return {x / h, y / h, 1}; }
        return {x, y, h};
    }
};

}// namespace kf::driver::sensor