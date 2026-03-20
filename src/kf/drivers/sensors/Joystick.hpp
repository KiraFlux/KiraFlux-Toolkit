// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Initable.hpp"
#include "kf/tuner/Tuner.hpp"

#include "kf/drivers/sensors/NormalizedAdcInput.hpp"
#include "kf/drivers/sensors/Sensor.hpp"

namespace kf::drivers::sensors {

namespace internal {
struct Value {
    f32 x;        ///< Normalized X-axis value (-1.0 to 1.0)
    f32 y;        ///< Normalized Y-axis value (-1.0 to 1.0)
    f32 magnitude;///< Combined vector magnitude (0.0 to 1.0)
};
}// namespace internal

/// @brief Two-axis joystick with calibration support
/// @note Uses filtered analog inputs and includes dead-zone compensation
template<typename I> struct Joystick final : Sensor<Joystick<I>, internal::Value, void> {
    using NormalizedAdcInputImpl = I;

    /// @brief Normalized joystick reading value
    using Value = internal::Value;

    struct Config;// forward declaration

    /// @brief Tuner for a complete two‑axis joystick.
    /// @note Aggregates two NormalizedAdcInput::Tuner instances (X and Y).
    ///       After reset(), call poll() repeatedly until running() returns false.
    ///       The tuner reads raw values from the joystick axes internally.
    struct Tuner : tuner::Tuner<Tuner> {
        explicit Tuner(Config &config, Joystick &joystick, u16 samples) noexcept :
            _tuner_x{config.x.createTuner(joystick.axis_x, samples)},
            _tuner_y{config.y.createTuner(joystick.axis_y, samples)} {}

    private:
        typename NormalizedAdcInputImpl::Tuner _tuner_x, _tuner_y;

        // impl
        using This = Tuner;

        KF_IMPL_RESETTABLE(This);
        void resetImpl() noexcept {
            _tuner_x.reset();
            _tuner_y.reset();
        }

        KF_IMPL_POLLABLE(This);
        void pollImpl() noexcept {
            _tuner_x.poll();
            _tuner_y.poll();
        }

        KF_IMPL_TUNER(This);
        bool runningImpl() const noexcept {
            return _tuner_x.running() or _tuner_y.running();
        }
    };

    struct Config {
        typename NormalizedAdcInputImpl::Config x, y;

        [[nodiscard]] Tuner createTuner(Joystick &joystick, u16 samples) noexcept {
            return Tuner{*this, joystick, samples};
        }
    };

    NormalizedAdcInputImpl axis_x, axis_y;

    explicit Joystick(
        const Config &config,
        const typename NormalizedAdcInputImpl::FilterImpl::Config &filter_config,
        typename NormalizedAdcInputImpl::AdcPinImpl &&pin_x,
        typename NormalizedAdcInputImpl::AdcPinImpl &&pin_y) noexcept :
        axis_x{config.x, filter_config, std::move(pin_x)},
        axis_y{config.y, filter_config, std::move(pin_y)} {}

private:
    // impl
    using This = Joystick<I>;

    KF_IMPL_INITABLE(This, void);
    void initImpl() noexcept {
        axis_x.init();
        axis_y.init();
    }

    KF_IMPL(Sensor<This, Value, void>);
    [[nodiscard]] Value readImpl() noexcept {
        // Output is normalized to unit circle (clamped at magnitude 1.0)

        const auto x = axis_x.read();
        const auto y = axis_y.read();
        const auto h = std::hypot(x, y);

        if (h < 1e-3) { return {0, 0, 0}; }
        if (h > 1) { return {x / h, y / h, 1}; }
        return {x, y, h};
    }
};

}// namespace kf::drivers::sensors