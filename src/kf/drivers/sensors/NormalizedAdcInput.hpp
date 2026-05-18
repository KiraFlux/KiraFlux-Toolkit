// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "kf/algorithm.hpp"
#include "kf/primitives.hpp"
#include "kf/gpio/GPIO.hpp"
#include "kf/math/filters/ExponentialFilter.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/tuner/SampleCollectingTuner.hpp"

#include "kf/drivers/sensors/Sensor.hpp"

namespace kf::drivers::sensors {
namespace internal::nai {// NormalizedAdcInput
struct Config final : mixin::NonCopyable {
    using AdcSignedValue = i16;

    bool inverted;
    AdcSignedValue dead_zone;
    AdcSignedValue range_positive;
    AdcSignedValue range_negative;

    [[nodiscard]] constexpr static AdcSignedValue calcPositiveRange(AdcSignedValue max_analog_value, AdcSignedValue center) noexcept {
        return static_cast<AdcSignedValue>(max_analog_value - center);
    }

    [[nodiscard]] constexpr static AdcSignedValue calcNegativeRange(AdcSignedValue center) noexcept {
        return center;
    }
};
}// namespace internal::nai

/// @brief Single analog joystick axis with filtering and dead-zone compensation
template<typename I> struct NormalizedAdcInput final : Sensor<NormalizedAdcInput<I>, f32, void>, mixin::Configurable<internal::nai::Config> {
    KF_CHECK_IMPL(I, kf::gpio::AdcInputTag);

    using AdcPinImpl = I;
    using FilterImpl = math::filters::ExponentialFilter<f32>;
    using Config = internal::nai::Config;

    /// @brief Tuner for a single analog axis.
    /// @note Inherits from SampleCollectingTuner and implements the concrete logic:
    ///
    ///       - on reset: reset min, max, sum.
    ///
    ///       - on sample: update min, max, sum.
    ///
    ///       - on calculate: compute dead zone and ranges based on collected data.
    struct Tuner final : kf::tuner::SampleCollectingTuner<Tuner, Config> {
        using TunerBase = kf::tuner::SampleCollectingTuner<Tuner, Config>;

        explicit Tuner(Config &config, NormalizedAdcInput &normalized_input, u16 samples) :
            _normalized_input{normalized_input}, TunerBase{config, samples} {}

    private:
        NormalizedAdcInput &_normalized_input;
        i64 _sum{};
        Config::AdcSignedValue _max_sample{};
        Config::AdcSignedValue _min_sample{};

        // impl
        using This = Tuner;

        KF_IMPL_RESETTABLE(This);
        void resetImpl() noexcept {
            _max_sample = 0;
            _min_sample = AdcPinImpl::maxValue();
            _sum = 0;
        }

        KF_IMPL_POLLABLE(This);
        void pollImpl() noexcept {
            const auto sample = Config::AdcSignedValue(_normalized_input.readRaw());
            _max_sample = kf::max(_max_sample, sample);
            _min_sample = kf::min(_min_sample, sample);
            _sum += sample;
        }

        friend TunerBase;
        void calculateImpl(Config &config) const noexcept {
            constexpr auto margin{10};
            constexpr auto zone_percents{10};

            config.dead_zone = static_cast<Config::AdcSignedValue>((_max_sample - _min_sample) / zone_percents + margin);
            const auto center = static_cast<Config::AdcSignedValue>(_sum / this->samples_total);
            config.range_positive = Config::calcPositiveRange(AdcPinImpl::maxValue(), center);
            config.range_negative = Config::calcNegativeRange(center);
        }
    };

    explicit NormalizedAdcInput(const Config &config, const typename FilterImpl::Config &filter_config, AdcPinImpl &&pin) noexcept :
        mixin::Configurable<Config>{config}, _filter{filter_config}, _pin{std::move(pin)} {}

    [[nodiscard]] u16 readRaw() const noexcept { return _pin.read(); }

private:
    FilterImpl _filter;
    AdcPinImpl _pin;

    // impl
    using This = NormalizedAdcInput<I>;

    KF_IMPL_INITABLE(This, void);
    void initImpl() noexcept { _pin.init(); }

    KF_IMPL(Sensor<This, f32, void>);
    [[nodiscard]] f32 readImpl() noexcept {
        // Applies dead zone, filtering, and optional inversion
        const auto deviation = static_cast<Config::AdcSignedValue>(readRaw()) - this->config().range_negative;

        if (kf::abs(deviation) < this->config().dead_zone) {
            return 0.0f;
        }

        auto value = _filter.calc(f32(deviation));

        value /= f32((value < 0.0f) ? this->config().range_negative : this->config().range_positive);

        if (this->config().inverted) {
            value *= -1;
        }

        return value;
    }
};

}// namespace kf::drivers::sensors